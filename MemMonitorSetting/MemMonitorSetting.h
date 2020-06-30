#pragma once

#include <QMainWindow>
#include "ui_MemMonitorSetting.h"
#include "mem.h"
#include <mutex>
#include <QSystemTrayIcon>

template<typename T, size_t N = 400>
class DataRing
{
public:
    DataRing() : _head_pos(0), _is_cover(false), _data{ 0 }{ }

    void append(const T& d) 
    {
        int su = _head_pos / N;
        if (su)
            _is_cover = true;

        _head_pos %= N;
        _data[_head_pos] = d;
        _head_pos++;
    }

    int size()
    {
        if (_is_cover)
            return N;

        return _head_pos;
    }

    std::vector<T> data()
    {
        int cp_size = 0;
        std::vector<T> d;
        
        if (_is_cover) {
            d.resize(N);
            cp_size = N - _head_pos;
            std::copy_n(_data + _head_pos, N - _head_pos, d.begin());
        } else 
            d.resize(_head_pos);
        
        std::copy_n(_data, _head_pos, d.begin() + cp_size);

        return d;
    }

    bool isEmpty()
    {
        return _head_pos == 0 && !_is_cover;
    }
private:
    T _data[N];

    int _head_pos;
    bool _is_cover;
};

class HistoryData
{
public:
    HistoryData() {}

    void remove(unsigned long pid)
    {
        auto it = _data.find(pid);
        if (it != _data.end())
            _data.erase(it);
    }

    void add(unsigned long pid) {
        _data.insert(std::make_pair(pid, DataRing<double>()));
    }

    void appendData(unsigned long pid, double d)
    {
        auto& _d = _data[pid];
        _d.append(d);
    }

    DataRing<double> data(unsigned long pid) const
    {
        auto it = _data.find(pid);
        if (it == _data.end())
            return DataRing<double>();

        return it->second;
    }
private:
    std::map<unsigned long, DataRing<double> > _data;
};

class MemMonitorSetting : public QMainWindow
{
    Q_OBJECT

public:
    MemMonitorSetting(QWidget *parent = Q_NULLPTR);
    ~MemMonitorSetting();
signals:
    void procInvalid(DWORD pid);
    void shotMemData(unsigned long, double);
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
    void showMemGraph(unsigned long pid);
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

    HistoryData _hisd;
};
