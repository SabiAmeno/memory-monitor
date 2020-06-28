#pragma once

#include <QtWidgets/QDialog>
#include "ui_MemMonitorSetting.h"
#include "mem.h"
#include <mutex>

class MemMonitorSetting : public QMainWindow
{
    Q_OBJECT

public:
    MemMonitorSetting(QWidget *parent = Q_NULLPTR);

private slots:
    void popMenu(const QPoint& p);
    void addToMonitor();
    void removeFromMonitor();
    void onlySeeSelected(int s);
private:
    void init();

    void run();

    QTreeWidgetItem* selectedItem();
private:
    Ui::MemMonitorSettingClass ui;

    ProcMonitorManage* _pmm;

    std::thread* _thr;
    std::mutex _mtx;
};
