#ifndef IO_OSCILLOSCOPE_MODEL_H
#define IO_OSCILLOSCOPE_MODEL_H

#include <QMutex>
#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QVector>
#include <atomic>
#include <memory>

#include "ioCircularBuffer.h"
#include "ioFtdiDevice.h"
#include "ioScaleShiftPipeline.h"
#include "ioThreadedReader.h"
#include "ioThreadedWriter.h"

namespace ioOscilloscopeModel {

class OscilloscopeModel : public QObject {
    Q_OBJECT

public:
    explicit OscilloscopeModel(QObject *parent = nullptr);
    ~OscilloscopeModel() override;

    bool isRunning() const { return running_.load(); }
    int readDeviceIndex() const { return readDeviceIndex_; }

public slots:
    void startAcquisition();
    void stopAcquisition();
    void setScale(double value);
    void setShift(double value);
    void setSampleHz(double hz);
    void setRawBufferSize(int bytes);
    void setReadDeviceIndex(int index);
    void setWriteDeviceIndex(int index);
    void setDualFtdiOutput(bool enabled);
    /** When false or dual-FTDI mode, processed bytes go to capture.bin instead. */
    void setWriteBackToReadDevice(bool enabled);
    /** DB0 toggles each processed sample (same rate as sample Hz from UI). */
    void setBlinkDb0(bool enabled);

signals:
    void samplesUpdated(const QVector<double> &normalizedSamples);
    void logLine(const QString &text);
    void errorMessage(const QString &text);

private slots:
    void deliverSample(quint8 value);

private:
    void appendDisplaySample(unsigned char value);

    std::atomic<bool> running_;
    double scaleValue_;
    double shiftValue_;
    double sampleHz_;
    int rawBufferSize_;
    int readDeviceIndex_;
    int writeDeviceIndex_;
    bool dualFtdiOutput_;
    bool writeBackToReadDevice_;
    bool blinkDb0_;

    std::unique_ptr<ioFtdiDevice::FtdiDevice> readDevice_;
    std::unique_ptr<ioFtdiDevice::FtdiDevice> writeDevice_;
    std::unique_ptr<ioCircularBuffer::CircularBuffer> rawBuffer_;
    std::unique_ptr<ioCircularBuffer::CircularBuffer> processedBuffer_;

    ioThreadedReader::ThreadedReader *reader_;
    ioScaleShiftPipeline::ScaleShiftPipeline *pipeline_;
    ioThreadedWriter::ThreadedWriter *writer_;

    QVector<double> displaySamples_;
    QMutex displayMutex_;
    int displayEmitCounter_;
};

} // namespace ioOscilloscopeModel

#endif
