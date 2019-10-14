#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // DPI support  https://stackoverflow.com/questions/24367355/automatic-rescaling-of-an-application-on-high-dpi-windows-platform
    QApplication a(argc, argv);
    //a.setStyle("fusion");
    MainWindow w;
    w.show();

    return a.exec();
}
