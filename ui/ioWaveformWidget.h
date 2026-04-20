#ifndef IO_WAVEFORM_WIDGET_H
#define IO_WAVEFORM_WIDGET_H

#include <QPoint>
#include <QVector>
#include <QWidget>

class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QScrollBar;
class QWheelEvent;

namespace ioWaveformWidget {

class WaveformWidget : public QWidget {
    Q_OBJECT

public:
    explicit WaveformWidget(QWidget *parent = nullptr);

    void setSamples(const QVector<double> &values);
    /** Scale: vertical amplitude; Shift: vertical pan (see widget implementation). */
    void setTraceParams(double scale, double shift);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void clampViewStart();
    void syncFollowLiveToEnd();
    void syncScrollBar();
    /** Horizontal pitch in pixels (fixed; independent of vertical Scale). */
    double pixelsPerSample() const;
    /** How many sample–sample segments fit in the widget width at current pitch. */
    int visibleSegmentCount() const;

    QVector<double> history_;
    int viewStart_{0};
    bool followLive_{true};
    bool dragging_{false};
    QPoint dragPressPos_;
    int viewStartAtDrag_{0};

    double traceScale_{1.0};
    double traceShift_{0.0};

    QScrollBar *scrollBar_{nullptr};
};

} // namespace ioWaveformWidget

#endif
