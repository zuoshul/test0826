#include <math.h>
#include <QPicture>
#include <QKeyEvent>
#include <QDebug>
#include <QFileInfo>
#include <QMenu>
#include <QMimeData>
#include <QSvgWidget>
#include <QLabel>
#include <QGraphicsProxyWidget>
#include <QQueue>
#include <QThread>
#include "gismapitemipc.h"
#include "gismapgraphwidget.h"
#include "src/gis/old/gismapgraphicaddmapbutton.h"
#include "src/sppclient/sppclient.h"
#include "src/gis/tileSources/OSMTileSource.h"
#include "src/gis/tileSources/CompositeTileSource.h"
#include "src/gis/old/gisareaselectobj.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

#define MIN_GRAPH_WIDGET_WIDTH 400
#define MIN_GRAPH_WIDGET_HEIGHT 400
GisMapGraphWidget::GisMapGraphWidget(QWidget *parent)
    : QGraphicsView(parent),
	m_pButtonSelect(NULL),
    m_pBubbleWidget(NULL),
    m_pBubbleLabel(NULL),
    m_pBubbleWidgetItem(NULL),
    m_pDelDevDlg(NULL),
    m_pDelMapDlg(NULL),
    m_scalePercent(100),
    m_pTileSource(NULL),
    m_curMapMode(LocalMap),
    m_objAreaSelect(NULL),
    m_bDrawing(false),
    m_objMapPath(QString()),
    m_bNoticeCenterChange(false)
{
    initActions();

    setCacheMode(CacheBackground);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setDragModeGisMap(GisDragMode::ScrollHandDrag);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMinimumSize(MIN_GRAPH_WIDGET_WIDTH, MIN_GRAPH_WIDGET_HEIGHT);

    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    qDebug() << "GisMapGraphWidget() width " << this->width() << "height " << this->height();
    setScene(scene);

    m_pDelDevDlg = new SNMessageDialog(this);
    m_pDelMapDlg = new SNMessageDialog(this);
    connect(m_pDelDevDlg,SIGNAL(signalConfirmed()),this,SLOT(slotConfirmDelDev()));
    connect(m_pDelMapDlg,SIGNAL(signalConfirmed()),this,SLOT(slotConfirmDelMap()));

    initGis();
}


GisMapGraphWidget::~GisMapGraphWidget()
{
    qDebug() << "GisMapGraphWidget::~GisMapGraphWidget()"<<endl;
    if (!m_pTileSource.isNull())
    {
        //Find the tileSource's thread
        QPointer<QThread> tileSourceThread = m_pTileSource->thread();

        //Clear the QSharedPointer to the tilesource. Unless there's a serious problem, we should be the
        //last thing holding that reference and we expect it to be deleted
        m_pTileSource.clear();

        //After the tilesource is deleted, we wait for the thread it was running in to shut down
        int count = 0;
        const int maxCount = 100;
        while (!tileSourceThread.isNull() && !tileSourceThread->wait(100))
        {
            //We have to process events while it's shutting down in case it uses signals/slots to shut down
            QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers | QEventLoop::ExcludeUserInputEvents);
            if (++count == maxCount)
            {
                break;
            }
        }
    }
}

void GisMapGraphWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    if(!isMapExist())
    {
        if(m_curMapMode == LocalMap)
        {
            //没有地图可用？禁止添加设备和拖拽事件
            changeDragableState(false);
        }
        else
        {
            //Layout the tile objects
            this->doTileLayout();
        }
    }
    else
    {
        //如果当前是编辑状态,并且禁止的拖拽事件? 使能拖拽事件
        if(m_nCurMapState == MapStateEdit && !this->acceptDrops())
        {
            changeDragableState(false);
        }

        QPixmap pixmap(m_objMapPath);
        QRectF source(0.0, 0.0, pixmap.size().width(), pixmap.size().height());

        qDebug() <<"GisMapGraphWidget::drawBackground mapPath " <<m_objMapPath << " size " << pixmap.size();
        painter->drawPixmap(this->sceneRect(), pixmap, source);
    }
}

void GisMapGraphWidget::dragMoveEvent(QDragMoveEvent *event)
{
    Q_UNUSED(event);
    qDebug() << "GisMapGraphWidget::dragMoveEvent()";
}

void GisMapGraphWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *myData = qobject_cast<const QMimeData *>(event->mimeData());
    if(NULL == myData)
    {
        qDebug() << "GisMapGraphWidget::dropEvent mimedata is null";
        return;
    }

    const QMimeData * curMineData = event->mimeData();
    if(NULL == curMineData)
    {
        qDebug() << "GisMapGraphWidget::dropEvent curMineData is null";
        return;
    }

    QVariant variant = event->mimeData()->colorData();
    QList<QVariant> dataList = variant.toList();
    if(1 != dataList.size())
    {
        return;
    }

    QPointF dropPoint = mapToScene(event->pos());
    Node node = dataList.at(0).value<Node>();

    qDebug() << "GisMapGraphWidget::dropEvent() add new dev " << node.strNodeName
             << "id " << node.id
             << "pos " << dropPoint;

    QPointF gisPoint;
    if(m_curMapMode == GisMap && m_pTileSource)
    {
        gisPoint = m_pTileSource->qgs2ll(dropPoint,this->zoomLevel());
    }

    emit signalAddDev(node, m_curMapMode == GisMap ? gisPoint : dropPoint);

    event->acceptProposedAction();
}

