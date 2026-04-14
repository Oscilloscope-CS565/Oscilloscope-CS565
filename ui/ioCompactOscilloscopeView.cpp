#include "ioCompactOscilloscopeView.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace ioCompactOscilloscopeView {

CompactOscilloscopeView::CompactOscilloscopeView(QWidget *parent)
    : QWidget(parent),
      model_(nullptr),
      waveform_(new ioWaveformWidget::WaveformWidget(this)),
      log_(new QTextEdit(this)),
      scaleSpin_(new QDoubleSpinBox(this)),
      shiftSpin_(new QDoubleSpinBox(this)),
      hzSpin_(new QDoubleSpinBox(this)),
      bufferSpin_(new QSpinBox(this)),
      readIndexSpin_(new QSpinBox(this)),
      writeIndexSpin_(new QSpinBox(this)),
      dualFtdiCheck_(new QCheckBox(tr("Dual FTDI: write to second device"), this)),
      writeBackCheck_(new QCheckBox(tr("Single device: write processed data back (DB0 LED)"), this)),
      blinkDb0Check_(new QCheckBox(tr("Toggle DB0 every sample (sample rate cadence)"), this)),
      startButton_(new QPushButton(tr("Start"), this)),
      stopButton_(new QPushButton(tr("Stop"), this)) {
    log_->setReadOnly(true);
    log_->setMaximumHeight(120);

    scaleSpin_->setRange(-10.0, 10.0);
    scaleSpin_->setDecimals(3);
    scaleSpin_->setSingleStep(0.1);
    scaleSpin_->setValue(1.0);

    shiftSpin_->setRange(-255.0, 255.0);
    shiftSpin_->setDecimals(2);
    shiftSpin_->setSingleStep(1.0);

    hzSpin_->setRange(0.5, 500.0);
    hzSpin_->setDecimals(1);
    hzSpin_->setValue(20.0);

    bufferSpin_->setRange(256, 65536);
    bufferSpin_->setSingleStep(256);
    bufferSpin_->setValue(2048);

    readIndexSpin_->setRange(0, 7);
    writeIndexSpin_->setRange(0, 7);
    writeIndexSpin_->setValue(1);

    writeBackCheck_->setChecked(true);
    blinkDb0Check_->setChecked(true);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->addWidget(new QLabel(tr("Compact view — pipeline: FTDI → ring buffer → scale/shift → ring buffer → output")));

    QGroupBox *plotBox = new QGroupBox(tr("Signal display"), this);
    QVBoxLayout *plotLay = new QVBoxLayout(plotBox);
    plotLay->addWidget(waveform_);
    root->addWidget(plotBox);

    QGroupBox *paramBox = new QGroupBox(tr("Pipeline parameters"), this);
    QFormLayout *form = new QFormLayout(paramBox);
    form->addRow(tr("Scale"), scaleSpin_);
    form->addRow(tr("Shift"), shiftSpin_);
    form->addRow(tr("Sample rate (Hz)"), hzSpin_);
    form->addRow(tr("Ring buffer size (bytes)"), bufferSpin_);
    form->addRow(tr("Read device index"), readIndexSpin_);
    form->addRow(tr("Write device index"), writeIndexSpin_);
    form->addRow(dualFtdiCheck_);
    form->addRow(writeBackCheck_);
    form->addRow(blinkDb0Check_);
    root->addWidget(paramBox);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addWidget(startButton_);
    btnRow->addWidget(stopButton_);
    root->addLayout(btnRow);

    root->addWidget(new QLabel(tr("Log"), this));
    root->addWidget(log_);

    connect(startButton_, &QPushButton::clicked, this, &CompactOscilloscopeView::handleStart);
    connect(stopButton_, &QPushButton::clicked, this, &CompactOscilloscopeView::handleStop);
    connect(scaleSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CompactOscilloscopeView::applyScale);
    connect(shiftSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CompactOscilloscopeView::applyShift);
    connect(hzSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CompactOscilloscopeView::applySampleHz);
    connect(bufferSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &CompactOscilloscopeView::applyBufferSize);
    connect(readIndexSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &CompactOscilloscopeView::applyReadIndex);
    connect(writeIndexSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &CompactOscilloscopeView::applyWriteIndex);
    connect(dualFtdiCheck_, &QCheckBox::checkStateChanged, this, &CompactOscilloscopeView::applyDualFtdi);
    connect(writeBackCheck_, &QCheckBox::checkStateChanged, this, &CompactOscilloscopeView::applyWriteBack);
    connect(blinkDb0Check_, &QCheckBox::checkStateChanged, this, &CompactOscilloscopeView::applyBlinkDb0);
}

