#ifndef GISMAPWIDGET_H
#define GISMAPWIDGET_H

#include <QWidget>
#include <QTimerEvent>
#include <QMap>
#include "gisaddnewmapdialog.h"
#include "src/base/basewidget.h"
#include "src/sppclient/sppclientmapmanager.h"
#include "src/common/Data.h"
#include "src/base/basewidget.h"
#include "src/miniplayer/videosubwinfactory.h"
#include "src/sppclient/sppclientalarmmanager.h"
#include "maputil.h"
#include "src/gis/video/mapalarmvideowidget.h"
#include "src/common/snmessagedialog.h"
#include "sntoolbox.h"
#include "gismapgraphwidget.h"
#include "src/gis/tool/functiontoolwidget.h"
#include "src/gis/tool/resourcessettingdialog.h"
#include "src/gis/tool/mapsearchdialog.h"
#include "src/gis/tool/areaselectdialog.h"
#include "src/gis/tool/alarmpopdialog.h"
#include "gisaddfirstmapwidget.h"
#include "gisaddmapnodedialog.h"
#include "src/gis/tool/gismapnodeunitwidget.h"
#include "src/gis/tool/zoomtoolwidget.h"

const QString RootMapCode = "001";

namespace Ui {
class GisMapWidget;
}

class GisMapWidget : public BaseWidget
{
    Q_OBJECT

public:
    explicit GisMapWidget(QWidget *parent = 0);

    ~GisMapWidget();

    virtual void timerEvent(QTimerEvent *event);
    //界面关闭时释放资源
    void releaseResource();

    bool eventFilter(QObject *watched, QEvent *event);

    int getCurLayoutMode();

    QVector< QPair<int, int> > getCurrLayoutList();

signals:
    void devPlay(const int&, const QVector<int>&);

    void doubleClickedPlay(const int&, const int&);

public slots:
    void slot_sendWarning(const QString& p_strText);

protected:
    virtual void showEvent(QShowEvent *event);

    virtual void resizeEvent(QResizeEvent *event);

private slots:
    //点击了地图添加按钮
    void slotAddButtonClicked(QPointF pos);

	//点击了删除地图
    void slotDelMapClicked(int mapId);

    //地图上的子地图位置被拖动
    void slotMapInfoChanged(int mapId, QPointF position);

    //通过拖拽向地图中添加一个设备
    void slotAddDev(Node, QPointF);

	//点击了删除设备
    void slotDelDevClicked(int channelId);

    //地图上的IPC监控范围变化
    void slotDevInfoChanged(int devId, QPointF position, QPoint point1, QPoint point2);

    //通过地图选择框选中地图后，触发
    void slotMapSelectFinished(QString mapName, QString mapPath);

    void slotGisMapModeSelected();

    void slotMapSelected(int mapId);

	void slotSubMapButtonClicked(int);

	void slotIpcButtonClicked(int);

    void on_pushButtonEdit_clicked();

    //删除当前地图
    void on_pushButtonDelete_clicked();

    void on_mapLayout1_clicked();

    void on_mapLayout2_clicked();

    void on_mapLayout3_clicked();

    void slotAlarmNotifyReceived(InviewAlarmMessage alarmMessage, int messageCount);

    void slotDownloadMapFinished(int state);

    void slotRenameMapClicked(int mapId,QString strNewName);

    void slot_signalDeviceInfoChanged();

    //主界面上删除地图
    void slotConfirmDeleteMapMain();

    void on_pushButtonAdd_clicked();

    //右键播放全部
    void slot_startPlay(const QVector<int>& p_vecChannelId);

    //双击设备播放
    void slot_doubleClickedPlay(const int& p_nChannelId);

    //设备树右键菜单切流播放
    void slot_changeStream(const int& p_nStreamId, const int& p_nChannelId);

    //窗口被选中
    void slot_winSelected(const QString& p_strChannelUID);

    //设备按钮被单击
    void slotIpcButtonPress(int channelId);

    //手动关闭视频窗口处理
    void slot_closeAll();

    //手动关闭视频窗口处理
    void slot_close(const int& channelId);

    //清除线
    void slotClearDrawLine();