bool GisMapGraphWidget::getDevPosition(int channelId,QPointF &devPoint)
{
    QMap<int, GisMapItemIpc*>::iterator ipcIt = m_objIpcButtons.begin();
    while(ipcIt != m_objIpcButtons.end())
    {
        int tmpChannelId;
        QPointF point;
        ipcIt.value()->getCenterPoint(tmpChannelId, point);
        if(channelId == tmpChannelId)
        {
            devPoint = mapFromScene(point);
            return true;
        }
        ipcIt++;
    }
    return false;
}

void GisMapGraphWidget::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
    {
        return;
    }

    if((m_scalePercent < 50 && scaleFactor < 1) || (m_scalePercent > 250 && scaleFactor > 1))
    {
        return;
    }
    scale(scaleFactor, scaleFactor);
    m_scalePercent = m_scalePercent * scaleFactor;
    emit scaled(m_scalePercent);
}

void GisMapGraphWidget::slotZoomIn()
{
    if(m_curMapMode == GisMap)
    {
        zoomInGisMap(CenterZoom);
    }
    else
    {
        scaleView(qreal(1.2));
    }
}

void GisMapGraphWidget::slotZoomOut()
{
    if(m_curMapMode == GisMap)
    {
        zoomOutGisMap(CenterZoom);
    }
    else
    {
        scaleView(1 / qreal(1.2));
    }
}

void GisMapGraphWidget::reset()
{
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    QMatrix q;
    q.setMatrix(1,matrix().m12(),matrix().m21(),1,matrix().dx(),matrix().dy());
    setMatrix(q,false);
}

void GisMapGraphWidget::clearMapInfo()
{
    //手动一个一个释放子地图按钮
    QMap<int, GisMapGraphicSubMapButton*>::iterator mapButtonit = m_objSubMapButtons.begin();
    while(!m_objSubMapButtons.isEmpty())
    {
        mapButtonit = m_objSubMapButtons.begin();
        GisMapGraphicSubMapButton* subMapButton = mapButtonit.value();
        scene()->removeItem(subMapButton);
        delete subMapButton;
        m_objSubMapButtons.erase(mapButtonit);
    }

    //手动一个一个释放IPC按钮
    QMap<int, GisMapItemIpc*>::iterator ipcButtonIt = m_objIpcButtons.begin();
    while(!m_objIpcButtons.empty())
    {
        ipcButtonIt = m_objIpcButtons.begin();
        GisMapItemIpc* ipcButton = ipcButtonIt.value();
        //释放IPC中的4个GraphicItem
        ipcButton->releaseMapItemIpc();
        delete ipcButton;
        m_objIpcButtons.erase(ipcButtonIt);
    }

    if(NULL != m_pBubbleWidgetItem)
    {
        scene()->removeItem(m_pBubbleWidgetItem);
        delete m_pBubbleWidgetItem;
        m_pBubbleWidgetItem = NULL;

        //m_pBubbleLabel和m_pBubbleWidget会和m_pBubbleWidgetItem一起释放。
        m_pBubbleLabel = NULL;
        m_pBubbleWidget = NULL;
        curFlagImage.clear();
    }

    foreach(MapTileGraphicsObject * tileObject, m_tileObjects)
    {
        //释放瓦片
        scene()->removeItem(tileObject);
        delete tileObject;
        tileObject = NULL;
    }
    m_tileObjects.clear();
}

void GisMapGraphWidget::setMapInfo(const QString& mapPath,QList<GisMapButton>& subMapButtons,QList<GisMapButton>& ipcButtons)
{
    //删除地图上原有的元素
    clearMapInfo();

    m_objMapPath = mapPath;
    m_curMapMode = LocalMap;

    QPixmap pixmap(m_objMapPath);
    QRectF sceneRectSize(
        pixmap.size().width()/2*-1,
        pixmap.size().height()/2*-1,
        pixmap.size().width(),
        pixmap.size().height());
    this->setSceneRect(sceneRectSize);

    fitInView(sceneRect(), Qt::KeepAspectRatio);

    //painter坐标系也是笛卡尔的。
    qDebug() <<"GisMapGraphWidget::setMapInfo() mapPath "<< m_objMapPath << "mapSize " << pixmap.size();

    addSubMapButtonsToMap(subMapButtons);

    addIpcButtonsToMap(ipcButtons);

    //刷新背景图
    resetCachedContent();

    update();
}

void GisMapGraphWidget::setGisMapInfo(QList<GisMapButton>& subMapButtons,QList<GisMapButton>& ipcButtons)
{
    //删除地图上原有的元素
    clearMapInfo();

    m_objMapPath = QString();
    m_curMapMode = GisMap;

    addSubMapButtonsToMap(subMapButtons);

    addIpcButtonsToMap(ipcButtons);

    this->resetQGSSceneSize();

    setZoomLevel(m_lastViewParam.zoomLevel);
    centerOnGisMap(m_lastViewParam.centerPoint.x(), m_lastViewParam.centerPoint.y());

    //gis暂时不允许缩放
    reset();

    //刷新背景图
    resetCachedContent();

    update();
}

void GisMapGraphWidget::addSubMapButtons(QList<GisMapButton>& buttons)
{
    //新增子地图
    addSubMapButtonsToMap(buttons);
    update();
}


void GisMapGraphWidget::addIpcButtons(QList<GisMapButton>& buttons)
{
    //新增摄像机
    addIpcButtonsToMap(buttons);
    update();
}

