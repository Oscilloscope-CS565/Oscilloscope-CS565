#ifndef IO_WAVEFORM_WIDGET_H
#define IO_WAVEFORM_WIDGET_H

#include <QVector>
#include <QWidget>

namespace ioWaveformWidget {

class WaveformWidget : public QWidget {
    Q_OBJECT

public:
    explicit WaveformWidget(QWidget *parent = nullptr);

    void setSamples(const QVector<double> &values);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> samples_;
    static const int kMaxSamples = 512;
};

} // namespace ioWaveformWidget

#endif