    void slotToolMenuClicked(FunctionToolWidget::ClickStatus status);

    void slotStartAlarmSearch(QList<int> channelList);

    void slotStartLiveview(QList<int> channelList);

    void slotAlarmDlgClicked();

    void slotResourceFilter(ResourcesSettingDialog::ResourcesFilter filter);

    void on_pushButtonAddGisMapNode_clicked();

    void slotMapNodeSetFinished(QString name,int parentid);

private:
    QString organizeMapCode(int parentId);

    void uploadStateChanged();

    QPoint convertStringToPoint(const QString& str);

    QPointF convertStringToPointF(const QString& str);

    QString convertPointToString(QPoint& point);

    QString convertPointToString(QPointF& point);

    void changeMapInfo(MapMode mode = LocalMap);

    bool addMapInfoToDB(InviewLocalMap& map);

    bool havaSubMap(int mapId);

    bool saveMapInfoChanged();

    void initMap();

    //调整地图及播放窗口的大小
    void adjustCurWindows();

    InviewLocalMap getMapById(int mapId);

    QString getMapNameByCode(QString& mapCode);

    void setCurMapFullName();

    void refreshMapInfo();

    void closeVideoByMapId(int mapId);

    bool verifyContents(QString strMapName);

    void startDrawLine(bool bStart);

    QPoint pointConver(int winid,QRect rect);

    void RecoveryPlay();

    void initMapTool();

    void moveToolWidget();

private:
    virtual void initForm();

    virtual void retranslateUi();

    void switchMapState(MapState state);

    void initCustomStyle();

private:
    Ui::GisMapWidget *ui;
    GisAddNewMapDialog* m_pAddNewMapDialog;
    std::vector<InviewLocalMap>* m_pMaps;
    int m_nUploadTimeId;
    QString m_mapLocalDir;

    InviewLocalMap m_objCurMapToAdd; //需要被写入数据库的地图信息，等待上传结束后再写入。
    //InviewLocalMap m_objCurMap; //当前被选中的地图信息
    QMap<int, InviewLocalMap> m_objSubMaps;
    QMap<int, InviewChannel> m_objChannels;
    QMap<int, InviewLocalMap> m_objSubMapsChanged;
    QMap<int, InviewChannel> m_objChannelsChanged;
    MapState m_mCurMapState;
    GisAddFirstMapWidget* m_pAddFirstMapWidget;

    //视频联动相关开始
    VideoSubWinFactory* m_pVideoSubWinFactory;
    QMap<int, QRect> m_objWindowRectList;   //当前每个窗口的尺寸
    QVector<LayoutinfoStruct> m_curChannelIDMap; //用来判断当前布局中的通道列表是否有变化
    LayoutModel m_nCurLayoutMode; //当前布局类型
    FunctionWidget m_nFunctionId; //当前窗口ID
    bool m_bIsFirstShow; //窗口第一次显示
    MapUtil* m_pMapUtil;
    MapAlarmVideoWidget* m_pObjAlarmVideoWidget;
    SNMessageDialog* m_pDelMapDlgMain;

    QWidget *m_lineWidget;
    bool m_bDrawLine;
    QPoint m_winPoint;
    QPointF m_devPoint;
    QTimer* m_pTimer;

    //工具相关
    ZoomToolWidget* m_pToolWidget;
    FunctionToolWidget* m_pTopToolWidget;
    ResourcesSettingDialog* m_pResourcesSettingDlg;
    MapSearchDialog* m_pMapSearchDlg;
    AreaSelectDialog* m_pAreaSelectDlg;
    QPushButton *m_pAlarmBtn;
    AlarmPopDialog *m_pAlarmPopDlg;
    bool m_bFullScreen;

    //定位
    GisAddMapNodeDialog* m_pAddGisMapNodeDlg;
    GisMapNodeUnitWidget* m_pGisMapNodeUnitWidget;
    InviewLocalMap m_objGisMapNodeToAdd;

    //本模块比较复杂，严禁任何人滥增成员变量，尽量使用局部变量，同一类型数据尽量使用结构体或类，不用的成员变量及时删除，不同成员变量请分类注释！！！ zuoshul20180901
};

#endif // GISMAPWIDGET_H
