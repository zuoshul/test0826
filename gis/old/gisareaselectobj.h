#ifndef GISAREASELECTOBJ_H
#define GISAREASELECTOBJ_H

#include <QGraphicsObject>
#include "gismapgraphwidget.h"

QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
QT_END_NAMESPACE

class GisAreaSelectObj : public QGraphicsObject
{
    Q_OBJECT
public:
    GisAreaSelectObj(GisMapGraphWidget *graphWidget);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void updateAreaParam(QPointF start,QPointF end,bool isDrawing);

signals:
    void signalSelectFinished(QRectF rect);

private:
    GisMapGraphWidget *m_pMap;
    QPointF m_startPoint;
    bool m_isDrawing;
    QPointF m_endPoint;
};

#endif // GISAREASELECTOBJ_H
