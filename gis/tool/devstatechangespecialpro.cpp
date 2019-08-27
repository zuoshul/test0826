#include "devstatechangespecialpro.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

DevStateChangeSpecialPro::DevStateChangeSpecialPro(QWidget *parent) : QObject(parent),m_pTimer(NULL)
{
    m_pTimer = new QTimer(this);
    connect(m_pTimer,SIGNAL(timeout()), this, SLOT(slotTimeout()));
    connect(g_SppClient, SIGNAL(signalDeviceInfoChanged(InviewDevice, InviewOperatorType)),this, SLOT(slot_signalDeviceInfoChanged(InviewDevice, InviewOperatorType)));
}

DevStateChangeSpecialPro::~DevStateChangeSpecialPro()
{
    if(m_pTimer)
    {
        m_pTimer->stop();
        m_pTimer->deleteLater();
        m_pTimer = 0;
    }
}

void DevStateChangeSpecialPro::slot_signalDeviceInfoChanged(InviewDevice device, InviewOperatorType operatorType)
{
    //忽略2秒内的变化
    m_pTimer->start(2000);
}

void DevStateChangeSpecialPro::slotTimeout()
{
    m_pTimer->stop();
    emit signalDeviceInfoChanged();
}