void GisMapGraphWidget::deleteSubMapButtons(QList<int>& buttons)
{
    //在当前地图上删除子地图按钮
    QMap<int, GisMapGraphicSubMapButton*>::iterator it = m_objSubMapButtons.begin();
    for(int i=0; i<buttons.size(); i++)
    {
        int id = buttons.at(i);
        if((it = m_objSubMapButtons.find(id)) != m_objSubMapButtons.end())
        {
            GisMapGraphicSubMapButton* subMapButton = it.value();
            //从sence中移除后，需要手动delete
            scene()->removeItem(subMapButton);
            delete subMapButton;
            m_objSubMapButtons.erase(it);
        }
    }
    update();
}

void GisMapGraphWidget::deleteIpcButtons(QList<int>& buttons)
{
    //在当前地图上删除IPC按钮
    QMap<int, GisMapItemIpc*>::iterator it = m_objIpcButtons.begin();
    for(int i=0; i<buttons.size(); i++)
    {
        int id = buttons.at(i);
        if((it = m_objIpcButtons.find(id)) != m_objIpcButtons.end())
        {
            GisMapItemIpc* ipcButton = it.value();
            //释放IPC中的4个GraphicItem
            ipcButton->releaseMapItemIpc();
            delete ipcButton;
            m_objIpcButtons.erase(it);
        }
    }
    update();
}

void GisMapGraphWidget::addSubMapButtonsToMap(QList<GisMapButton>& buttons)
{
    //添加到地图上
    for(int i=0; i<buttons.size(); i++)
    {
        const GisMapButton& button = buttons.at(i);

        //gis坐标转换
        QPointF gisPoint;
        if(m_curMapMode == GisMap && m_pTileSource)
        {
            QPointF mapPoint;
            if(button.centerPoint == QPointF(0,0))
            {
                mapPoint = QPointF(PointWuHan.x(),PointWuHan.y());
            }
            else
            {
                mapPoint = button.centerPoint;
            }
            gisPoint = m_pTileSource->ll2qgs(mapPoint,this->zoomLevel());
        }
        //new 出来的Item被添加到sence中，会有sence自动释放
        GisMapGraphicSubMapButton* buttonItem  =  new GisMapGraphicSubMapButton(
                    this,
                    m_curMapMode == GisMap ? gisPoint : button.centerPoint,
                    button.id,
                    button.buttonName);
		
        connect(buttonItem,
                SIGNAL(signalRightButtonDoubleClicked(int )),
                this,
                SIGNAL(signalSubMapButtonClicked(int)));

        connect(buttonItem,
                SIGNAL(signalSubMapInfoChanged(int , QPointF)),
                this,
                SLOT(slotMapInfoChanged(int , QPointF)));

        m_objSubMapButtons.insert(button.id, buttonItem);
    }
}

void GisMapGraphWidget::addIpcButtonsToMap(QList<GisMapButton>& buttons, bool showConnection)
{
    Q_UNUSED(showConnection);

    QList<QPointF> ipcs;
    for(int i = 0; i < buttons.size(); i++)
    {
        const GisMapButton& button = buttons.at(i);
        GisMapItemIpc* buttonItem = m_objIpcButtons.value(button.id);
        if(buttonItem != NULL)
        {
            //地图上已经包含这个按钮，删除原来的按钮
            buttonItem->disconnect();
            buttonItem->releaseMapItemIpc();
            delete buttonItem;
            m_objIpcButtons.remove(button.id);
        }

        //gis坐标转换
        if(m_curMapMode == GisMap && m_pTileSource)
        {
            QPointF qgsPos = m_pTileSource->ll2qgs(button.centerPoint,this->zoomLevel());
            buttonItem = new GisMapItemIpc(this,
                           qgsPos,
                           button.id,
                           button.buttonName);
        }
        else
        {
            //new 出来的Item被添加到sence中，会由sence自动释放
            buttonItem = new GisMapItemIpc(this,
                           button.centerPoint,
                           button.firstPoint,
                           button.secondPoint,
                           button.id,
                           button.buttonName);
        }
        connect(buttonItem,SIGNAL(signalIpcInfoChanged(int, QPointF, QPoint, QPoint)),this, SLOT(slotIpcInfoChanged(int, QPointF, QPoint, QPoint)));
		connect(buttonItem, SIGNAL(signalIpcDoubleClicked(int)), this, SIGNAL(signalIpcButtonClicked(int)));
        connect(buttonItem, SIGNAL(signalIpcButtonPress(int)), this, SIGNAL(signalIpcButtonPress(int)));

        m_objIpcButtons.insert(button.id, buttonItem);

        ipcs.push_back(button.centerPoint);
    }
    slotResourceFilter(m_filter);
}

#ifndef QT_NO_CONTEXTMENU
void GisMapGraphWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QGraphicsItem * item = itemAt(event->pos());
    m_objCurMousePos = mapToScene(event->pos());
    if(item != NULL)
    {
        qDebug() << "GisMapGraphWidget::contextMenuEvent " << item->type() << " pos in sence Cartesian " << item->pos();
		m_pButtonSelect = NULL;
        switch (item->type())
        {
        case CustomGraphItemIpcButton:
            m_pButtonSelect = qgraphicsitem_cast<GisMapGraphIpcButton*>(item);
            qDebug() << "GisMapGraphWidget::contextMenuEvent CustomGraphItemIpcButton " << (m_pButtonSelect == NULL);
            menu.addAction(m_objDeleteDev);
            break;

        case CustomGraphItemSubMapButton:
            m_pButtonSelect = qgraphicsitem_cast<GisMapGraphicSubMapButton*>(item);
            qDebug() << "GisMapGraphWidget::contextMenuEvent CustomGraphItemSubMapButton " << (m_pButtonSelect == NULL);
            menu.addAction(m_objDeleteMap);
            break;

        case CustomGraphItemMonitorArea:
        case CustomGraphItemDefaultButton:
        default:
            menu.addAction(m_objAddMap);
            break;
        }
    }
    else
    {
        qDebug() << "GisMapGraphWidget::contextMenuEvent no content menu ";
        menu.addAction(m_objAddMap);
    }
    if (isMapExist() || m_curMapMode == GisMap)
    {
        //只有当前有地图选中时，才弹出右键菜单
		menu.exec(event->globalPos());
	}
    qDebug() << event->pos() << "  " << event->globalPos();
}
#endif

