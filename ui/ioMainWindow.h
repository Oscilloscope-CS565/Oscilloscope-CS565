#ifndef IO_MAIN_WINDOW_H
#define IO_MAIN_WINDOW_H

#include "ioAbstractOscilloscopeView.h"
#include "ioOscilloscopeModel.h"
#include <QMainWindow>
#include <QStackedWidget>

namespace ioMainWindow {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /** personNumber: 1 or 2 — window title and default read FTDI index (0 / 1) for running two instances. */
    explicit MainWindow(int personNumber, QWidget *parent = nullptr);

private slots:
    void showCompactView();
    void showWorkspaceView();
    void onModelError(const QString &message);

private:
    void attachCurrentView();
    void detachCurrentView();

    ioOscilloscopeModel::OscilloscopeModel *model_;
    QStackedWidget *stack_;
    ioAbstractOscilloscopeView::AbstractOscilloscopeView *compactView_;
    ioAbstractOscilloscopeView::AbstractOscilloscopeView *workspaceView_;
    ioAbstractOscilloscopeView::AbstractOscilloscopeView *currentView_;
};

} // namespace ioMainWindow

#endif
