#include "ioOscilloscopeModel.h"
#include <QCoreApplication>
#include <cmath>

namespace ioOscilloscopeModel {

OscilloscopeModel::OscilloscopeModel(QObject *parent)
    : QObject(parent),
      running_(false),
      scaleValue_(1.0),
      shiftValue_(0.0),
      sampleHz_(20.0),
      rawBufferSize_(2048),
      readDeviceIndex_(0),
      writeDeviceIndex_(1),
      dualFtdiOutput_(false),
      writeBackToReadDevice_(true),
      blinkDb0_(true),
      reader_(nullptr),
      pipeline_(nullptr),
      writer_(nullptr),
      displayEmitCounter_(0) {}

OscilloscopeModel::~OscilloscopeModel() {
    stopAcquisition();
}

void OscilloscopeModel::setScale(double value) {
    scaleValue_ = value;
    if (pipeline_ != nullptr) {
        pipeline_->scale.store(value);
    }
}

void OscilloscopeModel::setShift(double value) {
    shiftValue_ = value;
    if (pipeline_ != nullptr) {
        pipeline_->shift.store(value);
    }
}

void OscilloscopeModel::setSampleHz(double hz) {
    if (hz > 0.1) {
        sampleHz_ = hz;
    }
}

void OscilloscopeModel::setRawBufferSize(int bytes) {
    if (bytes >= 256) {
        rawBufferSize_ = bytes;
    }
}

void OscilloscopeModel::setReadDeviceIndex(int index) {
    readDeviceIndex_ = index;
}

void OscilloscopeModel::setWriteDeviceIndex(int index) {
    writeDeviceIndex_ = index;
}

void OscilloscopeModel::setDualFtdiOutput(bool enabled) {
    dualFtdiOutput_ = enabled;
}

void OscilloscopeModel::setWriteBackToReadDevice(bool enabled) {
    writeBackToReadDevice_ = enabled;
}

void OscilloscopeModel::setBlinkDb0(bool enabled) {
    blinkDb0_ = enabled;
    if (pipeline_ != nullptr) {
        pipeline_->blinkDb0.store(enabled);
    }
}

void OscilloscopeModel::deliverSample(quint8 value) {
    appendDisplaySample(static_cast<unsigned char>(value));
}

void OscilloscopeModel::appendDisplaySample(unsigned char value) {
    QVector<double> snapshot;
    {
        QMutexLocker lock(&displayMutex_);
        // Blink: o = (v&0xFE)|blinkBit — only LSB flips, so value/255 looks flat. Plotting only (value&1)
        // drops pipeline Scale/Shift (they live in the upper bits of v). Combine plateau (v with LSB 0)
        // with a ±0.5 normalized swing from DB0 so zigzag stays large and DC still tracks Scale/Shift.
        double y;
        if (blinkDb0_) {
            const double plateau = static_cast<double>(value & 0xFEU) / 255.0;
            y = plateau * 0.5 + static_cast<double>(value & 1U) * 0.5;
        } else {
            y = static_cast<double>(value) / 255.0;
        }
        displaySamples_.push_back(y);
        static const int kMaxDisplayHistory = 65536;
        if (displaySamples_.size() > kMaxDisplayHistory) {
            displaySamples_.remove(0, displaySamples_.size() - kMaxDisplayHistory);
        }
        ++displayEmitCounter_;
        const bool emitNow = blinkDb0_ || (displayEmitCounter_ % 4 == 0);
        if (emitNow) {
            snapshot = displaySamples_;
        }
    }
    if (!snapshot.isEmpty()) {
        emit samplesUpdated(snapshot);
    }
}

void OscilloscopeModel::startAcquisition() {
    if (running_.load()) {
        return;
    }

    readDevice_ = std::make_unique<ioFtdiDevice::FtdiDevice>();
    if (readDevice_->open(readDeviceIndex_) != FT_OK) {
        emit errorMessage(tr("Failed to open read FTDI device (index %1).").arg(readDeviceIndex_));
        readDevice_.reset();
        return;
    }

    writeDevice_.reset();
    if (dualFtdiOutput_) {
        writeDevice_ = std::make_unique<ioFtdiDevice::FtdiDevice>();
        if (writeDevice_->open(writeDeviceIndex_) != FT_OK) {
            emit errorMessage(tr("Failed to open write FTDI device (index %1). Aborted.").arg(writeDeviceIndex_));
            readDevice_->close();
            readDevice_.reset();
            writeDevice_.reset();
            return;
        }
    }

    rawBuffer_ = std::make_unique<ioCircularBuffer::CircularBuffer>(static_cast<std::size_t>(rawBufferSize_));
    processedBuffer_ = std::make_unique<ioCircularBuffer::CircularBuffer>(static_cast<std::size_t>(rawBufferSize_));

    reader_ = new ioThreadedReader::ThreadedReader(readDevice_.get());
    reader_->configure(rawBuffer_.get(), 1, sampleHz_);

    pipeline_ = new ioScaleShiftPipeline::ScaleShiftPipeline(rawBuffer_.get(), processedBuffer_.get());
    pipeline_->scale.store(scaleValue_);
    pipeline_->shift.store(shiftValue_);
    pipeline_->blinkDb0.store(blinkDb0_);
    pipeline_->setSampleCallback([this](unsigned char v) {
        QMetaObject::invokeMethod(
            this,
            "deliverSample",
            Qt::QueuedConnection,
            Q_ARG(quint8, static_cast<quint8>(v)));
    });

    if (dualFtdiOutput_ && writeDevice_) {
        writer_ = new ioThreadedWriter::ThreadedWriter(writeDevice_.get());
    } else if (!dualFtdiOutput_ && writeBackToReadDevice_) {
        writer_ = new ioThreadedWriter::ThreadedWriter(readDevice_.get());
    } else {
        const QString path = QCoreApplication::applicationDirPath() + QStringLiteral("/capture.bin");
        writer_ = new ioThreadedWriter::ThreadedWriter(path.toUtf8().constData());
    }
    writer_->configure(processedBuffer_.get(), 1, sampleHz_);

    running_.store(true);
    displayEmitCounter_ = 0;
    {
        QMutexLocker lock(&displayMutex_);
        displaySamples_.clear();
    }

    pipeline_->start();
    reader_->start();
    writer_->start();

    QString dest;
    if (dualFtdiOutput_) {
        dest = tr("Write device %1 (second FTDI)").arg(writeDeviceIndex_);
    } else if (writeBackToReadDevice_) {
        dest = tr("Write back to same device (DB0 LED; D2XX locked)");
    } else {
        dest = tr("Processed stream to capture.bin");
    }
    emit logLine(tr("Acquisition started: read index %1; %2; DB0 blink: %3")
                     .arg(readDeviceIndex_)
                     .arg(dest)
                     .arg(blinkDb0_ ? tr("on") : tr("off")));
}

void OscilloscopeModel::stopAcquisition() {
    if (!running_.load() && reader_ == nullptr && pipeline_ == nullptr && writer_ == nullptr) {
        return;
    }

    if (reader_ != nullptr) {
        reader_->stop();
    }
    if (pipeline_ != nullptr) {
        pipeline_->stopJoin();
    }
    if (writer_ != nullptr) {
        writer_->stop();
    }

    delete writer_;
    writer_ = nullptr;
    delete pipeline_;
    pipeline_ = nullptr;
    delete reader_;
    reader_ = nullptr;

    processedBuffer_.reset();
    rawBuffer_.reset();

    if (readDevice_) {
        readDevice_->close();
        readDevice_.reset();
    }
    if (writeDevice_) {
        writeDevice_->close();
        writeDevice_.reset();
    }

    running_.store(false);
    emit logLine(tr("Acquisition stopped."));
}

} // namespace ioOscilloscopeModel