QWidget *CompactOscilloscopeView::asWidget() {
    return this;
}

QString CompactOscilloscopeView::viewTitle() const {
    return tr("Compact view");
}

void CompactOscilloscopeView::bindModel(ioOscilloscopeModel::OscilloscopeModel *model) {
    unbindModel();
    model_ = model;
    if (model_ == nullptr) {
        return;
    }
    connections_.append(connect(model_, &ioOscilloscopeModel::OscilloscopeModel::samplesUpdated,
                                this, &CompactOscilloscopeView::onSamplesUpdated));
    connections_.append(connect(model_, &ioOscilloscopeModel::OscilloscopeModel::logLine,
                                this, &CompactOscilloscopeView::onLogLine));
    model_->setWriteBackToReadDevice(writeBackCheck_->isEnabled() && writeBackCheck_->isChecked());
    model_->setBlinkDb0(blinkDb0Check_->isChecked());
}

void CompactOscilloscopeView::unbindModel() {
    for (const QMetaObject::Connection &c : connections_) {
        QObject::disconnect(c);
    }
    connections_.clear();
    model_ = nullptr;
}

void CompactOscilloscopeView::onSamplesUpdated(const QVector<double> &samples) {
    waveform_->setSamples(samples);
}

void CompactOscilloscopeView::onLogLine(const QString &text) {
    log_->append(text);
}

void CompactOscilloscopeView::handleStart() {
    if (model_ != nullptr) {
        model_->startAcquisition();
    }
}

void CompactOscilloscopeView::handleStop() {
    if (model_ != nullptr) {
        model_->stopAcquisition();
    }
}

void CompactOscilloscopeView::applyScale(double value) {
    if (model_ != nullptr) {
        model_->setScale(value);
    }
}

void CompactOscilloscopeView::applyShift(double value) {
    if (model_ != nullptr) {
        model_->setShift(value);
    }
}

void CompactOscilloscopeView::applySampleHz(double value) {
    if (model_ != nullptr) {
        model_->setSampleHz(value);
    }
}

void CompactOscilloscopeView::applyBufferSize(int value) {
    if (model_ != nullptr) {
        model_->setRawBufferSize(value);
    }
}

void CompactOscilloscopeView::applyReadIndex(int value) {
    if (model_ != nullptr) {
        model_->setReadDeviceIndex(value);
    }
}

void CompactOscilloscopeView::applyWriteIndex(int value) {
    if (model_ != nullptr) {
        model_->setWriteDeviceIndex(value);
    }
}

void CompactOscilloscopeView::applyDualFtdi(Qt::CheckState state) {
    if (model_ != nullptr) {
        model_->setDualFtdiOutput(state == Qt::Checked);
    }
    refreshWriteBackAvailability();
}

void CompactOscilloscopeView::applyWriteBack(Qt::CheckState state) {
    if (model_ != nullptr && dualFtdiCheck_->checkState() != Qt::Checked) {
        model_->setWriteBackToReadDevice(state == Qt::Checked);
    }
}

void CompactOscilloscopeView::applyBlinkDb0(Qt::CheckState state) {
    if (model_ != nullptr) {
        model_->setBlinkDb0(state == Qt::Checked);
    }
}

void CompactOscilloscopeView::refreshWriteBackAvailability() {
    const bool dual = dualFtdiCheck_->isChecked();
    writeBackCheck_->setEnabled(!dual);
    if (dual) {
        writeBackCheck_->setChecked(false);
        if (model_ != nullptr) {
            model_->setWriteBackToReadDevice(false);
        }
    } else if (model_ != nullptr) {
        model_->setWriteBackToReadDevice(writeBackCheck_->isChecked());
    }
}

} // namespace ioCompactOscilloscopeView
