#include <QtWidgets>
#include "otterstation-qt/mainwindow.h"

MainWindow::MainWindow() :
    core([this](float fps) {
        UpdateTitle(fps);
    }) {

    CreateMenubar();

    setMinimumSize(640, 480);
    resize(1280, 720);
}

void MainWindow::CreateMenubar() {
    CreateFileMenu();
    CreateEmulationMenu();
    CreateViewMenu();
}

void MainWindow::CreateFileMenu() {
    QMenu* file_menu = menuBar()->addMenu(tr("File"));

    QAction* load_action = file_menu->addAction(tr("Load ROM..."));
    file_menu->addSeparator();
    QAction* exit_action = file_menu->addAction(tr("Exit"));

    connect(load_action, &QAction::triggered, this, &MainWindow::LoadFile);
    connect(exit_action, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::CreateEmulationMenu() {
    QMenu* emulation_menu = menuBar()->addMenu(tr("Emulation"));

    pause_action = emulation_menu->addAction(tr("Pause"));

    connect(pause_action, &QAction::triggered, this, [this]() {
        if (core.GetState() == CoreState::Running) {
            core.SetState(CoreState::Paused);
        } else {
            core.SetState(CoreState::Running);
        }
    });
}

void MainWindow::CreateViewMenu() {
    QMenu* view_menu = menuBar()->addMenu(tr("View"));
    QAction* debugger_action = view_menu->addAction(tr("Debugger"));

    connect(debugger_action, &QAction::triggered, this, &MainWindow::ShowDebuggerWindow);
}

void MainWindow::ShowDebuggerWindow() {
    debugger_window = new DebuggerWindow(core.system.ee_core);
    debugger_window->show();
}

void MainWindow::LoadFile() {
    QString file_name = QFileDialog::getOpenFileName(
        this, tr("Open File"),
        "../roms",
        tr("PS2 Games (*.elf)")
    );

    if (!file_name.isEmpty()) {
        core.Reset();
        core.SetGamePath(file_name.toStdString());
        core.SetState(CoreState::Running);
    }
}

void MainWindow::UpdateTitle(float fps) {

}