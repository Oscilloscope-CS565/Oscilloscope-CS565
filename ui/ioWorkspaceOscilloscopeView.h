#ifndef IO_WORKSPACE_OSCILLOSCOPE_VIEW_H
#define IO_WORKSPACE_OSCILLOSCOPE_VIEW_H

#include "ioAbstractOscilloscopeView.h"
#include "ioOscilloscopeModel.h"
#include "ioWaveformWidget.h"
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVector>
#include <QWidget>

namespace ioWorkspaceOscilloscopeView {

class WorkspaceOscilloscopeView : public QWidget, public ioAbstractOscilloscopeView::AbstractOscilloscopeView {
    Q_OBJECT

public:
    explicit WorkspaceOscilloscopeView(QWidget *parent = nullptr);

    QWidget *asWidget() override;
    QString viewTitle() const override;
    void bindModel(ioOscilloscopeModel::OscilloscopeModel *model) override;
    void unbindModel() override;

public slots:
    void onSamplesUpdated(const QVector<double> &samples);
    void onLogLine(const QString &text);

private slots:
    void handleStart();
    void handleStop();
    void applyScale(double value);
    void applyShift(double value);
    void applySampleHz(double value);
    void applyBufferSize(int value);
    void applyReadIndex(int value);
    void applyWriteIndex(int value);
    void applyDualFtdi(Qt::CheckState state);
    void applyWriteBack(Qt::CheckState state);
    void applyBlinkDb0(Qt::CheckState state);
    void refreshWriteBackAvailability();

private:
    ioOscilloscopeModel::OscilloscopeModel *model_;
    QVector<QMetaObject::Connection> connections_;

    ioWaveformWidget::WaveformWidget *waveform_;
    QTextEdit *log_;
    QDoubleSpinBox *scaleSpin_;
    QDoubleSpinBox *shiftSpin_;
    QDoubleSpinBox *hzSpin_;
    QSpinBox *bufferSpin_;
    QSpinBox *readIndexSpin_;
    QSpinBox *writeIndexSpin_;
    QCheckBox *dualFtdiCheck_;
    QCheckBox *writeBackCheck_;
    QCheckBox *blinkDb0Check_;
    QPushButton *startButton_;
    QPushButton *stopButton_;
};

} // namespace ioWorkspaceOscilloscopeView

#endif
