#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include "gismapgraphicsubmapbutton.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

#define SUB_MAP_BUTTION_SIZE 96
#define SUB_MAP_BUTTION_IMG ":/images/default/map/map_map.png"
GisMapGraphicSubMapButton::GisMapGraphicSubMapButton(
        GisMapGraphWidget *graphWidget,
        QPointF position,
        int buttonId,
        QString buttonName)
        :GisMapGraphButton(graphWidget, position, buttonId, buttonName)
{
	connect(this,
        SIGNAL(signalPositionChanged(QPointF)),
		this,
        SLOT(slotPositionChanged(QPointF)));
}

QRectF GisMapGraphicSubMapButton::boundingRect() const
{
    qreal adjust = 10;
    return QRectF(SUB_MAP_BUTTION_SIZE/2*-1 - adjust,
                   SUB_MAP_BUTTION_SIZE/2*-1 - adjust,
                   SUB_MAP_BUTTION_SIZE + adjust,
                   SUB_MAP_BUTTION_SIZE + adjust);

}

void GisMapGraphicSubMapButton::paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setPen(QPen(Qt::black, 0));
    painter->setRenderHint(QPainter::Antialiasing, true); //抗锯齿
    painter->setRenderHint(QPainter::TextAntialiasing, true); //字体抗锯齿
    QRectF target(-32.0, -32.0, 64.0, 64.0);
    QRectF source(0.0, 0.0, 64.0, 64.0);
    QImage image(SUB_MAP_BUTTION_IMG);
    painter->drawImage(target, image, source);

    QString buttonName = getButtonName();
    if(!buttonName.isEmpty())
    {
        QRectF text(-32.0, 20.0, 64.0, 32);
        //painter->drawText(text, buttonName);
        painter->drawText(text, Qt::AlignCenter, buttonName);
    }
}

void GisMapGraphicSubMapButton::slotPositionChanged(QPointF position)
{
    emit signalSubMapInfoChanged(getButtonId(), position);
}

QPointF GisMapGraphicSubMapButton::getPosition()
{
    QPointF point = scenePos();
    return point;
}

void GisMapGraphicSubMapButton::movePosition(QPointF newPoint)
{
    setPos(newPoint);
}
