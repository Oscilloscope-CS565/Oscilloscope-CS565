#include "ioMainWindow.h"
#include "ioCompactOscilloscopeView.h"
#include "ioWorkspaceOscilloscopeView.h"
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>

namespace ioMainWindow {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      model_(new ioOscilloscopeModel::OscilloscopeModel(this)),
      stack_(new QStackedWidget(this)),
      compactView_(new ioCompactOscilloscopeView::CompactOscilloscopeView(this)),
      workspaceView_(new ioWorkspaceOscilloscopeView::WorkspaceOscilloscopeView(this)),
      currentView_(nullptr) {
    setWindowTitle(tr("CS565 Oscilloscope / FTDI pipeline"));

    stack_->addWidget(compactView_->asWidget());
    stack_->addWidget(workspaceView_->asWidget());
    setCentralWidget(stack_);

    QActionGroup *viewGroup = new QActionGroup(this);
    QAction *actCompact = new QAction(tr("Compact view"), viewGroup);
    actCompact->setCheckable(true);
    QAction *actWorkspace = new QAction(tr("Workspace view"), viewGroup);
    actWorkspace->setCheckable(true);
    actCompact->setChecked(true);
    viewGroup->setExclusive(true);
    viewGroup->addAction(actCompact);
    viewGroup->addAction(actWorkspace);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(actCompact);
    viewMenu->addAction(actWorkspace);

    connect(actCompact, &QAction::triggered, this, &MainWindow::showCompactView);
    connect(actWorkspace, &QAction::triggered, this, &MainWindow::showWorkspaceView);

    connect(model_, &ioOscilloscopeModel::OscilloscopeModel::errorMessage,
            this, &MainWindow::onModelError);

    showCompactView();
    statusBar()->showMessage(tr("Ready — use View menu to switch UI"));
}

void MainWindow::detachCurrentView() {
    if (currentView_ != nullptr) {
        currentView_->unbindModel();
    }
    currentView_ = nullptr;
}

void MainWindow::attachCurrentView() {
    if (currentView_ != nullptr) {
        currentView_->bindModel(model_);
    }
}

void MainWindow::showCompactView() {
    detachCurrentView();
    stack_->setCurrentWidget(compactView_->asWidget());
    currentView_ = compactView_;
    attachCurrentView();
    statusBar()->showMessage(tr("Current: Compact view"));
}

void MainWindow::showWorkspaceView() {
    detachCurrentView();
    stack_->setCurrentWidget(workspaceView_->asWidget());
    currentView_ = workspaceView_;
    attachCurrentView();
    statusBar()->showMessage(tr("Current: Workspace view"));
}

void MainWindow::onModelError(const QString &message) {
    QMessageBox::warning(this, tr("Device error"), message);
}

} // namespace ioMainWindow
