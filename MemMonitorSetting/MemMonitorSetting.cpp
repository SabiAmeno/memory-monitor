#include "MemMonitorSetting.h"
#include <QMenu>
#include <QMessageBox>

MemMonitorSetting::MemMonitorSetting(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    setWindowIcon(QIcon(":/MemMonitorSetting/resource.png"));
    setWindowTitle(QString::fromLocal8Bit("内存监控"));
    _tray_icon = new QSystemTrayIcon(this);
    _tray_icon->setIcon(QIcon(":/MemMonitorSetting/resource.png"));
    _tray_icon->show();

    _pmm = new ProcMonitorManage();
    _pmm->SetPrinter(new FilePrinter());

    init();

    ui.treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.treeWidget->setColumnWidth(0, 30);
    ui.treeWidget->setColumnWidth(1, 30);

    _thr = new std::thread(&MemMonitorSetting::run, this);

    connect(ui.treeWidget, &QTreeWidget::customContextMenuRequested, this, &MemMonitorSetting::popMenu);
    connect(ui.checkBox, &QCheckBox::stateChanged, this, &MemMonitorSetting::onlySeeSelected);
    connect(ui.treeWidget, &QTreeWidget::itemClicked, this, &MemMonitorSetting::showProcInfo);
    connect(ui.actionOpenLog, &QAction::triggered, this, &MemMonitorSetting::showLog);
    connect(this, &MemMonitorSetting::procInvalid, this, &MemMonitorSetting::procInvalidProcess, Qt::QueuedConnection);
    connect(ui.actionRefresh, &QAction::triggered, this, &MemMonitorSetting::procRefresh);

    connect(_tray_icon, &QSystemTrayIcon::activated, this, &MemMonitorSetting::trayIconActived);
}

MemMonitorSetting::~MemMonitorSetting()
{
    std::lock_guard<std::mutex> _(_mtx);
}

void MemMonitorSetting::addToMonitor()
{
    QTreeWidgetItem* item = selectedItem();
    if (!item) return;

    item->setIcon(0, QIcon(":/MemMonitorSetting/sel.png"));
    ulong pid = item->data(2, Qt::UserRole).toULongLong();
    std::string pname = item->text(2).toStdString();

    {
        std::lock_guard<std::mutex> _lock(_mtx);
        _pmm->AddProc(ProcInfo{ pid, pname, 0 });
    }
    ui.label->setText(QString::number(_pmm->Procs().size()));
}

void MemMonitorSetting::removeFromMonitor()
{
    QTreeWidgetItem* item = selectedItem();
    if (!item) return;

    item->setIcon(0, QIcon());

    {
        std::lock_guard<std::mutex> _lock(_mtx);
        _pmm->RemoveProc(item->data(2, Qt::UserRole).toULongLong());
    }

    ui.label->setText(QString::number(_pmm->Procs().size()));

    if (ui.checkBox->isChecked()) {
        item->setHidden(true);
    }
}

void MemMonitorSetting::onlySeeSelected(int s)
{
    int count = ui.treeWidget->topLevelItemCount();

    if (ui.checkBox->checkState() == Qt::Checked) {
        for (int i = count - 1; i > -1; --i)
        {
            QTreeWidgetItem* item = ui.treeWidget->topLevelItem(i);
            int p = -1;
            if (!_pmm->InMonitor(item->data(2, Qt::UserRole).toULongLong(), p)) {
                item->setHidden(true);
            }
        }
    } else {
        for (int i = count - 1; i > -1; --i)
        {
            QTreeWidgetItem* item = ui.treeWidget->topLevelItem(i);
            item->setHidden(false);
        }
    }
}

void MemMonitorSetting::showProcInfo(QTreeWidgetItem* item, int column)
{
    DWORD pid = item->data(2, Qt::UserRole).toULongLong();
    QString name = item->text(2);
    
    ui.proc_shower->setProc(pid, name);
}

void MemMonitorSetting::showLog()
{
    ProcInfoLog pil;
    pil.show(_pmm);
    pil.exec();
}

void MemMonitorSetting::procInvalidProcess(DWORD pid)
{
    std::lock_guard<std::mutex> _lock(_mtx);
    _pmm->RemoveProc(pid);

    procRefresh();

    _tray_icon->showMessage(QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("进程 %1 已退出").arg(pid));
}