void GisMapGraphWidget::initActions()
{
    m_objAddMap = new QAction(QObject::tr("TK_AddMap"), this);
    connect(m_objAddMap, SIGNAL(triggered()), this, SLOT(slotAddMap()));

    m_objDeleteMap = new QAction(QObject::tr("TK_DeleteMap"), this);
    connect(m_objDeleteMap, SIGNAL(triggered()), this, SLOT(slotDelteMap()));

    m_objDeleteDev = new QAction(QObject::tr("TK_DeleteDev"), this);
    connect(m_objDeleteDev, SIGNAL(triggered()), this, SLOT(slotDelteDev()));
}

void GisMapGraphWidget::slotAddMap()
{
    QPointF point;
    if(m_curMapMode == GisMap)
    {
        QPointF qgsPos = m_pTileSource->qgs2ll(m_objCurMousePos,this->zoomLevel());
        point = qgsPos;
    }
    else
    {
        point = m_objCurMousePos;
    }
    emit(signalAddMap(point));
}

void GisMapGraphWidget::slotDelteMap()
{
    m_pDelMapDlg->showDialog(QObject::tr("TK_Map"), QObject::tr("TK_MapDeleteConfim"), SNMessageDialog::MessageTypeRequest);
}

void GisMapGraphWidget::slotDelteDev()
{
    m_pDelDevDlg->showDialog(QObject::tr("TK_Map"), QObject::tr("TK_DevDeleteConfim"), SNMessageDialog::MessageTypeRequest);
}

void GisMapGraphWidget::slotConfirmDelDev()
{
    if (m_pButtonSelect != NULL)
    {
        emit(signalDelDevice(m_pButtonSelect->getButtonId()));
    }
}

void GisMapGraphWidget::slotConfirmDelMap()
{
    if (m_pButtonSelect != NULL)
    {
        emit(signalDelMap(m_pButtonSelect->getButtonId()));
    }
}

bool GisMapGraphWidget::isMapExist()
{
	QFileInfo file(m_objMapPath);
	if (!file.exists())
	{
		return false;
	}
	else
	{
		return true;
	}
}

void GisMapGraphWidget::changeDragableState(bool dragAble)
{
    QMap<int, GisMapGraphicSubMapButton*>::iterator mapIt = m_objSubMapButtons.begin();
    while(mapIt != m_objSubMapButtons.end())
    {
        mapIt.value()->setDragAble(dragAble);
        ++mapIt;
    }

    QMap<int, GisMapItemIpc*>::iterator ipcIt = m_objIpcButtons.begin();
    while(ipcIt != m_objIpcButtons.end())
    {
        ipcIt.value()->setDragAble(dragAble);
        ++ipcIt;
    }

    this->setAcceptDrops(dragAble);
}

void GisMapGraphWidget::switchMapState(MapState mapState)
{
    if(mapState == MapStateEdit)
    {
        //开启拖拽事件
        changeDragableState(true);
        //开启右键弹框
        setContextMenuPolicy(Qt::DefaultContextMenu);
    }
    else
    {
        //禁止拖拽事件
        changeDragableState(false);
        //禁止右键弹框
        setContextMenuPolicy(Qt::NoContextMenu);
    }
    m_nCurMapState = mapState;
}

void GisMapGraphWidget::startAlarm(int id)
{
    QMap<int, GisMapItemIpc*>::iterator ipcIt = m_objIpcButtons.find(id);
    if(ipcIt != m_objIpcButtons.end())
    {
        ipcIt.value()->startAlarmFlicker();
    }
    else
    {
        qDebug() << "GisMapGraphWidget::startAlarm " << id << " not found!";
    }
}

void GisMapGraphWidget::stopAlarm(int id)
{
    QMap<int, GisMapItemIpc*>::iterator ipcIt = m_objIpcButtons.find(id);
    if(ipcIt != m_objIpcButtons.end())
    {
        ipcIt.value()->stopAlarmFlicker();
    }
    else
    {
        qDebug() << "GisMapGraphWidget::stopAlarm " << id << " not found!";
    }
}

void GisMapGraphWidget::addTrack(QList<GisMapButton>& ipcs)
{
    addIpcButtonsToMap(ipcs, true);
    update();
}

