#include "MemMonitorSetting.h"
#include <QMenu>

MemMonitorSetting::MemMonitorSetting(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    _pmm = new ProcMonitorManage();
    _pmm->SetPrinter(new FilePrinter());

    init();

    ui.treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.treeWidget->setColumnWidth(0, 50);

    _thr = new std::thread(&MemMonitorSetting::run, this);

    connect(ui.treeWidget, &QTreeWidget::customContextMenuRequested, this, &MemMonitorSetting::popMenu);
    connect(ui.checkBox, &QCheckBox::stateChanged, this, &MemMonitorSetting::onlySeeSelected);
}

void MemMonitorSetting::addToMonitor()
{
    QTreeWidgetItem* item = selectedItem();
    if (!item) return;

    item->setIcon(0, QIcon(":/MemMonitorSetting/sel.png"));
    ulong pid = item->text(1).toULong();
    std::string pname = item->text(2).toStdString();

    std::lock_guard<std::mutex> _lock(_mtx);
    _pmm->AddProc(ProcInfo{ pid, pname, 0 });

    ui.label->setText(QString::number(_pmm->Procs().size()));
}

void MemMonitorSetting::removeFromMonitor()
{
    QTreeWidgetItem* item = selectedItem();
    if (!item) return;

    item->setIcon(0, QIcon());

    std::lock_guard<std::mutex> _lock(_mtx);
    _pmm->RemoveProc(item->text(1).toULong());

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
            if (!_pmm->InMonitor(item->text(1).toULong(), p)) {
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

void MemMonitorSetting::init()
{
    auto procs = Proc::GetProcs();
    QList<QTreeWidgetItem*> items;

    for (auto p : procs)
    {
        items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList() << QString() << QString::number(p.first) << QString::fromStdString(p.second)));
    }
    ui.treeWidget->insertTopLevelItems(0, items);
}

void MemMonitorSetting::run()
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> _lock(_mtx);
            for (auto p : _pmm->Procs())
            {
                int pos = -1;
                Proc proc(p.procid);
                if (proc.IsValid() && (*_pmm).InMonitor(p.procid, pos)) {
                    ProcInfo& pi = (*_pmm)[pos];
                    pi.memory = proc.GetProcMemory();
                }
            }
        }
        _pmm->Save();
        Sleep(2000);
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
    if (!_pmm->InMonitor(item->text(1).toULong(), pos)) {
        QAction* atm = menu.addAction("add to monitor");

        connect(atm, &QAction::triggered, this, &MemMonitorSetting::addToMonitor);
    } else {
        QAction* rtm = menu.addAction("remove from monitor");

        connect(rtm, &QAction::triggered, this, &MemMonitorSetting::removeFromMonitor);
    }

    menu.move(mapToGlobal(p));
    menu.exec();
}