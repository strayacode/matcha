#pragma once

#include <QMainWindow>
#include <QTableView>
#include <QHBoxLayout>
#include "otterstation-qt/debugger/registersmodel.h"

class EECore;

class DebuggerWindow : public QMainWindow {
    Q_OBJECT
public:
    DebuggerWindow(EECore& ee_core);

    QTableView* registers_view = nullptr;
    RegistersModel* registers_model = nullptr;
    QHBoxLayout *layout = nullptr;

    EECore& ee_core;

private:
protected:
};