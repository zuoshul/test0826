#ifndef GISMAPGRAPHICSUBMAPBUTTON_H
#define GISMAPGRAPHICSUBMAPBUTTON_H
#include "gismapgraphbutton.h"

class GisMapGraphicSubMapButton:public GisMapGraphButton
{
    Q_OBJECT
public:
    GisMapGraphicSubMapButton(GisMapGraphWidget *graphWidget,QPointF position,int buttonId = 0,QString buttonName = QString(""));

    QRectF boundingRect() const override;

    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget) override;

    int type() const override
    {
        return CustomGraphItemSubMapButton;
    }

    QPointF getPosition();

    void movePosition(QPointF newPoint);

private slots:
    void slotPositionChanged(QPointF position);

signals:
    void signalSubMapInfoChanged(int mapId, QPointF position);
};

#endif // GISMAPGRAPHICSUBMAPBUTTON_H