void GisMapGraphWidget::moveFlag(const QString& flagImage, const QPoint& postion)
{
    if(NULL == m_pBubbleWidgetItem)
    {
        m_pBubbleWidget = new QSvgWidget(":images/default/map/bubble.svg");
        m_pBubbleWidget->setStyleSheet("background-color: transparent;");

        m_pBubbleLabel = new QLabel(m_pBubbleWidget);
        m_pBubbleLabel->setGeometry(10,7,50,50);

        m_pBubbleWidgetItem = scene()->addWidget(m_pBubbleWidget);
        m_pBubbleWidgetItem->setZValue(900);
    }

    if(curFlagImage.compare(flagImage) != 0)
    {
        QString styleSheetFormat("border-image: url(%1);border:0px groove gray;border-radius:10px;padding:2px 4px;");
        QString styleSheet = styleSheetFormat.arg(flagImage);

        m_pBubbleLabel->setStyleSheet(styleSheet);
        curFlagImage = flagImage;
    }

    m_pBubbleWidget->setGeometry(postion.x()-35, postion.y()-70, 70, 70);
}

void GisMapGraphWidget::doTileLayout()
{
    if (m_pTileSource.isNull())
    {
        qDebug() << "GisMapGraphWidget::doTileLayout() No MapTileSource to render";
        return;
    }

    //Calculate the center point and polygon of the viewport in QGraphicsScene coordinates
    const QPointF centerPointQGS = this->mapToScene(this->width()/2.0,this->height()/2.0);
    QPolygon viewportPolygonQGV;
    viewportPolygonQGV << QPoint(0,0) << QPoint(0,this->height()) << QPoint(this->width(),this->height()) << QPoint(this->width(),0);

    const QPolygonF viewportPolygonQGS = this->mapToScene(viewportPolygonQGV);
    const QRectF boundingRect = viewportPolygonQGS.boundingRect();

    //We exaggerate the bounding rect for some purposes!
    QRectF exaggeratedBoundingRect = boundingRect;
    exaggeratedBoundingRect.setSize(boundingRect.size()*2.0);
    exaggeratedBoundingRect.moveCenter(boundingRect.center());

    //We'll mark tiles that aren't being displayed as free so we can use them
    QQueue<MapTileGraphicsObject *> freeTiles;

    QSet<QPointF> placesWhereTilesAre;
    foreach(MapTileGraphicsObject * tileObject, m_tileObjects)
    {
        if (!tileObject->isVisible() || !exaggeratedBoundingRect.contains(tileObject->pos()))
        {
            freeTiles.enqueue(tileObject);
            tileObject->setVisible(false);
        }
        else
        {
            placesWhereTilesAre.insert(tileObject->pos());
        }
    }

    const quint16 tileSize = m_pTileSource->tileSize();
    const quint32 tilesPerRow = sqrt((long double)m_pTileSource->tilesOnZoomLevel(this->zoomLevel()));
    const quint32 tilesPerCol = tilesPerRow;

    const qint32 perSide = qMax(boundingRect.width()/tileSize,boundingRect.height()/tileSize) + 3;

    const qint32 xc = qMax((qint32)0,(qint32)(centerPointQGS.x() / tileSize) - perSide/2);

    const qint32 yc = qMax((qint32)0,(qint32)(centerPointQGS.y() / tileSize) - perSide/2);

    const qint32 xMax = qMin((qint32)tilesPerRow,xc + perSide);

    const qint32 yMax = qMin(yc + perSide,(qint32)tilesPerCol);

    for (qint32 x = xc; x < xMax; x++)
    {
        for (qint32 y = yc; y < yMax; y++)
        {
            const QPointF scenePos(x*tileSize + tileSize/2,y*tileSize + tileSize/2);

            bool tileIsThere = false;
            if (placesWhereTilesAre.contains(scenePos))
            {
                tileIsThere = true;
            }

            if (tileIsThere)
            {
                continue;
            }

            //Just in case we're running low on free tiles, add one
            if (freeTiles.isEmpty())
            {
                MapTileGraphicsObject * tileObject = new MapTileGraphicsObject(tileSize);
                tileObject->setTileSource(m_pTileSource);
                m_tileObjects.insert(tileObject);
                scene()->addItem(tileObject);
                freeTiles.enqueue(tileObject);
                connect(tileObject, SIGNAL(zoomIn()), this, SLOT(slotZoomInGisMap()));
            }
            //Get the first free tile and make it do its thing
            MapTileGraphicsObject * tileObject = freeTiles.dequeue();
            if (tileObject->pos() != scenePos)
            {
                tileObject->setPos(scenePos);
            }
            if (tileObject->isVisible() != true)
            {
                tileObject->setVisible(true);
            }
            tileObject->setTile(x,y,this->zoomLevel());
        }
    }

    //If we've got a lot of free tiles left over, delete some of them
    while (freeTiles.size() > 2)
    {
        MapTileGraphicsObject * tileObject = freeTiles.dequeue();
        m_tileObjects.remove(tileObject);
        scene()->removeItem(tileObject);
        delete tileObject;
    }
}

quint8 GisMapGraphWidget::zoomLevel()
{
    return m_zoomLevel;
}

