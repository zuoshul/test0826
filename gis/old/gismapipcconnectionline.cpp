#include <math.h>
#include <QPainter>
#include <QGraphicsScene>
#include <QRectF>
#include <QLine>
#include <QDebug>
#include "mapgraphwidget.h"
#include "mapipcconnectionline.h"

MapIpcConnectionLine::MapIpcConnectionLine(
        MapGraphWidget *graphWidget,
        QList<QPoint>& points
        )
    :m_pMapWidget(graphWidget),
    m_objPoints(points)
{
    if(NULL != m_pMapWidget)
    {
        m_pMapWidget->scene()->addItem(this);
    }
}

//QGraphicsView 会根据这个返回值来判断哪些区域需要重新绘制，必须覆盖整个地图，
//否则监控的反省区域操作这个rect时，会出现拖影
QRectF MapIpcConnectionLine::boundingRect() const
{
    if (m_objPoints.isEmpty()
            || m_objPoints.size()<=1)
        return QRectF();

    //比地图的实际区域大一些，避免靠近地图边缘时出现拖影
    qreal adjust = 100;

    //计算boundingRect的范围，这个范围是整个地图。坐标系查看cpp顶部注释
    int topLeftX = m_pMapWidget->width()/2*-1;
    int topLeftY = m_pMapWidget->height()/2*-1;
    QRectF rectF(topLeftX - adjust,
                          topLeftY - adjust,
                          m_pMapWidget->width() + adjust,
                          m_pMapWidget->height() + adjust);

    //qDebug() << "boundingRect " << rectF;
    return rectF;
}

void MapIpcConnectionLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (m_objPoints.isEmpty()
            || m_objPoints.size()<=1)
        return;


    QVector<QLineF> lines;
    for(int i=0; i<m_objPoints.size()-1; i++)
    {
        lines.append(QLineF(m_objPoints.at(i), m_objPoints.at(i+1)));
    }

    painter->setBrush( QBrush(Qt::green));
    QPen pen(Qt::green);
    pen.setWidth(5);
    painter->setRenderHint(QPainter::Antialiasing, true); //抗锯齿
    painter->setRenderHint(QPainter::TextAntialiasing, true); //字体抗锯齿
    painter->setPen(pen);
    painter->drawLines(lines);

    this->setZValue(-99);

}

