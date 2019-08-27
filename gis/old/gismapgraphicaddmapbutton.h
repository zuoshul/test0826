#ifndef GISMAPGRAPHICADDMAPBUTTON_H
#define GISMAPGRAPHICADDMAPBUTTON_H
#include "gismapgraphbutton.h"

class GisMapGraphicAddMapButton: public GisMapGraphButton
{
public:
    GisMapGraphicAddMapButton(
            GisMapGraphWidget *graphWidget,
            QPoint position,
            int channelId = 0,
            QString channelName = QString(""));

    int type() const override
    {
        return CustomGraphItemAddMapButton;
    }

    QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
};

#endif // GISMAPGRAPHICADDMAPBUTTON_H
