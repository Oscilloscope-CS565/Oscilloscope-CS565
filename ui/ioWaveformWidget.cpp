#include "ioWaveformWidget.h"
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QAbstractSlider>
#include <QResizeEvent>
#include <QScrollBar>
#include <QWheelEvent>
#include <QtMath>
#include <cmath>

namespace ioWaveformWidget {

namespace {

constexpr int kPlotMarginY = 4;
/** Horizontal pitch between samples (independent of vertical Scale). */
constexpr double kPixelsPerSample = 28.0;
/** Display gain on Shift (same spinbox units); larger ⇒ stronger vertical pan. */
constexpr double kShiftDisplayGain = 10.0;
/** Stroke color for trace (#2E7D32). */
constexpr QColor kTraceGreen(0x2e, 0x7d, 0x32);
constexpr QColor kFrameBar(0xe0, 0xe0, 0xe0);
/** Fraction of inner plot height left empty above/below the trace (each end). */
constexpr double kTraceVerticalInsetFrac = 0.08;
/** Reserved height at bottom for horizontal history scrollbar. */
constexpr int kScrollBarH = 18;

} // namespace

WaveformWidget::WaveformWidget(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(160 + kScrollBarH);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setToolTip(tr("Mouse wheel: pan history; drag to pan; scrollbar or scroll right for live follow"));

    scrollBar_ = new QScrollBar(Qt::Horizontal, this);
    scrollBar_->setFocusPolicy(Qt::ClickFocus);
    scrollBar_->setRange(0, 0);
    connect(scrollBar_, &QAbstractSlider::valueChanged, this, [this](int value) {
        viewStart_ = value;
        const int n = history_.size();
        const int span = visibleSegmentCount();
        const int maxStart = qMax(0, n - 1 - span);
        followLive_ = (viewStart_ >= maxStart - 1);
        update();
    });
}

void WaveformWidget::setTraceParams(double scale, double shift) {
    traceScale_ = scale;
    traceShift_ = shift;
    syncScrollBar();
    update();
}

void WaveformWidget::syncScrollBar() {
    if (scrollBar_ == nullptr) {
        return;
    }
    const int n = history_.size();
    const int span = visibleSegmentCount();
    const int maxStart = qMax(0, n - 1 - span);
    scrollBar_->blockSignals(true);
    if (n <= span + 1 || maxStart <= 0) {
        scrollBar_->setEnabled(false);
        scrollBar_->setRange(0, 0);
        scrollBar_->setValue(0);
    } else {
        scrollBar_->setEnabled(true);
        scrollBar_->setRange(0, maxStart);
        scrollBar_->setPageStep(qMax(1, span / 2));
        scrollBar_->setSingleStep(1);
        scrollBar_->setValue(qBound(0, viewStart_, maxStart));
    }
    scrollBar_->blockSignals(false);
}

void WaveformWidget::setSamples(const QVector<double> &values) {
    history_ = values;
    clampViewStart();
    if (followLive_) {
        syncFollowLiveToEnd();
    }
    syncScrollBar();
    update();
}

double WaveformWidget::pixelsPerSample() const {
    return qMax(4.0, kPixelsPerSample);
}

int WaveformWidget::visibleSegmentCount() const {
    const int w = qMax(1, width());
    const double px = pixelsPerSample();
    return qMax(1, static_cast<int>(std::floor(static_cast<double>(w) / px)));
}

void WaveformWidget::clampViewStart() {
    const int n = history_.size();
    const int span = visibleSegmentCount();
    const int maxStart = qMax(0, n - 1 - span);
    viewStart_ = qBound(0, viewStart_, maxStart);
}

void WaveformWidget::syncFollowLiveToEnd() {
    const int n = history_.size();
    const int span = visibleSegmentCount();
    viewStart_ = qMax(0, n - 1 - span);
}

void WaveformWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), palette().color(QPalette::Base));

    const int w = width();
    const int hPlot = qMax(1, height() - kScrollBarH);
    const int plotH = hPlot - 2 * kPlotMarginY;

    if (history_.isEmpty()) {
        painter.setPen(Qt::gray);
        painter.drawText(QRect(0, 0, w, hPlot), Qt::AlignCenter, tr("No waveform data"));
        syncScrollBar();
        return;
    }

    const int n = history_.size();
    if (n < 2 || w < 2) {
        syncScrollBar();
        return;
    }

    clampViewStart();

    constexpr int kFrameH = 3;
    painter.fillRect(0, 0, w, kFrameH, kFrameBar);
    painter.fillRect(0, hPlot - kFrameH, w, hPlot, kFrameBar);

    const int span = visibleSegmentCount();
    const int lastIdx = qMin(viewStart_ + span, n - 1);

    const double innerH = static_cast<double>(plotH - 2 * kFrameH);
    const double inset = qBound(4.0, innerH * kTraceVerticalInsetFrac, innerH * 0.33);
    const double traceTop = static_cast<double>(kPlotMarginY + kFrameH) + inset;
    const double traceBottom = static_cast<double>(kPlotMarginY + plotH - kFrameH) - inset;
    const double traceSpan = traceBottom - traceTop;

    /* Scale: vertical amplitude only (fixed 0→1 → band around center). Shift: vertical pan in pixels. */
    const double centerY = (traceTop + traceBottom) * 0.5;
    const double halfSpan = (traceBottom - traceTop) * 0.5;
    const double shiftPx = traceShift_ * (traceSpan / 255.0) * kShiftDisplayGain;
    const double px = pixelsPerSample();
    const double spanW = px * static_cast<double>(span);
    const double xMargin = qMax(0.0, (static_cast<double>(w) - spanW) * 0.5);

    auto valueToY = [&](double norm01) {
        const double v = qBound(0.0, norm01, 1.0);
        const double u = (v - 0.5) * 2.0;
        const double y = centerY - u * halfSpan * traceScale_ - shiftPx;
        return qBound(traceTop, y, traceBottom);
    };

    /* Sharp zigzag / sawtooth: straight segments between samples (peaks and valleys lie on horizontal lines when data alternates). */
    painter.setRenderHint(QPainter::Antialiasing, false);

    QPainterPath outline;
    for (int i = viewStart_; i <= lastIdx; ++i) {
        const double x = xMargin + static_cast<double>(i - viewStart_) * px;
        const double y = valueToY(history_.at(i));
        if (i == viewStart_) {
            outline.moveTo(x, y);
        } else {
            outline.lineTo(x, y);
        }
    }

    const double lineW = qBound(1.0, 1.25 * std::abs(traceScale_), 2.5);
    painter.strokePath(outline, QPen(kTraceGreen, lineW, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

    if (!followLive_ && n > span + 1) {
        painter.setPen(Qt::gray);
        const QString hint = tr("History view — wheel, drag, or scrollbar; far right = live");
        painter.drawText(QRect(8, hPlot - 22, w - 16, 18), Qt::AlignLeft | Qt::AlignVCenter, hint);
    }

    syncScrollBar();
}

void WaveformWidget::wheelEvent(QWheelEvent *event) {
    const int n = history_.size();
    const int span = visibleSegmentCount();
    if (n <= span + 1) {
        QWidget::wheelEvent(event);
        return;
    }

    const int maxStart = qMax(0, n - 1 - span);
    const int step = qMax(1, span / 4);
    const int delta = (event->angleDelta().y() > 0) ? step : -step;
    viewStart_ -= delta;
    viewStart_ = qBound(0, viewStart_, maxStart);

    followLive_ = (viewStart_ >= maxStart - 1);
    syncScrollBar();
    update();
    event->accept();
}

void WaveformWidget::mousePressEvent(QMouseEvent *event) {
    if (event->pos().y() >= height() - kScrollBarH) {
        QWidget::mousePressEvent(event);
        return;
    }
    if (event->button() == Qt::LeftButton) {
        dragging_ = true;
        dragPressPos_ = event->pos();
        viewStartAtDrag_ = viewStart_;
        followLive_ = false;
    }
    QWidget::mousePressEvent(event);
}

void WaveformWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!dragging_) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    const int n = history_.size();
    const int span = visibleSegmentCount();
    const int maxStart = qMax(0, n - 1 - span);
    const int dx = event->pos().x() - dragPressPos_.x();
    const int deltaSamples = static_cast<int>(qRound(static_cast<double>(dx) / pixelsPerSample()));
    viewStart_ = viewStartAtDrag_ - deltaSamples;
    viewStart_ = qBound(0, viewStart_, maxStart);
    followLive_ = (viewStart_ >= maxStart - 1);
    syncScrollBar();
    update();
}

void WaveformWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragging_ = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void WaveformWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (scrollBar_ != nullptr) {
        scrollBar_->setGeometry(0, height() - kScrollBarH, width(), kScrollBarH);
    }
    clampViewStart();
    if (followLive_) {
        syncFollowLiveToEnd();
    }
    syncScrollBar();
}

} // namespace ioWaveformWidget
