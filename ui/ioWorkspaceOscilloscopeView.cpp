#include "ioWorkspaceOscilloscopeView.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>

namespace ioWorkspaceOscilloscopeView {

WorkspaceOscilloscopeView::WorkspaceOscilloscopeView(QWidget *parent)
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
      dualFtdiCheck_(new QCheckBox(tr("Dual FTDI (separate write ring buffer → second device)"), this)),
      writeBackCheck_(new QCheckBox(tr("Single device: write processed data back (DB0 LED)"), this)),
      blinkDb0Check_(new QCheckBox(tr("Toggle DB0 every sample (sample rate cadence)"), this)),
      startButton_(new QPushButton(tr("Start"), this)),
      stopButton_(new QPushButton(tr("Stop"), this)) {
    waveform_->setMinimumHeight(280);
    log_->setReadOnly(true);

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

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    QWidget *left = new QWidget(splitter);
    QVBoxLayout *leftLay = new QVBoxLayout(left);
    leftLay->addWidget(new QLabel(tr("Workspace view — large plot and side controls"), this));
    QGroupBox *plotBox = new QGroupBox(tr("Processed signal"), this);
    QVBoxLayout *plotLay = new QVBoxLayout(plotBox);
    plotLay->addWidget(waveform_);
    leftLay->addWidget(plotBox);
    leftLay->addWidget(new QLabel(tr("Log"), this));
    leftLay->addWidget(log_, 1);

    QWidget *right = new QWidget(splitter);
    QVBoxLayout *rightLay = new QVBoxLayout(right);
    QGroupBox *paramBox = new QGroupBox(tr("Pipeline & FTDI"), this);
    QFormLayout *form = new QFormLayout(paramBox);
    form->addRow(tr("Scale"), scaleSpin_);
    form->addRow(tr("Shift"), shiftSpin_);
    form->addRow(tr("Sample rate (Hz)"), hzSpin_);
    form->addRow(tr("Ring buffer capacity (each)"), bufferSpin_);
    form->addRow(tr("Read device index"), readIndexSpin_);
    form->addRow(tr("Write device index"), writeIndexSpin_);
    form->addRow(dualFtdiCheck_);
    form->addRow(writeBackCheck_);
    form->addRow(blinkDb0Check_);
    rightLay->addWidget(paramBox);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addWidget(startButton_);
    btnRow->addWidget(stopButton_);
    rightLay->addLayout(btnRow);
    rightLay->addStretch();

    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->addWidget(splitter);

    connect(startButton_, &QPushButton::clicked, this, &WorkspaceOscilloscopeView::handleStart);
    connect(stopButton_, &QPushButton::clicked, this, &WorkspaceOscilloscopeView::handleStop);
    connect(scaleSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &WorkspaceOscilloscopeView::applyScale);
    connect(shiftSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &WorkspaceOscilloscopeView::applyShift);
    connect(hzSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &WorkspaceOscilloscopeView::applySampleHz);
    connect(bufferSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &WorkspaceOscilloscopeView::applyBufferSize);
    connect(readIndexSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &WorkspaceOscilloscopeView::applyReadIndex);
    connect(writeIndexSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &WorkspaceOscilloscopeView::applyWriteIndex);
    connect(dualFtdiCheck_, &QCheckBox::checkStateChanged, this, &WorkspaceOscilloscopeView::applyDualFtdi);
    connect(writeBackCheck_, &QCheckBox::checkStateChanged, this, &WorkspaceOscilloscopeView::applyWriteBack);
    connect(blinkDb0Check_, &QCheckBox::checkStateChanged, this, &WorkspaceOscilloscopeView::applyBlinkDb0);

    waveform_->setTraceParams(scaleSpin_->value(), shiftSpin_->value());
}

QWidget *WorkspaceOscilloscopeView::asWidget() {
    return this;
}

QString WorkspaceOscilloscopeView::viewTitle() const {
    return tr("Workspace view");
}

void WorkspaceOscilloscopeView::bindModel(ioOscilloscopeModel::OscilloscopeModel *model) {
    unbindModel();
    model_ = model;
    if (model_ == nullptr) {
        return;
    }
    connections_.append(connect(model_, &ioOscilloscopeModel::OscilloscopeModel::samplesUpdated,
                                this, &WorkspaceOscilloscopeView::onSamplesUpdated));
    connections_.append(connect(model_, &ioOscilloscopeModel::OscilloscopeModel::logLine,
                                this, &WorkspaceOscilloscopeView::onLogLine));
    model_->setWriteBackToReadDevice(writeBackCheck_->isEnabled() && writeBackCheck_->isChecked());
    model_->setBlinkDb0(blinkDb0Check_->isChecked());

    readIndexSpin_->blockSignals(true);
    readIndexSpin_->setValue(model_->readDeviceIndex());
    readIndexSpin_->blockSignals(false);
}

void WorkspaceOscilloscopeView::unbindModel() {
    for (const QMetaObject::Connection &c : connections_) {
        QObject::disconnect(c);
    }
    connections_.clear();
    model_ = nullptr;
}

void WorkspaceOscilloscopeView::onSamplesUpdated(const QVector<double> &samples) {
    waveform_->setTraceParams(scaleSpin_->value(), shiftSpin_->value());
    waveform_->setSamples(samples);
}

void WorkspaceOscilloscopeView::onLogLine(const QString &text) {
    log_->append(text);
}

void WorkspaceOscilloscopeView::handleStart() {
    if (model_ != nullptr) {
        model_->startAcquisition();
    }
}

void WorkspaceOscilloscopeView::handleStop() {
    if (model_ != nullptr) {
        model_->stopAcquisition();
    }
}

void WorkspaceOscilloscopeView::applyScale(double value) {
    waveform_->setTraceParams(value, shiftSpin_->value());
    if (model_ != nullptr) {
        model_->setScale(value);
    }
}

void WorkspaceOscilloscopeView::applyShift(double value) {
    waveform_->setTraceParams(scaleSpin_->value(), value);
    if (model_ != nullptr) {
        model_->setShift(value);
    }
}

void WorkspaceOscilloscopeView::applySampleHz(double value) {
    if (model_ != nullptr) {
        model_->setSampleHz(value);
    }
}

void WorkspaceOscilloscopeView::applyBufferSize(int value) {
    if (model_ != nullptr) {
        model_->setRawBufferSize(value);
    }
}

void WorkspaceOscilloscopeView::applyReadIndex(int value) {
    if (model_ != nullptr) {
        model_->setReadDeviceIndex(value);
    }
}

void WorkspaceOscilloscopeView::applyWriteIndex(int value) {
    if (model_ != nullptr) {
        model_->setWriteDeviceIndex(value);
    }
}

void WorkspaceOscilloscopeView::applyDualFtdi(Qt::CheckState state) {
    if (model_ != nullptr) {
        model_->setDualFtdiOutput(state == Qt::Checked);
    }
    refreshWriteBackAvailability();
}

void WorkspaceOscilloscopeView::applyWriteBack(Qt::CheckState state) {
    if (model_ != nullptr && dualFtdiCheck_->checkState() != Qt::Checked) {
        model_->setWriteBackToReadDevice(state == Qt::Checked);
    }
}

void WorkspaceOscilloscopeView::applyBlinkDb0(Qt::CheckState state) {
    if (model_ != nullptr) {
        model_->setBlinkDb0(state == Qt::Checked);
    }
}

void WorkspaceOscilloscopeView::refreshWriteBackAvailability() {
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

} // namespace ioWorkspaceOscilloscopeView
