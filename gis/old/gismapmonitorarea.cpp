/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <math.h>
#include <QPainter>
#include <QGraphicsScene>
#include <QRectF>
#include <QLine>
#include <QDebug>
#include "gismapmonitorarea.h"
#include "gismapgraphwidget.h"
/********************************
            -y
            ^
            |
            |
            |
            |(0,0)
-x ---------------------> x
            |
            |
            |
            |
            y
************************************/
static const double Pi = 3.14159265358979323846264338327950288419717;
static double TwoPi = 2.0 * Pi;

//! [0]
GisMapMonitorArea::GisMapMonitorArea(GisMapGraphWidget *graphWidget,
                         QGraphicsItem* centerPoint,
                         QGraphicsItem* point1,
                         QGraphicsItem* point2)
    :m_pMapWidget(graphWidget),
     m_pCenterPoint(centerPoint),
     m_pPoint1(point1),
     m_pPoint2(point2),
     m_objAreaColor("blue")
{
    setAcceptedMouseButtons(0);
    if(NULL != m_pMapWidget)
    {
        m_pMapWidget->scene()->addItem(this);
    }
}

void GisMapMonitorArea::adjust()
{
    if (!m_pCenterPoint || !m_pPoint1 || !m_pPoint2)
    {
        return;
    }

    //使MonitorArea立即repiant
    prepareGeometryChange();
}

//QGraphicsView 会根据这个返回值来判断哪些区域需要重新绘制，必须覆盖整个地图，
//否则监控的反省区域操作这个rect时，会出现拖影
QRectF GisMapMonitorArea::boundingRect() const
{
    if (!m_pCenterPoint || !m_pPoint1 || !m_pPoint2)
    {
        return QRectF();
    }

    //比地图的实际区域大一些，避免靠近地图边缘时出现拖影
    qreal adjust = 100;

    //计算boundingRect的范围，这个范围是整个地图。坐标系查看cpp顶部注释
    int topLeftX = m_pMapWidget->width()/2*-1;
    int topLeftY = m_pMapWidget->height()/2*-1;
    QRectF rectF(topLeftX - adjust,
                  topLeftY - adjust,
                  m_pMapWidget->width() + adjust,
                  m_pMapWidget->height() + adjust);

    return rectF;
}

void GisMapMonitorArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!m_pCenterPoint || !m_pPoint1 || !m_pPoint2)
    {
        return;
    }

    bool bRotation = false;
    QPointF centPoint = mapFromItem(m_pCenterPoint, 0, 0);
    QPoint firstPoint = mapFromItem(m_pPoint1, 0, 0).toPoint();
    QPoint secondPoint = mapFromItem(m_pPoint2, 0, 0).toPoint();
    if(centPoint != m_objCenterPoint || firstPoint != m_objFirstPoint || secondPoint != m_objSecondPoint)
    {
        bRotation = true;
    }

    m_objCenterPoint = mapFromItem(m_pCenterPoint, 0, 0);
    m_objFirstPoint = mapFromItem(m_pPoint1, 0, 0).toPoint();
    m_objSecondPoint = mapFromItem(m_pPoint2, 0, 0).toPoint();

    QLineF line1(m_objCenterPoint, m_objFirstPoint);
    QLineF line2(m_objCenterPoint, m_objSecondPoint);
    painter->setBrush( QBrush(m_objAreaColor, Qt::Dense7Pattern));
    painter->setPen(QPen(m_objAreaColor));

    QPointF leftTop = QPointF(m_objCenterPoint.x()-line1.length(),
                            m_objCenterPoint.y()-line1.length());
    int diameter = line1.length()*2; //饼图直径
    painter->setRenderHint(QPainter::Antialiasing, true); //抗锯齿
    painter->setRenderHint(QPainter::TextAntialiasing, true); //字体抗锯齿
    painter->drawPie(QRectF(leftTop.x(),
                            leftTop.y(),
                            diameter,
                            diameter),//饼图所在矩形区域
                     line1.angle()*16,//饼图的起始角度
                     line1.angleTo(line2)*16);//饼图结束角度和起始角度的差

    //旋转一下中心的摄像头方向。
    //setRotation时顺时针旋转的
    if(bRotation)
    {
        m_pCenterPoint->setRotation(-1*(line1.angle() + line1.angleTo(line2)/2));
    }
    this->setZValue(-1);
}

