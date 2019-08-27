#include <QDebug>
#include <QTextDocument>
#include <QTextBlockFormat>
#include <QTextCursor>
#include "gismapitemipc.h"
#include "gismapgraphipcbutton.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

GisMapItemIpc::GisMapItemIpc(GisMapGraphWidget* map,
                       QPointF centerPosition,
                       QPoint firstPosition,
                       QPoint secondPosition,
                       int channelId,
                       QString buttonName)
      :QObject(map),
      m_pMap(map),
      m_objCenterOldPosition(centerPosition),
      m_objFirstOldPosition(firstPosition),
      m_objSecondOldPosition(secondPosition),
      m_nChannelId(channelId),
      m_nAlarmFlicker(0),
      m_FlickerDuration(0)
{
    m_pCenterPoint = new GisMapGraphIpcButton(map, centerPosition, channelId, buttonName);
    m_pFirstPoint = new GisMapGraphButton(map, firstPosition);
    m_pSecondPoint = new GisMapGraphButton(map, secondPosition);
    m_pMonitorArea = new GisMapMonitorArea(map, m_pCenterPoint, m_pFirstPoint, m_pSecondPoint);

    m_ipcName = new QGraphicsTextItem();
    m_pMap->scene()->addItem(m_ipcName);
    m_ipcName->setPos(centerPosition + QPointF(-32,25));
    //m_ipcName->setTextWidth(m_ipcName->boundingRect().width());
    m_ipcName->setPlainText(buttonName);
    QTextBlockFormat format;
    format.setAlignment(Qt::AlignCenter);
    QTextCursor cursor = m_ipcName->textCursor();
    cursor.select(QTextCursor::Document);
    cursor.mergeBlockFormat(format);
    cursor.clearSelection();
    m_ipcName->setTextCursor(cursor);

    connect(m_pCenterPoint,
            SIGNAL(signalRightButtonDoubleClicked(int)),
            this,
            SLOT(slotRightButtonDoubleClicked(int)));

    connect(m_pCenterPoint,
            SIGNAL(signalPositionChanged(QPointF)),
            this,
            SLOT(slotCenterPositionChanged(QPointF)));

    connect(m_pCenterPoint,
            SIGNAL(signalButtonPress(int)),
            this,
            SLOT(slotButtonPress(int)));

    connect(m_pFirstPoint,
            SIGNAL(signalPositionChanged(QPointF)),
            this,
            SLOT(slotFirstPointPositionChanged(QPointF)));

    connect(m_pSecondPoint,
            SIGNAL(signalPositionChanged(QPointF)),
            this,
            SLOT(slotSecondPointPositionChanged(QPointF)));
}

GisMapItemIpc::GisMapItemIpc(GisMapGraphWidget* map,
                       QPointF centerPosition,
                       int channelId,
                       QString buttonName)
    :QObject(map),
      m_pMap(map),
      m_objCenterOldPosition(centerPosition),
      m_nChannelId(channelId),
      m_nAlarmFlicker(0),
      m_FlickerDuration(0),
      m_pMonitorArea(NULL),
      m_pFirstPoint(NULL),
      m_pSecondPoint(NULL)
{
    m_pCenterPoint = new GisMapGraphIpcButton(map, centerPosition, channelId, buttonName);

    m_ipcName = new QGraphicsTextItem();
    m_pMap->scene()->addItem(m_ipcName);
    m_ipcName->setPos(centerPosition + QPointF(-32,25));
    m_ipcName->setPlainText(buttonName);
    QTextBlockFormat format;
    format.setAlignment(Qt::AlignCenter);
    QTextCursor cursor = m_ipcName->textCursor();
    cursor.select(QTextCursor::Document);
    cursor.mergeBlockFormat(format);
    cursor.clearSelection();
    m_ipcName->setTextCursor(cursor);

    connect(m_pCenterPoint,
            SIGNAL(signalRightButtonDoubleClicked(int)),
            this,
            SLOT(slotRightButtonDoubleClicked(int)));

    connect(m_pCenterPoint,
            SIGNAL(signalPositionChanged(QPointF)),
            this,
            SLOT(slotCenterPositionChanged(QPointF)));

    connect(m_pCenterPoint,
            SIGNAL(signalButtonPress(int)),
            this,
            SLOT(slotButtonPress(int)));
}

