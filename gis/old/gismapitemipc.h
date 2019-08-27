#ifndef GISMAPITEMIPC_H
#define GISMAPITEMIPC_H

#include <QObject>
#include <QGraphicsTextItem>
#include <QTimerEvent>
#include "gismapgraphbutton.h"
#include "gismapgraphwidget.h"
#include "gismapmonitorarea.h"

class GisMapItemIpc:public QObject
{
    Q_OBJECT

public:
    enum ShowParam
    {
        ShowDevOnly,
        ShowDevAndArea,
        NotShowAny
    };

public:
    GisMapItemIpc(GisMapGraphWidget* map,
               QPointF centerPosition,
               QPoint firstPosition,
               QPoint secondPosition,
               int channelId = 0,
               QString buttonName = QString(""));

    GisMapItemIpc(GisMapGraphWidget* map,
               QPointF centerPosition,
               int channelId,
               QString buttonName);

    virtual ~GisMapItemIpc();

    //手动把MapItemIpc中的QGraphicsItem从sence中删除
    void releaseMapItemIpc();

    void setDragAble(bool dragAble);

    //搜索报警后，不停闪烁.
    //FlickerDuration 闪烁多少秒
    void startAlarmFlicker(int flickerDuration = 10);

    void stopAlarmFlicker();

    void timerEvent(QTimerEvent *event);

    //获取窗口坐标
    void getCenterPoint(int &channelId,QPointF &point);

    void moveCenterPoint(QPointF newPoint);

    void showDeviceObj(ShowParam param);

private slots:
    void slotCenterPositionChanged(QPointF position);

    void slotFirstPointPositionChanged(QPointF position);

    void slotSecondPointPositionChanged(QPointF position);

    void slotRightButtonDoubleClicked(int);

    void slotButtonPress(int channelId);

signals:
    void signalIpcDoubleClicked(int channelId);

    void signalIpcInfoChanged(int channelId, QPointF position, QPoint point1, QPoint point2);

    void signalIpcButtonPress(int channelId);

private:
    GisMapGraphWidget* m_pMap;
    GisMapGraphButton* m_pCenterPoint;
    GisMapGraphButton* m_pFirstPoint;
    GisMapGraphButton* m_pSecondPoint;
    GisMapMonitorArea* m_pMonitorArea;
    QGraphicsTextItem* m_ipcName;

    QPointF m_objCenterOldPosition;
    QPointF m_objFirstOldPosition;
    QPointF m_objSecondOldPosition;

    int m_nChannelId;
    int m_nAlarmFlicker;
    int m_FlickerDuration;  //单位:秒
};

#endif // GISMAPITEMIPC_H
