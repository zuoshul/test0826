#ifndef GISMAPGRAPHWIDGET_H
#define GISMAPGRAPHWIDGET_H

#include <QGraphicsView>
#include <QList>
#include <QPoint>
#include <QMap>
#include <QAction>
#include <QContextMenuEvent>
#include <QDropEvent>
#include <QSvgWidget>
#include <QLabel>
#include <QHash>
#include <QStringBuilder>
#include "src/gis/old/gismapgraphbutton.h"
#include "src/gis/old/gismapmonitorarea.h"
#include "src/gis/old/gismapgraphicsubmapbutton.h"
#include "src/gis/old/gismapgraphipcbutton.h"
#include "mapipcconnectionline.h"
#include "src/common/Data.h"
#include "src/common/snmessagedialog.h"
#include "src/gis/guts/MapTileGraphicsObject.h"
#include "src/gis/tileSources/MapTileSource.h"
#include "src/gis/MapGraphicsObject.h"
#include "src/gis/tool/areaselectunitwidget.h"
#include "src/gis/tool/resourcessettingdialog.h"

class GisMapItemIpc;
class GisAreaSelectObj;

const QPointF PointWuHan(114.2935,30.5623);

typedef struct
{
    int id;
    QString buttonName;
    QPointF centerPoint;
    QPoint firstPoint;
    QPoint secondPoint;
}GisMapButton;

enum GisDragMode
{
    NoDrag,
    ScrollHandDrag,
    RubberBandDrag
};

enum ZoomMode
{
    CenterZoom,
    MouseZoom
};

enum MapMode
{
    GisMap,
    LocalMap
};

struct GisLastViewParam
{
    GisLastViewParam()
    {
        centerPoint = PointWuHan;
        zoomLevel = 4;
    }
    QPointF centerPoint;
    int zoomLevel;
};

class GisMapGraphWidget : public QGraphicsView
{
    Q_OBJECT

public:
    GisMapGraphWidget(QWidget *parent = 0);

    virtual ~GisMapGraphWidget();

    //切换地图
    void setMapInfo(const QString& mapPath,
                    QList<GisMapButton>& subMapButtons,
                    QList<GisMapButton>& ipcButtons);

    void setGisMapInfo(QList<GisMapButton>& subMapButtons,QList<GisMapButton>& ipcButtons);

    //在当前地图上添加子地图按钮
    void addSubMapButtons(QList<GisMapButton>& buttons);

    //在当前地图上删除子地图按钮
    void deleteSubMapButtons(QList<int>& buttons);

    //在当前地图上添加IPC按钮
    void addIpcButtons(QList<GisMapButton>& buttons);

    //在当前地图上删除IPC按钮
    void deleteIpcButtons(QList<int>& buttons);

    //切换地图编辑状态。
    void switchMapState(MapState mapState);

    //让地图上于btnId相同的ipc开始闪烁报警
    void startAlarm(int btnId);

    //停止闪烁报警
    void stopAlarm(int btnId);

    //在地图上添加ipcs所指定几个button，并通过直线顺序连接
    void addTrack(QList<GisMapButton>& ipcs);

    //轨迹播放
    void moveFlag(const QString& flagImage, const QPoint& postion);

    //清理地图上所有数据
    void clearMapInfo();

    //获取设备真实坐标
    bool getDevPosition(int channelId,QPointF &devPoint);

    void doSelectArea();

    void getCenterAndZoomLevel(QPointF &center,int &zoomLevel);

    void doResourceFilter();

    void setNoticeCenterChange(bool notice);

    void setCenterAndZoomlevel(QPointF point,int zoomLevel);

    void saveLastViewParam();

protected:

#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif

	void wheelEvent(QWheelEvent *event) override;

	void drawBackground(QPainter *painter, const QRectF &rect) override;

	void scaleView(qreal scaleFactor);

    virtual void dropEvent(QDropEvent *event);

    void dragMoveEvent(QDragMoveEvent *event);

signals:
    void signalAddButtonClicked(QPointF);

    void signalAddMap(QPointF pos);

	void signalDelMap(int mapId);

	void signalSubMapButtonClicked(int mapId);

    void signalMapInfoChanged(int channelId, QPointF position);

    void signalAddDev(Node, QPointF);

    void signalDelDevice(int devId);

    void signalIpcButtonClicked(int ipcId);

    void signalIpcButtonPress(int ipcId);

