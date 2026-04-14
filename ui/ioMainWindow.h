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
    explicit MainWindow(QWidget *parent = nullptr);

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
