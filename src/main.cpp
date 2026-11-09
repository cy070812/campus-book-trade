#include "mainwindow.h"
#include "welcome.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setFont(QFont("Microsoft YaHei", 9));
    Welcome w;
    w.show();
    return a.exec();
}