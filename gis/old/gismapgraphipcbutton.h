#ifndef GISMAPGRAPHIPCBUTTON_H
#define GISMAPGRAPHIPCBUTTON_H
#include "gismapgraphbutton.h"

struct GisIconPath
{
    GisIconPath()
    {
        normalIconPath = ":/images/default/map/map_device.png";
        alarmIconPath = ":/images/default/map/map_device_alarm.png";
        alarmIconPath2 = ":/images/default/map/map_device_alarm2.png";
        curIconPath = ":/images/default/map/map_device.png";
    }
    QString normalIconPath;  //正常图标
    QString alarmIconPath;   //报警图标
    QString alarmIconPath2;  //报警图标2
    QString curIconPath;     //当前图标
};

class GisMapGraphIpcButton: public GisMapGraphButton
{
public:
    GisMapGraphIpcButton(
            GisMapGraphWidget *graphWidget,
            QPointF position,
            int channelId = 0,
            QString channelName = QString(""));
    int type() const override
    {
        return CustomGraphItemIpcButton;
    }
    QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void startAlarm();

    void stopAlarm();

private:
    GisIconPath m_iconPath;
};

#endif // GISMAPGRAPHIPCBUTTON_H
