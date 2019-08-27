#include "gisareaselectobj.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

GisAreaSelectObj::GisAreaSelectObj(GisMapGraphWidget *graphWidget) : m_pMap(graphWidget),m_isDrawing(false)
{
    if(NULL != m_pMap)
    {
        m_pMap->scene()->addItem(this);
    }
    setFlag(ItemIsFocusable);
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemIsMovable, false);
    setCacheMode(DeviceCoordinateCache);
    setZValue(999);
}

QRectF GisAreaSelectObj::boundingRect() const
{
    return m_pMap->scene()->sceneRect();
}

void GisAreaSelectObj::updateAreaParam(QPointF start,QPointF end,bool isDrawing)
{
    if(!start.isNull())
    {
        m_startPoint = start;
        m_endPoint = end;
    }
    if(!end.isNull())
    {
        m_endPoint = end;
    }
    m_isDrawing = isDrawing;
    update();
}

void GisAreaSelectObj::paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *)
{
    Q_UNUSED(option);
    if(m_startPoint.isNull())
    {
        return;
    }
    painter->setPen(QPen(Qt::red, 2));
    painter->setRenderHint(QPainter::Antialiasing, true); //抗锯齿
    painter->setBrush(Qt::transparent);

    int x = m_startPoint.x();
    int y = m_startPoint.y();
    int w = m_endPoint.x() - x;
    int h = m_endPoint.y() - y;

    if(m_isDrawing)
    {
        if(!m_endPoint.isNull())
        {
            painter->drawRect(x,y,w,h);
        }
    }
    else
    {
        m_startPoint = QPointF();
        m_endPoint = QPointF();
        m_isDrawing = false;
        painter->drawRect(x,y,w,h);
        emit signalSelectFinished(QRectF(x,y,w,h));
    }
}


