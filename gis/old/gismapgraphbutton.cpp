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
#include "gismapgraphbutton.h"
#include "gismapgraphwidget.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

GisMapGraphButton::GisMapGraphButton(
        GisMapGraphWidget *graphWidget,
        QPointF position,
        int buttonId,
        QString buttonName)
    :m_pMap(graphWidget),
     m_bMousePressed(false),
     m_nButtonId(buttonId),
     m_objButtonName(buttonName)
{
    if(NULL != m_pMap)
    {
        m_pMap->scene()->addItem(this);
    }
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(4);
    this->setPos(position);
}

//返回graphButton的绘图区域
QRectF GisMapGraphButton::boundingRect() const
{
    return QRectF( -25, -30, 50, 60);
}

void GisMapGraphButton::paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *)
{
    QRadialGradient gradient(-3, -3, 10);
    //State_Sunken, if the button is down
    if (option->state & QStyle::State_Sunken)
    {
        gradient.setCenter(3, 3);
        gradient.setFocalPoint(3, 3);
        QColor color = QColor(Qt::yellow).light(120);
        color.setAlpha(100);
        gradient.setColorAt(1, color);
        color = QColor(Qt::darkYellow).light(120);
        color.setAlpha(100);
        gradient.setColorAt(0, color);
    }
    else
    {
        QColor color = QColor(Qt::yellow);
        color.setAlpha(100);
        gradient.setColorAt(0, color);

        color = QColor(Qt::darkYellow);
        color.setAlpha(100);
        gradient.setColorAt(1, color);
    }

    painter->setRenderHint(QPainter::Antialiasing, true); //抗锯齿
    painter->setRenderHint(QPainter::TextAntialiasing, true); //字体抗锯齿
    painter->setBrush(gradient);
    painter->setPen(QPen(Qt::black, 0));
    painter->drawEllipse(-5, -5, 10, 10);
}

QVariant GisMapGraphButton::itemChange(GraphicsItemChange change,const QVariant &value)
{
    switch (change)
    {
    case ItemPositionHasChanged:
        if(m_bMousePressed)
        {
            emit(signalPositionChanged(pos()));
        }
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void GisMapGraphButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_bMousePressed = true;
    update();
    if(event->button() == Qt::LeftButton)
    {
        //如果可以拖动时，signalRightButtonClicked和signalButtonPress都会触发，关联一个即可
        emit signalButtonPress(m_nButtonId);
    }
    QGraphicsItem::mousePressEvent(event);
}

void GisMapGraphButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_bMousePressed = false;
    if(event->button() == Qt::LeftButton)
    {
        emit signalRightButtonClicked(m_nButtonId);
    }
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}

void GisMapGraphButton::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "GisMapGraphButton::mouseDoubleClickEvent";
    emit(signalRightButtonDoubleClicked(m_nButtonId));
    //QGraphicsItem::mouseDoubleClickEvent(event);
}

void GisMapGraphButton::setDragAble(bool dragable)
{
    setFlag(ItemIsMovable, dragable);
}

void GisMapGraphButton::startAlarm()
{

}

void GisMapGraphButton::stopAlarm()
{
    //设备按钮子类去实现吧
}
