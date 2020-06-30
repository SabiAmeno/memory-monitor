#include "procinfoshower.h"

ProcInfoShower::ProcInfoShower(QWidget *parent)
    : QWidget(parent), _proc(0)
{
    ui.setupUi(this);

    _timer.setInterval(2000);

    connect(&_timer, &QTimer::timeout, this, &ProcInfoShower::timeout);
    connect(ui.toolButton, &QToolButton::clicked, this, [&] {emit showGraph(_pid); });
}

ProcInfoShower::~ProcInfoShower()
{
    if (_proc)
        delete _proc;
}

void ProcInfoShower::setProc(ulong pid, const QString& name)
{
    _pid = pid;
    _timer.stop();
    if(_proc)
        delete _proc;
    _proc = new Proc(pid);

    ui.label_pid->setText(QString::number(pid));
    ui.label_name->setText(name);
    timeout();

    _timer.start();
}

void ProcInfoShower::timeout()
{
    double mem = _proc->GetProcMemory();
    ui.label_mem->setText(QString::number(mem) + " MB");
}


/////////////////////////////////////////////

ProcInfoLog::ProcInfoLog(QWidget* parent)
    : QDialog(parent)
{
    _tb = new QTextBrowser(this);

    QVBoxLayout* ly = new QVBoxLayout();
    ly->addWidget(_tb);

    setLayout(ly);
}

void ProcInfoLog::show(ProcMonitorManage* pmm)
{
    ProcInfos procs = pmm->Procs();
    std::string msg;

    for (auto pi : procs)
    {
        char buffer[1024] = { 0 };
        sprintf_s(buffer, "%5d[%s] => [%f]\n", pi.procid, pi.procname.c_str(), pi.memory);
        msg += buffer;
    }

    _tb->setText(QString::fromStdString(msg));
}