void GisMapGraphWidget::setZoomLevel(quint8 nZoom, ZoomMode zMode)
{
    if (m_pTileSource.isNull())
    {
        qDebug()<<"GisMapGraphWidget::setZoomLevel m_pTileSource null"<<endl;
        return;
    }

    //This stuff is for handling the re-centering upong zoom in/out
    const QPointF  centerGeoPos = this->mapToSceneGisMap(QPoint(this->width()/2,this->height()/2));
    QPointF mousePoint = this->mapToScene(this->mapFromGlobal(QCursor::pos()));
    QRectF sceneRect = scene()->sceneRect();
    const float xRatio = mousePoint.x() / sceneRect.width();
    const float yRatio = mousePoint.y() / sceneRect.height();
    const QPointF centerPos = this->mapToScene(QPoint(this->width()/2,this->height()/2));
    const QPointF offset = mousePoint - centerPos;

    //Change the zoom level
    nZoom = qMin(m_pTileSource->maxZoomLevel(),qMax(m_pTileSource->minZoomLevel(),nZoom));

    if (nZoom == m_zoomLevel)
    {
        return;
    }

    m_zoomLevel = nZoom;

    //Disable all tile display temporarily. They'll redisplay properly when the timer ticks
    foreach(MapTileGraphicsObject * tileObject, m_tileObjects)
    {
        tileObject->setVisible(false);
    }

    //Make sure the QGraphicsScene is the right size
    this->resetQGSSceneSize();

    //Re-center the view where we want it
    sceneRect = scene()->sceneRect();
    mousePoint = QPointF(sceneRect.width()*xRatio,sceneRect.height()*yRatio) - offset;

    if (zMode == MouseZoom)
    {
        this->centerOn(mousePoint);
    }
    else
    {
        centerOnGisMap(centerGeoPos);
    }

    //Make MapGraphicsObjects update
    this->zoomLevelChanged(nZoom);
}

void GisMapGraphWidget::zoomInGisMap(ZoomMode zMode)
{
    if (m_pTileSource.isNull())
    {
        return;
    }

    if (this->zoomLevel() < m_pTileSource->maxZoomLevel())
    {
        this->setZoomLevel(this->zoomLevel() + 1,zMode);
    }
}

void GisMapGraphWidget::zoomOutGisMap(ZoomMode zMode)
{
    if (m_pTileSource.isNull())
    {
        return;
    }

    if (this->zoomLevel() > m_pTileSource->minZoomLevel())
    {
        this->setZoomLevel(this->zoomLevel() - 1,zMode);
    }
}

void GisMapGraphWidget::resetQGSSceneSize()
{
    if (m_pTileSource.isNull())
    {
        return;
    }

    const quint64 dimension = sqrt((long double)m_pTileSource->tilesOnZoomLevel(this->zoomLevel()))*m_pTileSource->tileSize();
    if (scene()->sceneRect().width() != dimension)
    {
        scene()->setSceneRect(0,0,dimension,dimension);
    }
    setSceneRect(0,0,dimension,dimension);
}

QSharedPointer<MapTileSource> GisMapGraphWidget::tileSource() const
{
    return m_pTileSource;
}

void GisMapGraphWidget::setTileSource(QSharedPointer<MapTileSource> tSource)
{
    m_pTileSource = tSource;

    if (!m_pTileSource.isNull())
    {
        //Create a thread just for the tile source
        QThread * tileSourceThread = new QThread();
        tileSourceThread->start();
        m_pTileSource->moveToThread(tileSourceThread);

        //Set it up so that the thread is destroyed when the tile source is!
        connect(m_pTileSource.data(),
                SIGNAL(destroyed()),
                tileSourceThread,
                SLOT(quit()));

        connect(tileSourceThread,
                SIGNAL(finished()),
                tileSourceThread,
                SLOT(deleteLater()));
    }

    //Update our tile displays (if any) about the new tile source
    foreach(MapTileGraphicsObject * tileObject, m_tileObjects)
    {
        tileObject->setTileSource(tSource);
    }
}

void GisMapGraphWidget::centerOnGisMap(qreal longitude, qreal latitude)
{
    centerOnGisMap(QPointF(longitude,latitude));
}

void GisMapGraphWidget::centerOnGisMap(const QPointF &pos)
{
    if (m_pTileSource.isNull())
    {
        return;
    }
    QPointF qgsPos = m_pTileSource->ll2qgs(pos,this->zoomLevel());
    this->centerOn(qgsPos);
}

void GisMapGraphWidget::centerOnGisMap(const MapGraphicsObject *item)
{
    if(item)
    {
        centerOnGisMap(item->pos());
    }
}

QPointF GisMapGraphWidget::mapToSceneGisMap(const QPoint viewPos)
{
    if (m_pTileSource.isNull())
    {
        return QPointF(0,0);
    }

    QPointF qgsScenePos = this->mapToScene(viewPos);

    //Convert from QGraphicsScene coordinates to geo (MapGraphicsScene) coordinates
    quint8 zoom = this->zoomLevel();

    return m_pTileSource->qgs2ll(qgsScenePos,zoom);
}

void GisMapGraphWidget::initGis()
{
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    this->setDragModeGisMap(GisDragMode::ScrollHandDrag);
    this->resetQGSSceneSize();

    //Setup some tile sources
    QSharedPointer<OSMTileSource> osmTiles(new OSMTileSource(OSMTileSource::OSMTiles), &QObject::deleteLater);
    QSharedPointer<CompositeTileSource> composite(new CompositeTileSource(), &QObject::deleteLater);
    composite->addSourceBottom(osmTiles);
    setTileSource(composite);
    setZoomLevel(4);
    centerOnGisMap(PointWuHan.x(), PointWuHan.y());
}

GisDragMode GisMapGraphWidget::dragMode()
{
    return m_dragMode;
}

void GisMapGraphWidget::setDragModeGisMap(GisDragMode mode)
{
    m_dragMode = mode;

    QGraphicsView::DragMode qgvDragMode;
    if (m_dragMode == NoDrag)
    {
        qgvDragMode = QGraphicsView::NoDrag;
    }
    else if (m_dragMode == ScrollHandDrag)
    {
        qgvDragMode = QGraphicsView::ScrollHandDrag;
    }
    else
    {
        qgvDragMode = QGraphicsView::RubberBandDrag;
    }

    this->setDragMode(qgvDragMode);
}

void GisMapGraphWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QGraphicsView::mouseDoubleClickEvent(event);
}

void GisMapGraphWidget::slotZoomInGisMap()
{
    this->zoomGisMapAndProOther(true);
}

void GisMapGraphWidget::slotMapInfoChanged(int channelId,QPointF qgs)
{
    if(m_curMapMode == GisMap)
    {
        QPointF ll;
        if(m_pTileSource)
        {
            ll = m_pTileSource->qgs2ll(qgs,this->zoomLevel());
        }
        emit signalMapInfoChanged(channelId,ll);
    }
    else
    {
        emit signalMapInfoChanged(channelId,qgs);
    }
}

void GisMapGraphWidget::slotIpcInfoChanged(int channelId,QPointF center,QPoint first, QPoint second)
{
    if(m_curMapMode == GisMap)
    {
        QPointF ll;
        if(m_pTileSource)
        {
            ll = m_pTileSource->qgs2ll(center,this->zoomLevel());
        }
        emit signalIpcInfoChanged(channelId,ll,first,second);
    }
    else
    {
        emit signalIpcInfoChanged(channelId,center,first,second);
    }
}

void GisMapGraphWidget::getAllGisItemll(QMap<int, QPointF> &devOldPositionMap,QMap<int, QPointF> &subMapOldPositionMap)
{
    devOldPositionMap.clear();
    subMapOldPositionMap.clear();

    QMap<int, GisMapItemIpc*>::iterator itor;
    for(itor = m_objIpcButtons.begin();itor != m_objIpcButtons.end();itor++)
    {
        int channelId;
        QPointF point;
        itor.value()->getCenterPoint(channelId,point);
        QPointF ll = m_pTileSource->qgs2ll(point,this->zoomLevel());
        devOldPositionMap[itor.key()] = ll;
        qCritical() <<"GisMapGraphWidget::getAllGisItemll channelId "<<channelId<<"oldPoint "<<point<<"ll "<<ll<<endl;
    }

    QMap<int, GisMapGraphicSubMapButton*>::iterator itorSubMap;
    for(itorSubMap = m_objSubMapButtons.begin();itorSubMap != m_objSubMapButtons.end();itorSubMap++)
    {
        QPointF point = itorSubMap.value()->getPosition();
        QPointF ll = m_pTileSource->qgs2ll(point,this->zoomLevel());
        subMapOldPositionMap[itorSubMap.key()] = ll;
        qCritical() <<"GisMapGraphWidget::getAllGisItemll "<<"oldPoint "<<point<<"ll "<<ll<<endl;
    }
}

void GisMapGraphWidget::moveAllGisItem(QMap<int, QPointF> devOldPositionMap,QMap<int, QPointF> subMapOldPositionMap,int zoomLevel)
{
    QMap<int, GisMapItemIpc*>::iterator itor;
    for(itor = m_objIpcButtons.begin();itor != m_objIpcButtons.end();itor++)
    {
        QPointF qgs = m_pTileSource->ll2qgs(devOldPositionMap[itor.key()],zoomLevel);
        itor.value()->moveCenterPoint(qgs);
    }

    QMap<int, GisMapGraphicSubMapButton*>::iterator itorSubMap;
    for(itorSubMap = m_objSubMapButtons.begin();itorSubMap != m_objSubMapButtons.end();itorSubMap++)
    {
        QPointF qgs = m_pTileSource->ll2qgs(subMapOldPositionMap[itorSubMap.key()],zoomLevel);
        itorSubMap.value()->movePosition(qgs);
    }
}

void GisMapGraphWidget::zoomGisMapAndProOther(bool zoomIn)
{
    //获取之前ll
    QMap<int, QPointF> devOldPositionMap;
    QMap<int, QPointF> subMapOldPositionMap;
    getAllGisItemll(devOldPositionMap,subMapOldPositionMap);

    if(zoomIn)
    {
        this->zoomInGisMap(MouseZoom);
    }
    else
    {
        this->zoomOutGisMap(MouseZoom);
    }

    //移动设备
    moveAllGisItem(devOldPositionMap,subMapOldPositionMap,this->zoomLevel());
}

void GisMapGraphWidget::wheelEvent(QWheelEvent *event)
{
    if(m_curMapMode == GisMap)
    {
        if(m_pTileSource == NULL)
        {
            return;
        }
        this->zoomGisMapAndProOther(event->delta() > 0 ? true : false);
    }
    else
    {
        QFileInfo file(m_objMapPath);
        if(!file.exists())
        {
            qDebug() << "GisMapGraphWidget::wheelEvent failed, file not exist" << m_objMapPath;
            return;
        }
        scaleView(pow((double)2, -event->delta() / 240.0));
    }
    QGraphicsView::wheelEvent(event);
}

void GisMapGraphWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && m_bDrawing && m_objAreaSelect)
    {
        m_objAreaSelect->updateAreaParam(mapToScene(event->pos()),QPointF(),true);
    }
    QGraphicsView::mousePressEvent(event);
}

void GisMapGraphWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(m_bDrawing && m_objAreaSelect)
    {
        m_objAreaSelect->updateAreaParam(QPointF(),mapToScene(event->pos()),true);
    }
    if(m_bNoticeCenterChange)
    {
        QPoint view(width() / 2,height() / 2);
        QPointF centerPoint = this->mapToScene(view);
        QPointF ll = m_pTileSource->qgs2ll(centerPoint,this->zoomLevel());
        emit centerPointChanged(ll,zoomLevel());
    }
    QGraphicsView::mouseMoveEvent(event);
}