    void signalIpcInfoChanged(int channelId, QPointF position, QPoint point1, QPoint point2);

    void scaled(double percent);

    //gis
    void zoomLevelChanged(quint8 nZoom);

    void signalOpenSelectDialog(QList<AreaSelectUnitWidget::Param> list);

    void centerPointChanged(QPointF,int);

public slots:
    void slotZoomIn();

    void reset();

    void slotZoomOut();

    void slotAddMap();

    void slotDelteMap();

    void slotDelteDev();

    void slotConfirmDelDev();

    void slotConfirmDelMap();

    void slotZoomInGisMap();

    void slotMapInfoChanged(int channelId, QPointF position);

    void slotIpcInfoChanged(int channelId,QPointF center,QPoint first, QPoint second);

    void slotSelectFinished(QRectF rect);

    void slotRecoveryDrag();

    void slotResourceFilter(ResourcesSettingDialog::ResourcesFilter filter);

private:
    void addSubMapButtonsToMap(QList<GisMapButton>& buttons);

    //showConnection: 是否在两个button之间使用直线连接
    void addIpcButtonsToMap(QList<GisMapButton>& buttons, bool showConnection = false);

	void initActions();

	bool isMapExist();

    void changeDragableState(bool dragAble);

    //gis
    void doTileLayout();

    quint8 zoomLevel();

    void setZoomLevel(quint8 nZoom, ZoomMode zMode = CenterZoom);

    void resetQGSSceneSize();

    QSharedPointer<MapTileSource> tileSource() const;

    void setTileSource(QSharedPointer<MapTileSource> tSource);

    void centerOnGisMap(qreal longitude, qreal latitude);

    void centerOnGisMap(const QPointF &pos);

    void centerOnGisMap(const MapGraphicsObject *item);

    QPointF mapToSceneGisMap(const QPoint viewPos);

    GisDragMode dragMode();

    void setDragModeGisMap(GisDragMode mode);

    void zoomInGisMap(ZoomMode zMode);

    void zoomOutGisMap(ZoomMode zMode);

    void initGis();

    void zoomGisMapAndProOther(bool zoomIn);

    bool devIsContained(int channelId,int &channelState);

    void getAllGisItemll(QMap<int, QPointF> &devOldPositionMap,QMap<int, QPointF> &subMapOldPositionMap);

    void moveAllGisItem(QMap<int, QPointF> devOldPositionMap,QMap<int, QPointF> subMapOldPositionMap,int zoomLevel);

    virtual void mouseDoubleClickEvent(QMouseEvent *event);

    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

private:
    QString m_objMapPath;
    QMap<int, GisMapGraphicSubMapButton*> m_objSubMapButtons;
    QMap<int, GisMapItemIpc*> m_objIpcButtons;

    QAction* m_objAddMap;
    QAction* m_objDeleteMap;
    QAction* m_objDeleteDev;

    GisMapGraphButton* m_pButtonSelect;//仅用于右键选中删除地图
    QPointF	m_objCurMousePos; //当前鼠标右键点击的位置,仅用于添加地图
    MapState m_nCurMapState; //state

    //轨迹Play
    QSvgWidget* m_pBubbleWidget;
    QLabel* m_pBubbleLabel;
    QGraphicsProxyWidget* m_pBubbleWidgetItem;
    QString curFlagImage;

    SNMessageDialog *m_pDelDevDlg;
    SNMessageDialog *m_pDelMapDlg;
    double m_scalePercent;

    //gis相关
    QSharedPointer<MapTileSource> m_pTileSource;
    QSet<MapTileGraphicsObject *> m_tileObjects;
    quint8 m_zoomLevel;
    GisDragMode m_dragMode;
    MapMode m_curMapMode;
    GisLastViewParam m_lastViewParam;

    //tool
    GisAreaSelectObj* m_objAreaSelect;
    bool m_bDrawing;
    ResourcesSettingDialog::ResourcesFilter m_filter;
    bool m_bNoticeCenterChange;

    //本模块比较复杂，严禁任何人滥增成员变量，尽量使用局部变量，同一类型数据尽量使用结构体或类，不用的成员变量及时删除，不同成员变量请分类注释！！！ zuoshul20180901
};

inline uint qHash(const QPointF& key)
{
    const QString temp = QString::number(key.x()) % "," % QString::number(key.y());
    return qHash(temp);
}

#endif // GISMAPGRAPHWIDGET_H
