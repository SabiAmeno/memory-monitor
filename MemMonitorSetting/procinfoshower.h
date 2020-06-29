#pragma once

#include <QWidget>
#include <QDialog>
#include <QTimer>
#include "ui_procinfoshower.h"
#include "mem.h"
#include <QTextBrowser>

class ProcInfoShower : public QWidget
{
    Q_OBJECT

public:
    ProcInfoShower(QWidget *parent = Q_NULLPTR);
    ~ProcInfoShower();

    void setProc(ulong pid, const QString& name);

private slots:
    void timeout();
private:
    Ui::procinfoshower ui;

    QTimer _timer;
    Proc* _proc;
};

class ProcInfoLog : public QDialog
{
    Q_OBJECT
public:
    ProcInfoLog(QWidget* parent = 0);

    void show(ProcMonitorManage* pmm);
private:
    QTextBrowser* _tb;
};