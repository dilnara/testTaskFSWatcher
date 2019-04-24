#include "ui/mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/icon.ico"));

    MainWindow w;
    w.show();
    w.requestDirectory();

    return a.exec();
}
