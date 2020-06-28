#include "MemMonitorSetting.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MemMonitorSetting w;
    w.show();
    return a.exec();
}