GisMapItemIpc::~GisMapItemIpc()
{

}

void GisMapItemIpc::releaseMapItemIpc()
{
    if(m_pMap == NULL)
    {
        return;
    }

    if(NULL != m_pCenterPoint)
    {
        m_pMap->scene()->removeItem(m_pCenterPoint);
        delete m_pCenterPoint;
        m_pCenterPoint = NULL;
    }

    if(NULL != m_pFirstPoint)
    {
        m_pMap->scene()->removeItem(m_pFirstPoint);
        delete m_pFirstPoint;
        m_pFirstPoint = NULL;
    }

    if(NULL != m_pSecondPoint)
    {
        m_pMap->scene()->removeItem(m_pSecondPoint);
        delete m_pSecondPoint;
        m_pSecondPoint = NULL;
    }

    if(NULL != m_pMonitorArea)
    {
        m_pMap->scene()->removeItem(m_pMonitorArea);
        delete m_pMonitorArea;
        m_pMonitorArea = NULL;
    }

    if(NULL != m_ipcName)
    {
        m_pMap->scene()->removeItem(m_ipcName);
        delete m_ipcName;
        m_ipcName = NULL;
    }
}

void GisMapItemIpc::slotCenterPositionChanged(QPointF position)
{
    //中心点移动时，饼图的开始点，结束点，饼图都要重新计算
    m_objFirstOldPosition -= m_objCenterOldPosition - position;
    if(m_pFirstPoint)
    {
        m_pFirstPoint->setPos(m_objFirstOldPosition);
    }
    m_objSecondOldPosition -= m_objCenterOldPosition - position;
    if(m_pSecondPoint)
    {
        m_pSecondPoint->setPos(m_objSecondOldPosition);
    }
    if(m_pMonitorArea)
    {
        m_pMonitorArea->adjust();
    }
    m_objCenterOldPosition = position;
    m_ipcName->setPos(position + QPointF(-32,25));

    emit(signalIpcInfoChanged(
            m_nChannelId,
            m_objCenterOldPosition,
            m_objFirstOldPosition.toPoint(),
            m_objSecondOldPosition.toPoint()));
}

void GisMapItemIpc::slotFirstPointPositionChanged(QPointF position)
{
    //起始点移动时，结束点的位置也要移动，夹角不变，但是与中心点的距离了要变

    //新的球半径
    QLineF firstSide(m_objCenterOldPosition, position);

    //计算另外一边的点变化后的位置
    QLineF secondSide;
    secondSide.setP1(m_objCenterOldPosition);
    secondSide.setP2(m_objSecondOldPosition);
    secondSide.setLength(firstSide.length());
    m_objSecondOldPosition = secondSide.p2();

    //设置另外一边的终点
    if(m_pSecondPoint)
    {
        m_pSecondPoint->setPos(m_objSecondOldPosition);
    }

    m_objFirstOldPosition = position;

    //从绘监控区域
    if(m_pMonitorArea)
    {
        m_pMonitorArea->adjust();
    }

    //旋转中心点的IPC图片
    if(m_pCenterPoint)
    {
        m_pCenterPoint->setRotation(firstSide.angle()+-1*(360-secondSide.angleTo(firstSide))/2);
    }

    emit(signalIpcInfoChanged(
            m_nChannelId,
            m_objCenterOldPosition,
            m_objFirstOldPosition.toPoint(),
            m_objSecondOldPosition.toPoint()));
}

void GisMapItemIpc::slotSecondPointPositionChanged(QPointF position)
{
    //结束点移动时，起始点的位置也要移动，夹角不变，但是与中心点的距离了要变

    //新半径
    QLineF secondSide(m_objCenterOldPosition, position);

    QLineF firstSide;
    firstSide.setP1(m_objCenterOldPosition);
    firstSide.setP2(m_objFirstOldPosition);
    firstSide.setLength(secondSide.length());

    m_objFirstOldPosition = firstSide.p2();
    if(m_pFirstPoint)
    {
        m_pFirstPoint->setPos(m_objFirstOldPosition);
    }

    m_objSecondOldPosition = position;
    if(m_pMonitorArea)
    {
        m_pMonitorArea->adjust();
    }

    emit(signalIpcInfoChanged(
            m_nChannelId,
            m_objCenterOldPosition,
            m_objFirstOldPosition.toPoint(),
            m_objSecondOldPosition.toPoint()));
}

