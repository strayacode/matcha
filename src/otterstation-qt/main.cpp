#include <QApplication>
#include <memory>
#include "otterstation-qt/mainwindow.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Straya");
    QCoreApplication::setApplicationName("otterstation");

    std::unique_ptr<MainWindow> window = std::make_unique<MainWindow>();
    window->show();
    return app.exec(); 
}
