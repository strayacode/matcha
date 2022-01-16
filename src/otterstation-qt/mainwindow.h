#pragma once

#include <QMainWindow>
#include "core/core.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();

private:
    void CreateMenubar();
    void CreateFileMenu();
    void CreateEmulationMenu();
    void LoadFile();
    void UpdateTitle(float fps);

    QAction* pause_action;
    Core core;

protected:
};