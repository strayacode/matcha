#pragma once

#include <QMainWindow>
#include "otterstation-qt/debugger/debuggerwindow.h"
#include "core/core.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();

private:
    void CreateMenubar();
    void CreateFileMenu();
    void CreateViewMenu();
    void ShowDebuggerWindow();
    void LoadFile();
    void UpdateTitle(float fps);

    DebuggerWindow debugger_window;
    
    Core core;
protected:
};