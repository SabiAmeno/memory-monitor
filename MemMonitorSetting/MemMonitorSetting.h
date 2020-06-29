#pragma once

#include <QMainWindow>
#include "ui_MemMonitorSetting.h"
#include "mem.h"
#include <mutex>
#include <QSystemTrayIcon>

class MemMonitorSetting : public QMainWindow
{
    Q_OBJECT

public:
    MemMonitorSetting(QWidget *parent = Q_NULLPTR);

signals:
    void procInvalid(DWORD pid);
private slots:
    void popMenu(const QPoint& p);
    void addToMonitor();
    void removeFromMonitor();
    void onlySeeSelected(int s);
    void showProcInfo(QTreeWidgetItem* item, int column);
    void showLog();
    void procInvalidProcess(DWORD pid);
    void procRefresh();
    void killProcess();

    void trayIconActived(QSystemTrayIcon::ActivationReason);

protected:
    void hideEvent(QHideEvent* e);
private:
    void init();

    void run();

    QTreeWidgetItem* selectedItem();
private:
    Ui::MemMonitorSettingClass ui;

    ProcMonitorManage* _pmm;

    std::thread* _thr;
    std::mutex _mtx;

    QSystemTrayIcon* _tray_icon;
};
