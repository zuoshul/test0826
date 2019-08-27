#ifndef MAPIPCCONNECTIONLINE_H
#define MAPIPCCONNECTIONLINE_H
#include <QGraphicsItem>
#include <QColor>
#include <QList>
#include "src/base/customgraphitem.h"
class MapGraphWidget;

class MapIpcConnectionLine: public QGraphicsItem
{
public:
    MapIpcConnectionLine(
            MapGraphWidget *graphWidget,
            QList<QPoint>& points
            );


protected:
    //QGraphicsView uses this to determine whether the item requires redrawing.
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;


private:
    MapGraphWidget *m_pMapWidget;
    QList<QPoint> m_objPoints;
};

#endif // MAPIPCCONNECTIONLINE_H
