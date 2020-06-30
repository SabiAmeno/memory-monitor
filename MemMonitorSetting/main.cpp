#include "MemMonitorSetting.h"
#include <QtWidgets/QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    MemMonitorSetting w;
    w.show();
    return a.exec();
}