void GisMapGraphWidget::setNoticeCenterChange(bool notice)
{
    m_bNoticeCenterChange = notice;
}

void GisMapGraphWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && m_bDrawing && m_objAreaSelect)
    {
        m_bDrawing = false;
        m_objAreaSelect->updateAreaParam(QPointF(),mapToScene(event->pos()),false);
        slotRecoveryDrag();
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void GisMapGraphWidget::doSelectArea()
{
    setDragModeGisMap(GisDragMode::NoDrag);
    if(m_objAreaSelect == NULL)
    {
        m_objAreaSelect = new GisAreaSelectObj(this);
        connect(m_objAreaSelect, SIGNAL(signalSelectFinished(QRectF)), this,  SLOT(slotSelectFinished(QRectF)));
    }
    m_bDrawing = true;
}

void GisMapGraphWidget::slotRecoveryDrag()
{
    setDragModeGisMap(GisDragMode::ScrollHandDrag);
    m_bDrawing = false;
}

void GisMapGraphWidget::slotSelectFinished(QRectF rect)
{
    QList<AreaSelectUnitWidget::Param> list;
    QMap<int, GisMapItemIpc*>::iterator itor;
    for(itor = m_objIpcButtons.begin();itor != m_objIpcButtons.end();itor++)
    {
        int channelId;
        QPointF point;
        itor.value()->getCenterPoint(channelId,point);
        if(point.x() >= rect.x() && point.x() <= rect.x() + rect.width()
          && point.y() >= rect.y() && point.y() <= rect.y() + rect.height())
        {
            AreaSelectUnitWidget::Param pa;
            InviewChannel* channel = g_SppClient->getChannelById(channelId);
            if(channel)
            {
                pa.devName = QString::fromStdString(channel->get_channel_name());
                int recordState = channel->get_channel_recordState();
                QString state;
                if(recordState == 0)
                {
                    state = QObject::tr("TK_RecordStop");
                }
                else if(recordState == 1)
                {
                    state = QObject::tr("TK_Recording");
                }
                pa.recorderStatus = state;

                pa.type = (ENUM_INVIEW_DEVICE_TYPE)channel->get_channel_type_id();
                pa.number = list.size() + 1;
                pa.channelId = channelId;
                list.append(pa);
            }
        }
    }
    if(!list.isEmpty())
    {
        emit signalOpenSelectDialog(list);
    }
}

void GisMapGraphWidget::getCenterAndZoomLevel(QPointF &center,int &zoomLevel)
{
    zoomLevel = m_zoomLevel;
    QPoint point(width() / 2,height() / 2);
    QPointF scenePoint = mapToScene(point);
    center = m_pTileSource->qgs2ll(scenePoint,this->zoomLevel());
}

void GisMapGraphWidget::setCenterAndZoomlevel(QPointF point,int zoomLevel)
{
    //获取之前ll
    QMap<int, QPointF> devOldPositionMap;
    QMap<int, QPointF> subMapOldPositionMap;
    getAllGisItemll(devOldPositionMap,subMapOldPositionMap);

    setZoomLevel(zoomLevel);
    centerOnGisMap(point.x(),point.y());

    //移动设备
    moveAllGisItem(devOldPositionMap,subMapOldPositionMap,this->zoomLevel());
}

void GisMapGraphWidget::slotResourceFilter(ResourcesSettingDialog::ResourcesFilter filter)
{
    m_filter = filter;
    doResourceFilter();
}

void GisMapGraphWidget::doResourceFilter()
{
    QMap<int, GisMapItemIpc*>::iterator itor;
    for(itor = m_objIpcButtons.begin();itor != m_objIpcButtons.end();itor++)
    {
        int channelId = itor.key();
        int channelState;
        GisMapItemIpc::ShowParam param = GisMapItemIpc::ShowParam::NotShowAny;
        if(devIsContained(channelId,channelState))
        {
            if((channelState == INVIEW_DEVICE_STATE_ONLINE && m_filter.online)
               || (channelState != INVIEW_DEVICE_STATE_ONLINE && m_filter.offline))
            {
                if(m_filter.showArea)
                {
                    param = GisMapItemIpc::ShowParam::ShowDevAndArea;
                }
                else
                {
                    param = GisMapItemIpc::ShowParam::ShowDevOnly;
                }
            }
        }
        itor.value()->showDeviceObj(param);
    }
}

bool GisMapGraphWidget::devIsContained(int channelId,int &channelState)
{
    //过滤规则是否包含该设备类型
    channelState = DM_NO_USE_VALUE;
    InviewChannel* pChannel = g_SppClient->getChannelById(channelId);
    if(!pChannel)
    {
        return false;
    }
    int typeId = pChannel->get_channel_type_id();
    for(int i = 0;i < m_filter.showList.size();i++)
    {
        if(m_filter.showList.at(i) == typeId)
        {
            channelState = pChannel->get_channel_state();
            return true;
        }
    }
    return false;
}

void GisMapGraphWidget::saveLastViewParam()
{
    //保存根节点浏览参数
    QPointF centerPoint;
    int zoomLevel;
    getCenterAndZoomLevel(centerPoint,zoomLevel);
    m_lastViewParam.zoomLevel = zoomLevel;
    m_lastViewParam.centerPoint = centerPoint;
}