void MemMonitorSetting::procRefresh()
{
//    _pmm->Reset();
    int items = ui.treeWidget->topLevelItemCount();
    for (int i = items - 1; i > -1; --i)
        ui.treeWidget->takeTopLevelItem(i);
    
    init();
}

void MemMonitorSetting::killProcess()
{
    auto button = QMessageBox::information(
        nullptr,
        QString::fromLocal8Bit("提示"),
        QString::fromLocal8Bit("确定要终止该进程吗?"),
        QMessageBox::Ok | QMessageBox::Cancel);
    if (button != QMessageBox::Ok)
        return;

    auto item = selectedItem();
    if (!item)
        return;

    DWORD pid = item->data(2, Qt::UserRole).toULongLong();
    Proc proc(pid);
    proc.Terminate();

    int p = -1;
    if (_pmm->InMonitor(pid, p))
        _pmm->RemoveProc(pid);

    procRefresh();
}

void MemMonitorSetting::trayIconActived(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Unknown:
        break;
    case QSystemTrayIcon::Context:
    {
        QMenu menu;
        QAction* exit = menu.addAction("Exit");

        connect(exit, &QAction::triggered, this, [&]() {close(); });
        menu.move(cursor().pos());
        menu.exec();
        break;
    }
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger:
        show();
        raise();
        break;
    case QSystemTrayIcon::MiddleClick:
        break;
    default:
        break;
    }
}

void MemMonitorSetting::hideEvent(QHideEvent* e)
{
    hide();
}

void MemMonitorSetting::init()
{
    auto procs = Proc::GetProcs();
    QList<QTreeWidgetItem*> items;

    for (auto p : procs)
    {
        int pos = -1;
        auto item = new QTreeWidgetItem((QTreeWidget*)0, QStringList() << "" << "" << QString::fromStdString(p.second));
        if (_pmm->InMonitor(p.first, pos))
            item->setIcon(0, QIcon(":/MemMonitorSetting/sel.png"));
        Proc pro(p.first);
        item->setIcon(1, QIcon(pro.getIcon()));
        item->setData(2, Qt::UserRole, (unsigned long long)p.first);
        items.append(item);
    }
    ui.treeWidget->insertTopLevelItems(0, items);

    onlySeeSelected(Qt::Checked);
}

void MemMonitorSetting::run()
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> _lock(_mtx);
            auto procs = Proc::GetProcs();
            for (auto p : _pmm->Procs())
            {
                auto it = procs.find(p.procid);
                if (it == procs.end())
                {
                    emit procInvalid(p.procid);
                    continue;
                }
                Proc proc(p.procid);
                int pos = -1;

                if ((*_pmm).InMonitor(p.procid, pos)) {
                    ProcInfo& pi = (*_pmm)[pos];
                    pi.memory = proc.GetProcMemory();
                }
            }
            _pmm->Save();
        }
        Sleep(200);
    }
}

QTreeWidgetItem* MemMonitorSetting::selectedItem()
{
    auto sel_items = ui.treeWidget->selectedItems();
    if (sel_items.isEmpty())
        return NULL;

    return sel_items.at(0);
}

void MemMonitorSetting::popMenu(const QPoint& p)
{
    QTreeWidgetItem* item = selectedItem();
    if (!item) return;

    int pos = -1;
    QMenu menu;
    if (!_pmm->InMonitor(item->data(2, Qt::UserRole).toULongLong(), pos)) {
        QAction* atm = menu.addAction(QIcon(":/MemMonitorSetting/add.png"), QString::fromLocal8Bit("添加监控"));

        connect(atm, &QAction::triggered, this, &MemMonitorSetting::addToMonitor);
    } else {
        QAction* rtm = menu.addAction(QIcon(":/MemMonitorSetting/remove.png"), QString::fromLocal8Bit("移除监控"));

        connect(rtm, &QAction::triggered, this, &MemMonitorSetting::removeFromMonitor);
    }

    QAction* killp = menu.addAction(QIcon(":/MemMonitorSetting/terminate.png"), QString::fromLocal8Bit("终止进程"));
    connect(killp, &QAction::triggered, this, &MemMonitorSetting::killProcess);

    menu.move(cursor().pos());
    menu.exec();
}