#include <QHeaderView>
#include <QFontDatabase>
#include "otterstation-qt/debugger/debuggerwindow.h"
#include "core/ee/ee_core.h"

DebuggerWindow::DebuggerWindow(EECore& ee_core) : ee_core(ee_core) {
    setMinimumSize(640, 480);
    this->setWindowTitle("otterstation Debugger");

    registers_view = new QTableView;
    registers_model = new RegistersModel(this, ee_core);

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    font.setFamily("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    registers_view->setFont(font);
    registers_view->setModel(registers_model);
    registers_view->setColumnWidth(1, 200);

    QHeaderView* vertical_header = registers_view->verticalHeader();

    vertical_header->setDefaultSectionSize(vertical_header->fontMetrics().height());

    QWidget *window = new QWidget;
    layout = new QHBoxLayout(window);

    layout->addWidget(registers_view);
    setCentralWidget(window);
}