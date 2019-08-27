#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include "gismapgraphipcbutton.h"
#include "sppclient.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

#define IPC_BUTTION_SIZE 64
GisMapGraphIpcButton::GisMapGraphIpcButton(
        GisMapGraphWidget *graphWidget,
        QPointF position,
        int channelId,
        QString channelName)
    :GisMapGraphButton(graphWidget, position, channelId, channelName)
{
    InviewChannel* pChannel = g_SppClient->getChannelById(channelId);
    if(pChannel)
    {
        int typeId = pChannel->get_channel_type_id();
        if(typeId == NoteTypeIPC)
        {
             m_iconPath.normalIconPath = ":/images/default/map/map_device.png";
             m_iconPath.alarmIconPath = ":/images/default/map/map_device_alarm.png";
             m_iconPath.alarmIconPath2 = ":/images/default/map/map_device_alarm2.png";
             m_iconPath.curIconPath = m_iconPath.normalIconPath;
        }
        else if(typeId == NodeTypeFaceDetect)
        {
            m_iconPath.normalIconPath = ":/images/default/map/map_deviceFaceDetect.png";
            m_iconPath.alarmIconPath = ":/images/default/map/map_deviceFaceDetect_alarm.png";
            m_iconPath.alarmIconPath2 = ":/images/default/map/map_deviceFaceDetect_alarm2.png";
            m_iconPath.curIconPath = m_iconPath.normalIconPath;
        }
        else if(typeId == NodeTypeThermoDetector)
        {
            m_iconPath.normalIconPath = ":/images/default/map/map_deviceThermoDetector.png";
            m_iconPath.alarmIconPath = ":/images/default/map/map_deviceThermoDetector_alarm.png";
            m_iconPath.alarmIconPath2 = ":/images/default/map/map_deviceThermoDetector_alarm2.png";
            m_iconPath.curIconPath = m_iconPath.normalIconPath;
        }
        else if(typeId == NodeTypeFourEye)
        {
            //四目
            m_iconPath.normalIconPath = ":/images/default/map/map_device.png";
            m_iconPath.alarmIconPath = ":/images/default/map/map_device_alarm.png";
            m_iconPath.alarmIconPath2 = ":/images/default/map/map_device_alarm2.png";
            m_iconPath.curIconPath = m_iconPath.normalIconPath;
        }
        else if(typeId == NodeTypeOnvifFishEye)
        {
            //鱼眼
            m_iconPath.normalIconPath = ":/images/default/map/map_device.png";
            m_iconPath.alarmIconPath = ":/images/default/map/map_device_alarm.png";
            m_iconPath.alarmIconPath2 = ":/images/default/map/map_device_alarm2.png";
            m_iconPath.curIconPath = m_iconPath.normalIconPath;
        }
        else if(typeId == NodeTypeThermalImage)
        {
            //热成像
            m_iconPath.normalIconPath = ":/images/default/map/map_device.png";
            m_iconPath.alarmIconPath = ":/images/default/map/map_device_alarm.png";
            m_iconPath.alarmIconPath2 = ":/images/default/map/map_device_alarm2.png";
            m_iconPath.curIconPath = m_iconPath.normalIconPath;
        }
        else if(typeId == NodeTypeOtherDevice)
        {
            m_iconPath.normalIconPath = ":/images/default/map/map_deviceOtherDevice.png";
            m_iconPath.alarmIconPath = ":/images/default/map/map_deviceOtherDevice_alarm.png";
            m_iconPath.alarmIconPath2 = ":/images/default/map/map_deviceOtherDevice_alarm2.png";
            m_iconPath.curIconPath = m_iconPath.normalIconPath;
        }
        else if(typeId == NodeTypeOnvifDevice)
        {
            //Onvif
            m_iconPath.normalIconPath = ":/images/default/map/map_device.png";
            m_iconPath.alarmIconPath = ":/images/default/map/map_device_alarm.png";
            m_iconPath.alarmIconPath2 = ":/images/default/map/map_device_alarm2.png";
            m_iconPath.curIconPath = m_iconPath.normalIconPath;
        }
        else if(typeId == NodeTypeIPdome)
        {
            //球机
            m_iconPath.normalIconPath = ":/images/default/map/map_deviceIPdome.png";
            m_iconPath.alarmIconPath = ":/images/default/map/map_deviceIPdome_alarm.png";
            m_iconPath.alarmIconPath2 = ":/images/default/map/map_deviceIPdome_alarm2.png";
            m_iconPath.curIconPath = m_iconPath.normalIconPath;
        }
    }
}

QRectF GisMapGraphIpcButton::boundingRect() const
{
    qreal adjust = 10;
    return QRectF( IPC_BUTTION_SIZE/2*-1 - adjust,
                   IPC_BUTTION_SIZE/2*-1 - adjust,
                   IPC_BUTTION_SIZE + adjust,
                   IPC_BUTTION_SIZE + adjust);
}

void GisMapGraphIpcButton::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option,
           QWidget *)
{
    painter->setPen(QPen(Qt::black, 0));
    painter->setRenderHint(QPainter::Antialiasing, true); //抗锯齿
    painter->setRenderHint(QPainter::TextAntialiasing, true); //字体抗锯齿
    QRectF target(-32.0, -32.0, 64.0, 64.0);
    QRectF source(0.0, 0.0, 64.0, 64.0);

    QString imgPath = m_iconPath.curIconPath;
    QImage image(imgPath);
    painter->drawImage(target, image, source);
}

void GisMapGraphIpcButton::startAlarm()
{
    if(m_iconPath.curIconPath == m_iconPath.normalIconPath)
    {
        m_iconPath.curIconPath = m_iconPath.alarmIconPath;
    }
    else if(m_iconPath.curIconPath == m_iconPath.alarmIconPath)
    {
        m_iconPath.curIconPath = m_iconPath.alarmIconPath2;
    }
    else if(m_iconPath.curIconPath == m_iconPath.alarmIconPath2)
    {
        m_iconPath.curIconPath = m_iconPath.alarmIconPath;
    }
    update();
}

void GisMapGraphIpcButton::stopAlarm()
{
    m_iconPath.curIconPath = m_iconPath.normalIconPath;
    update();
}
