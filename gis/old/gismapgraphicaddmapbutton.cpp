#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include "gismapgraphicaddmapbutton.h"

#define ADD_IPC_BUTTION_SIZE 166
#define ADD_MAP_BUTTION_IMG ":/images/default/layoutmanage/null.png"
#define ADD_MAP_BUTTION_HOVER_IMG ":/images/default/layoutmanage/null_hover.png"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

GisMapGraphicAddMapButton::GisMapGraphicAddMapButton(
        GisMapGraphWidget *graphWidget,
        QPoint position,
        int channelId,
        QString channelName)
    :GisMapGraphButton(graphWidget, position, channelId, channelName)
{
    //不让拖动
    setDragAble(false);
    //设置接收悬浮事件
    //setAcceptHoverEvents(true);
}

QRectF GisMapGraphicAddMapButton::boundingRect() const
{
    qreal adjust = 10;
    return QRectF( ADD_IPC_BUTTION_SIZE/2*-1,
                   ADD_IPC_BUTTION_SIZE/2*-1,
                   ADD_IPC_BUTTION_SIZE,
                   ADD_IPC_BUTTION_SIZE);
}

void GisMapGraphicAddMapButton::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
    painter->setPen(QPen(Qt::black, 0));
    painter->setRenderHint(QPainter::Antialiasing, true); //抗锯齿
    painter->setRenderHint(QPainter::TextAntialiasing, true); //字体抗锯齿
    if((option->state & QStyle::State_Sunken))
    {
        QRectF target(ADD_IPC_BUTTION_SIZE/2*-1,
                      ADD_IPC_BUTTION_SIZE/2*-1,
                      ADD_IPC_BUTTION_SIZE,
                      ADD_IPC_BUTTION_SIZE);
        QRectF source(0.0, 0.0, ADD_IPC_BUTTION_SIZE, ADD_IPC_BUTTION_SIZE);
        QImage image(ADD_MAP_BUTTION_HOVER_IMG);
        painter->drawImage(target, image, source);
    }
    else
    {
        QRectF target(ADD_IPC_BUTTION_SIZE/2*-1,
                      ADD_IPC_BUTTION_SIZE/2*-1,
                      ADD_IPC_BUTTION_SIZE,
                      ADD_IPC_BUTTION_SIZE);
        QRectF source(0.0, 0.0, ADD_IPC_BUTTION_SIZE, ADD_IPC_BUTTION_SIZE);
        QImage image(ADD_MAP_BUTTION_IMG);
        painter->drawImage(target, image, source);
    }
}
