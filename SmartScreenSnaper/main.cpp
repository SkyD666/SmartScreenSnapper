#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    if (argc > 1 && a.arguments().at(1) == "-autorun") {
        w.showMinimized();      //先showMinimized，再setWindowState(Qt::WindowNoState)，避免闪烁一下后隐藏
        w.setVisible(false);
        w.setWindowState(Qt::WindowNoState);
    } else {
        w.show();
    }
    a.setApplicationName("SmartScreenSnaper");
    return a.exec();
}