void GisMapItemIpc::slotRightButtonDoubleClicked(int)
{
    emit(signalIpcDoubleClicked(m_nChannelId));
}

void GisMapItemIpc::slotButtonPress(int channelId)
{
    emit(signalIpcButtonPress(channelId));
}

void GisMapItemIpc::setDragAble(bool dragAble)
{
    if(dragAble)
    {
        if(m_pCenterPoint)
        {
            //根据是否可拖拽状态来改变鼠标形状
            m_pCenterPoint->setCursor(Qt::PointingHandCursor);
        }
    }
    else
    {
        if(m_pCenterPoint)
        {
            m_pCenterPoint->setCursor(Qt::ForbiddenCursor);
            m_pCenterPoint->setToolTip(tr("TK_CannotMoveIPC"));
        }
    }
    if(m_pCenterPoint)
    {
        m_pCenterPoint->setDragAble(dragAble);
    }
    if(m_pFirstPoint)
    {
        m_pFirstPoint->setDragAble(dragAble);
    }
    if(m_pSecondPoint)
    {
        m_pSecondPoint->setDragAble(dragAble);
    }
}

void GisMapItemIpc::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    if(m_pCenterPoint == NULL)
    {
        return;
    }
    if(m_FlickerDuration > 0)
    {
        if(m_pMonitorArea)
        {
            if(m_pMonitorArea->getAreaColor() == QColor("blue"))
            {
                m_pMonitorArea->setAreaColor(QColor("red"));
            }
            else
            {
                m_pMonitorArea->setAreaColor(QColor("blue"));
            }
            m_pMonitorArea->adjust();
        }
        m_pCenterPoint->startAlarm();
        m_FlickerDuration--;
    }
    else
    {
        killTimer(m_nAlarmFlicker);
        m_nAlarmFlicker = 0;
        m_pCenterPoint->stopAlarm();
    }
}

void GisMapItemIpc::startAlarmFlicker(int flickerDuration)
{
    if(m_nAlarmFlicker > 0)
    {
        killTimer(m_nAlarmFlicker);
    }
    m_nAlarmFlicker = startTimer(500);
    m_FlickerDuration = flickerDuration;
}

void GisMapItemIpc::stopAlarmFlicker()
{
    if(m_nAlarmFlicker > 0)
    {
        killTimer(m_nAlarmFlicker);
        m_nAlarmFlicker = 0;
    }

    if(m_pMonitorArea)
    {
        m_pMonitorArea->setAreaColor(QColor("blue"));
        m_pMonitorArea->adjust();
    }
}

void GisMapItemIpc::getCenterPoint(int &channelId,QPointF &point)
{
    //point = m_objCenterOldPosition; //场景变化不能侦测
    point = m_pCenterPoint->scenePos();
    channelId = m_nChannelId;
    if(point != m_objCenterOldPosition)
    {
        m_objCenterOldPosition = point;
    }
}

void GisMapItemIpc::moveCenterPoint(QPointF newPoint)
{
    if(m_pCenterPoint)
    {
        m_pCenterPoint->setPos(newPoint);
        m_ipcName->setPos(newPoint + QPointF(-32,25));
    }
}

void GisMapItemIpc::showDeviceObj(ShowParam param)
{
    if(param == ShowDevAndArea || param == ShowDevOnly)
    {
        m_pCenterPoint->show();
        m_ipcName->show();
    }
    else
    {
        m_pCenterPoint->hide();
        m_ipcName->hide();
    }
    if(param == ShowDevAndArea)
    {
        if(m_pFirstPoint)
        {
            m_pFirstPoint->show();
        }
        if(m_pSecondPoint)
        {
            m_pSecondPoint->show();
        }
        if(m_pMonitorArea)
        {
            m_pMonitorArea->show();
        }
    }
    else
    {
        if(m_pFirstPoint)
        {
            m_pFirstPoint->hide();
        }
        if(m_pSecondPoint)
        {
            m_pSecondPoint->hide();
        }
        if(m_pMonitorArea)
        {
            m_pMonitorArea->hide();
        }
    }
}
