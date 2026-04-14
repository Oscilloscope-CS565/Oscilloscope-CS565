#include "ioWaveformWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QtMath>

namespace ioWaveformWidget {

WaveformWidget::WaveformWidget(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(160);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void WaveformWidget::setSamples(const QVector<double> &values) {
    samples_ = values;
    if (samples_.size() > kMaxSamples) {
        samples_ = samples_.mid(samples_.size() - kMaxSamples);
    }
    update();
}

void WaveformWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), palette().color(QPalette::Base));
    painter.setPen(QPen(Qt::darkGreen, 2));

    if (samples_.isEmpty()) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, tr("No waveform data"));
        return;
    }

    const int w = width();
    const int h = height();
    const int n = samples_.size();
    if (n < 2) {
        return;
    }

    for (int i = 1; i < n; ++i) {
        double x0 = (double)(i - 1) / (double)(n - 1) * (double)(w - 1);
        double x1 = (double)i / (double)(n - 1) * (double)(w - 1);
        double y0 = h - 1 - qBound(0.0, samples_.at(i - 1), 1.0) * (double)(h - 4) - 2;
        double y1 = h - 1 - qBound(0.0, samples_.at(i), 1.0) * (double)(h - 4) - 2;
        painter.drawLine(QPointF(x0, y0), QPointF(x1, y1));
    }
}

} // namespace ioWaveformWidget
