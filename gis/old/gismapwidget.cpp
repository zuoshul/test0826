#include <QDebug>
#include <QEvent>
#include <QDir>
#include "gismapwidget.h"
#include "ui_gismapwidget.h"
#include "src/base/customevent.h"
#include "src/gis/old/gisaddnewmapdialog.h"
#include "src/common/snmessageboxflowdownwidget.h"
#include "src/sppclient/sppclientdevicemanager.h"
#include "gismaptreeview.h"
#include "src/base/apptextvalidator.h"
#include "basehelper.h"
#include <QGraphicsDropShadowEffect>
#include <QLine>
#include "gismaplivevideosubwin.h"
#include "src/gis/tool/devstatechangespecialpro.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

#define DEFAULT_MAP_WIDTH 1024
#define DEFAULT_MAP_HEIGHT 768

GisMapWidget::GisMapWidget(QWidget *parent) :
    BaseWidget(parent),
    ui(new Ui::GisMapWidget),
    m_mCurMapState(MapStatePreview),
    m_nCurLayoutMode(LAYOUT_MODEL_4),
    m_nFunctionId(FUNCTION_MAP),
    m_bIsFirstShow(true),
    m_pObjAlarmVideoWidget(NULL),
    m_pToolWidget(NULL),
    m_pTopToolWidget(NULL),
    m_pResourcesSettingDlg(NULL),
    m_pMapSearchDlg(NULL),
    m_pAlarmBtn(NULL),
    m_pAlarmPopDlg(NULL),
    m_pAddFirstMapWidget(NULL),
    m_pAddGisMapNodeDlg(NULL),
    m_pGisMapNodeUnitWidget(NULL),
    m_bFullScreen(false)
{
    ui->setupUi(this);
    initForm();
}

GisMapWidget::~GisMapWidget()
{
    delete ui;
}

void GisMapWidget::initCustomStyle()
{
    setTitle(QObject::tr("TK_Map"));
    ui->pushButtonDelete->setFocusPolicy(Qt::NoFocus);
    ui->pushButtonDelete->setIcon(QIcon(":images/default/map/delete.svg"));
    ui->pushButtonAdd->setText(QObject::tr("TK_AddLocalMap"));
    ui->pushButtonAdd->setIcon(QIcon(":images/default/devmanage/add_16.png"));
    ui->pushButtonAddGisMapNode->setText(QObject::tr("TK_AddGisMapNode"));
    ui->pushButtonAddGisMapNode->setIcon(QIcon(":images/default/map/addGIS_16.png"));

    AppStyleHelper::updateWidgetStyle(ui->labelMapName, AppStyleHelper::Style_LabelTitleLevel2);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonDelete, AppStyleHelper::Style_PushButtonLight);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonEdit, AppStyleHelper::Style_PushButtonLight);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonAdd, AppStyleHelper::Style_PushButtonLight);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonAddGisMapNode, AppStyleHelper::Style_PushButtonLight);
}

void GisMapWidget::initForm()
{
    initCustomStyle();

    //下载地图
    m_pMapUtil = new MapUtil(this);
    m_mapLocalDir = MapUtil::getLocalMapDirPath();
    qDebug() << "GisMapWidget::initForm() maps local dir " << m_mapLocalDir;
    connect(m_pMapUtil, SIGNAL(downloadFinished(int)), this, SLOT(slotDownloadMapFinished(int)));

    ToolboxShowParam param;
    param.showDevice = true;
    param.showMap = true;
    param.expand = MapWidget_Expand;
    ui->toolbox->initToolBoxParam(param);

    m_pAddNewMapDialog = new GisAddNewMapDialog(this);
    connect(m_pAddNewMapDialog,
            SIGNAL(signalMapSelectFinished(QString, QString)),
            this,
            SLOT(slotMapSelectFinished(QString, QString)));

    connect(m_pAddNewMapDialog,
            SIGNAL(gisMapModeSelected()),
            this,
            SLOT(slotGisMapModeSelected()));

    connect(ui->widgetMap,
            SIGNAL(signalAddButtonClicked(QPointF)),
            this,
            SLOT(slotAddButtonClicked(QPointF)));

	connect(ui->widgetMap,
        SIGNAL(signalAddMap(QPointF)),
		this,
        SLOT(slotAddButtonClicked(QPointF)));

	connect(ui->widgetMap,
		SIGNAL(signalDelMap(int)),
		this,
		SLOT(slotDelMapClicked(int)));

	connect(ui->widgetMap,
		SIGNAL(signalDelDevice(int)),
		this,
		SLOT(slotDelDevClicked(int)));

	connect(ui->widgetMap,
		SIGNAL(signalSubMapButtonClicked(int)),
		this,
		SLOT(slotSubMapButtonClicked(int)));

	connect(ui->widgetMap,
        SIGNAL(signalIpcButtonClicked(int)),
		this,
		SLOT(slotIpcButtonClicked(int)));

    connect(ui->widgetMap,
        SIGNAL(signalIpcButtonPress(int)),
        this,
        SLOT(slotIpcButtonPress(int)));

	connect(ui->widgetMap,
        SIGNAL(signalAddDev(Node, QPointF)),
		this,
        SLOT(slotAddDev(Node , QPointF)));

    connect(ui->widgetMap,
        SIGNAL(signalMapInfoChanged(int, QPointF)),
        this,
        SLOT(slotMapInfoChanged(int, QPointF)));

    connect(ui->widgetMap,
        SIGNAL(signalIpcInfoChanged(int, QPointF, QPoint, QPoint)),
        this,
        SLOT(slotDevInfoChanged(int, QPointF, QPoint, QPoint)));

    connect(ui->toolbox->getMapObj(),
            SIGNAL(signalMapSelected(int)),
            this,
            SLOT(slotMapSelected(int)));

    connect(ui->toolbox->getMapObj(),
            SIGNAL(signalAddMap(QPointF)),
            this,
            SLOT(slotAddButtonClicked(QPointF)));

    connect(ui->toolbox->getMapObj(),
        SIGNAL(signalDelMap(int)),
        this,
        SLOT(slotDelMapClicked(int)));

    connect(ui->toolbox->getMapObj(),
        SIGNAL(signalRenameMap(int,QString)),
        this,
        SLOT(slotRenameMapClicked(int,QString)));

    if(g_SppClient->getCurUserId() != 1)
    {
        //只有超级管理员可以管理地图
        ui->pushButtonEdit->hide();
        ui->pushButtonDelete->hide();
        ui->pushButtonAdd->hide();
    }

    //默认选中第二个窗体
    m_pVideoSubWinFactory = new VideoSubWinFactory(ui->widgetMapContainer);
    m_pVideoSubWinFactory->createAllSubWin(m_nFunctionId,1,6);
    m_pVideoSubWinFactory->slot_selectWindow(2);

    connect(g_SPPClientAlarmManager, SIGNAL(signalAlarmNotifyReceived(InviewAlarmMessage, int)),this, SLOT(slotAlarmNotifyReceived(InviewAlarmMessage, int)));
    connect(this, SIGNAL(devPlay(const int&, const QVector<int>&)),m_pVideoSubWinFactory, SLOT(slot_devPlay(const int&, const QVector<int>&)));
    connect(this, SIGNAL(doubleClickedPlay(const int&, const int&)),m_pVideoSubWinFactory, SLOT(slot_doubleClickedPlay(const int&, const int&)));
    connect(m_pVideoSubWinFactory, SIGNAL(sendWarning(const QString&)),this, SLOT(slot_sendWarning(const QString&)));
    connect(m_pVideoSubWinFactory, SIGNAL(winSelected(const QString&)),ui->toolbox->getDeviceObj(), SLOT(slot_winSelected(const QString&)));
    connect(m_pVideoSubWinFactory, SIGNAL(winSelected(const QString&)),this, SLOT(slot_winSelected(const QString&)));

    connect(m_pVideoSubWinFactory, SIGNAL(closeAll()),this, SLOT(slot_closeAll()));
    connect(m_pVideoSubWinFactory, SIGNAL(close(const int&)),this, SLOT(slot_close(const int&)));

    //单布局窗口播放视频功能
    if(NULL == m_pObjAlarmVideoWidget)
    {
        m_pObjAlarmVideoWidget = new MapAlarmVideoWidget(this);
        if(NULL != m_pObjAlarmVideoWidget)
        {
            m_pObjAlarmVideoWidget->setTitleIcon("QLabel{image:url(:/images/default/map/map_device.png);}");
            m_pObjAlarmVideoWidget->setVisible(false);
        }
    }

    //删除确认对话框
    m_pDelMapDlgMain = new SNMessageDialog(this);
    connect(m_pDelMapDlgMain,SIGNAL(signalConfirmed()),this,SLOT(slotConfirmDeleteMapMain()));

    //设备树
    ui->toolbox->getDeviceObj()->setSelectedRowsFlag(false);
    //不显示人体测温仪第二个通道
    ui->toolbox->getDeviceObj()->setChannelFilterType(OnlyShowTempSecondChannel);
    connect(ui->toolbox->getDeviceObj(), SIGNAL(startPlay(const QVector<int>&)),
            this, SLOT(slot_startPlay(const QVector<int>&)));
	connect(ui->toolbox->getDeviceObj(), SIGNAL(stopPlay(const QVector<int>&)),
			m_pVideoSubWinFactory, SLOT(slot_stopPlay(const QVector<int>&)));
    connect(ui->toolbox->getDeviceObj(), SIGNAL(doubleClickedPlay(const int&)),
            this, SLOT(slot_doubleClickedPlay(const int&)));
    connect(ui->toolbox->getDeviceObj(), SIGNAL(changeStream(const int&, const int&)),
            this, SLOT(slot_changeStream(const int&, const int&)));

    //划线层叠控件
    m_lineWidget = new QWidget(ui->widgetMap);
    m_lineWidget->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    m_lineWidget->installEventFilter(this);
    m_bDrawLine = false;
    m_pTimer = new QTimer(this);
    connect(m_pTimer,SIGNAL(timeout()), this, SLOT(slotClearDrawLine()));

    m_pAddFirstMapWidget = new GisAddFirstMapWidget(ui->widgetMap);
    connect(m_pAddFirstMapWidget,SIGNAL(signalAddButtonClicked(QPointF)),this,SLOT(slotAddButtonClicked(QPointF)));
    m_objGisMapNodeToAdd.set_parent_map_id(DM_NO_USE_VALUE);

    //初始化工具控件
    initMapTool();

    retranslateUi();
}

void GisMapWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if(m_bIsFirstShow)
    {
        m_bIsFirstShow = false;

        //初始化地图数据
        initMap();

        //调整地图及播放窗口大小
        adjustCurWindows();

        RecoveryPlay();
    }
}

void GisMapWidget::initMap()
{
    m_pMaps = g_SPPClientMapManager->listMaps();
    m_pAddFirstMapWidget->showAddMapBtn(m_pMaps->empty() && (g_SppClient->getCurUserId() == 1));
    ui->toolbox->getMapObj()->setRootAsCurrentMap();
    if(m_pMaps->empty() && g_SppClient->getCurUserId() == 1)
    {
        //如果没有添加过地图，同时是超级管理员，默认开启编辑
        switchMapState(MapStateEdit);
    }
    else
    {
        //默认地图查看状态
        switchMapState(MapStatePreview);
    }
}

void GisMapWidget::releaseResource()
{
    if(NULL != m_pVideoSubWinFactory)
    {
        m_pVideoSubWinFactory->releaseResource();
    }
}

void GisMapWidget::adjustCurWindows()
{
    if(NULL == m_pVideoSubWinFactory)
    {
        return;
    }

    QRect rect = ui->widgetMapContainer->rect();
    rect.setX(0);
    rect.setY(0);
    m_pVideoSubWinFactory->splitForm(m_nCurLayoutMode,rect,m_objWindowRectList);

    QMap<int, QRect>::iterator recIt = m_objWindowRectList.begin();
    if(recIt != m_objWindowRectList.end())
    {
        ui->widgetMap->setGeometry(recIt.value());
        //第一个窗口显示地图，其他窗口用来播放视频
        m_objWindowRectList.erase(recIt);
    }
    else
    {
        ui->widgetMap->setGeometry(rect);
    }

    m_pVideoSubWinFactory->hideAllSubWin(); //以免留下上次布局的残影
    m_pVideoSubWinFactory->setSubWinPosition(m_objWindowRectList);
    m_pVideoSubWinFactory->resizeWindow(m_objWindowRectList);

    m_lineWidget->setStyleSheet("background-color:transparent");
    m_lineWidget->resize(ui->widgetMap->size());

    moveToolWidget();
}

void GisMapWidget::resizeEvent(QResizeEvent *)
{
    adjustCurWindows();
}

void GisMapWidget::retranslateUi()
{
    ui->pushButtonDelete->setText(QObject::tr("TK_Delete"));
    if(ui->pushButtonDelete->isHidden())
    {
        ui->pushButtonEdit->setText(QObject::tr("TK_Edit"));
    }
    else
    {
        ui->pushButtonEdit->setText(QObject::tr("TK_ExitEdit"));
    }
    ui->pushButtonAdd->setText(QObject::tr("TK_AddLocalMap"));

    ui->mapLayout1->setToolTip(QObject::tr("TK_OneLayout"));
    ui->mapLayout2->setToolTip(QObject::tr("TK_FourLayout"));
    ui->mapLayout3->setToolTip(QObject::tr("TK_SixLayout"));
    setTitle(QObject::tr("TK_Map"));
    ui->retranslateUi(this);
}

void GisMapWidget::slot_sendWarning(const QString& p_strText)
{
    SNMessageBoxFlowDownWidget::showMessage(p_strText, this, SNMessageBoxFlowDownWidget::MessageInfo);
}

void GisMapWidget::switchMapState(MapState state)
{
    switch (state)
    {
    case MapStatePreview:
        ui->pushButtonEdit->setText(QObject::tr("TK_Edit"));
        ui->pushButtonEdit->setIcon(QIcon(":images/default/map/modify.svg"));
        ui->pushButtonDelete->hide();
        ui->pushButtonAdd->hide();
        ui->pushButtonAddGisMapNode->hide();

        //所有子地图，设备不能拖动，只能接受双击事件
        //不响应鼠标右键事件
        //不接受来至地图树和设备树的拖拽事件
        ui->widgetMap->switchMapState(MapStatePreview);
        ui->toolbox->getMapObj()->switchMapState(MapStatePreview);
        break;

    case MapStateEdit:
        //编辑按钮用于保存
        ui->pushButtonEdit->setText(QObject::tr("TK_ExitEdit"));
        ui->pushButtonEdit->setIcon(QIcon(":images/default/map/saved.svg"));

        if(m_pMaps->empty() && g_SppClient->getCurUserId() == 1)
        {
            ui->pushButtonDelete->hide();
            ui->pushButtonEdit->hide();
        }
        else
        {
            ui->pushButtonDelete->show();
            ui->pushButtonEdit->show();
        }
        ui->pushButtonAdd->show();
        if(ui->toolbox->getMapObj()->getOperatorMap().get_local_map_type() == MSG_MAP_TYPE_NODE)
        {
            ui->pushButtonAddGisMapNode->show();
        }
        else
        {
            ui->pushButtonAddGisMapNode->hide();
        }

        //所有子地图，设备能拖动，能接受双击事件
        //响应鼠标右键事件
        //接受来至地图书和设备树的拖拽事件
        ui->widgetMap->switchMapState(MapStateEdit);
        ui->toolbox->getMapObj()->switchMapState(MapStateEdit);
        break;

    default:
        //默认预览
        ui->pushButtonEdit->setText(QObject::tr("TK_Edit"));
        ui->pushButtonDelete->hide();
        ui->pushButtonAdd->hide();
        ui->pushButtonAddGisMapNode->hide();
        ui->widgetMap->switchMapState(MapStatePreview);
        ui->toolbox->getMapObj()->switchMapState(MapStatePreview);
        break;
    }
    m_mCurMapState = state;
}

void GisMapWidget::slotAddButtonClicked(QPointF pos)
{
    //添加地图-All
    if(m_pMaps->size() >= 6)
    {
        //新增MAP级数限制，最多不能超过10级,zuoshul20180921
        int nCount = 0;
        InviewLocalMap tmpMapObj = ui->toolbox->getMapObj()->getOperatorMap();
        while(tmpMapObj.get_id() != tmpMapObj.get_parent_map_id())
        {
            nCount ++;
            tmpMapObj = getMapById(tmpMapObj.get_parent_map_id());
        }
        if(nCount >= 5)
        {
            SNMessageBoxFlowDownWidget::showMessage(QObject::tr("TK_TooMuchSubMap"),this,SNMessageBoxFlowDownWidget::MessageWarring);
            return;
        }
    }
    qDebug() << "GisMapWidget::slotAddButtonClicked() pos " << pos;
	m_objCurMapToAdd.set_local_map_postion_x(pos.x());  //需要修改
	m_objCurMapToAdd.set_local_map_position_y(pos.y()); //需要修改
    m_pAddNewMapDialog->openDialog(m_pMaps->empty());
}

void GisMapWidget::slotDelMapClicked(int mapId)
{
    //地图树上面右键删除地图

	//拥有子地图，不允许删除
	if (havaSubMap(mapId))
	{
		SNMessageBoxFlowDownWidget::showMessage(
            QObject::tr("TK_HaveSubMap"),
			this,
			SNMessageBoxFlowDownWidget::MessageError);
		return;
	}

	InviewIDList ids;
	ids.push_back(mapId);
	if (!g_SPPClientMapManager->deleteMap(ids))
	{
		SNMessageBoxFlowDownWidget::showMessage(
            QObject::tr("TK_ImageDeleteFaile"),
			this,
			SNMessageBoxFlowDownWidget::MessageError);
    }

    QList<int> buttons;
    buttons.push_back(mapId);

    //刪除子地图图标
    ui->widgetMap->deleteSubMapButtons(buttons);
    ui->toolbox->getMapObj()->setRootAsCurrentMap();

    closeVideoByMapId(mapId);

    refreshMapInfo();
    SNMessageBoxFlowDownWidget::showMessage(QObject::tr("TK_DeleteSuccess"), this, SNMessageBoxFlowDownWidget::MessageInfo);
}

void GisMapWidget::slotRenameMapClicked(int mapId,QString strNewName)
{
    //点保存后统一去获取最新的名字
    if(!verifyContents(strNewName))
    {
        return;
    }
    InviewLocalMap tmpMap = ui->toolbox->getMapObj()->getOperatorMap();
    tmpMap.set_local_map_name(strNewName.toStdString());
    m_objSubMapsChanged.insert(mapId,tmpMap);

    QString strFullName = ui->labelMapName->text();
    int index = strFullName.lastIndexOf(">>");
    if(index != -1)
    {
        strFullName = strFullName.mid(0,index + 2);
        strFullName += strNewName;
    }
    else
    {
        strFullName = strNewName;
    }
    ui->labelMapName->setText(strFullName);
    SNMessageBoxFlowDownWidget::showMessage(QObject::tr("TK_ModifySuccess"), this, SNMessageBoxFlowDownWidget::MessageInfo);
}

bool GisMapWidget::havaSubMap(int mapId)
{
	for (int i = 0; i < m_pMaps->size(); i++)
	{
        if (m_pMaps->at(i).get_parent_map_id() == mapId && m_pMaps->at(i).get_id() != mapId)
		{
			return true;
		}
	}
	return false;
}

void GisMapWidget::slotAddDev(Node devNode, QPointF devPos)
{
    //添加设备
    if(!g_SppClient->isSignalChannelDvice(devNode.nodeType) || 1 < devNode.nChannelCount)
    {
        qDebug() << "MapWidget::slotAddDev signal channel device support only!";
        return;
    }

    InviewChannel* channel = g_SppClient->getChannelById(devNode.id);
    if(NULL != channel)
    {
        GisMapButton button;
        button.id = channel->get_id();
        button.buttonName = devNode.strNodeName;
        button.centerPoint = devPos;
        button.firstPoint = devPos.toPoint() + QPoint(100, 45);
        button.secondPoint = devPos.toPoint() + QPoint(100, -45);

        QList<GisMapButton> buttons;
        buttons.push_back(button);
        ui->widgetMap->addIpcButtons(buttons);

        channel->set_position(convertPointToString(button.centerPoint).toStdString());
        channel->set_point1(convertPointToString(button.firstPoint).toStdString());
        channel->set_point2(convertPointToString(button.secondPoint).toStdString());
        InviewLocalMap tempMap = ui->toolbox->getMapObj()->getOperatorMap(true);
        channel->set_map_id(tempMap.get_id());
        m_objChannelsChanged.insert(channel->get_id(), *channel);

        //保存到buf中
        m_objChannels.insert(channel->get_id(), *channel);
    }
    else
    {
        qCritical() << "MapWidget::slotAddDev ipc " << devNode.id << " not found!";
    }
}

void GisMapWidget::slotMapInfoChanged(int mapId, QPointF position)
{
    //地图上的子地图位置被拖动
    qDebug() << "slotMapInfoChanged " << mapId << " " <<position;
    QMap<int, InviewLocalMap>::iterator it = m_objSubMapsChanged.find(mapId);

    if(it != m_objSubMapsChanged.end())
    {
        it.value().set_local_map_postion_x(position.x());
        it.value().set_local_map_position_y(position.y());
    }
    else
    {
        it = m_objSubMaps.find(mapId);
        if(it != m_objSubMaps.end())
        {
            it.value().set_local_map_postion_x(position.x());
            it.value().set_local_map_position_y(position.y());
            m_objSubMapsChanged.insert(mapId, it.value());
        }
    }
}

void GisMapWidget::slotDelDevClicked(int channelId)
{
	QList<int> buttons;
    buttons.push_back(channelId);
	ui->widgetMap->deleteIpcButtons(buttons);

    InviewChannel* channel = g_SppClient->getChannelById(channelId);
    if(NULL != channel)
    {
        //先缓冲到cache，点击保存按钮时再更新
        channel->set_map_id(0);
        channel->set_position("0,0");
        channel->set_point1("0,100");
        channel->set_point2("100,0");
        m_objChannelsChanged.insert(channel->get_id(), *channel);

        //删除cache中的通道
        m_objChannels.remove(channel->get_id());

        //关闭视频窗口,zuoshul20180925
        QMap<int,QRect>::iterator  itor;
        for (itor = m_objWindowRectList.begin(); itor != m_objWindowRectList.end(); itor++)
        {
            int nWinId = itor.key();
            int nChannelId = m_pVideoSubWinFactory->getChannelId(nWinId);
            if(channelId == nChannelId)
            {
                GisMapLiveVideoSubWin* pWindow = (GisMapLiveVideoSubWin*)m_pVideoSubWinFactory->getVideoSubWin(nWinId);
                pWindow->slot_close();
            }
        }
    }
    else
    {
        qCritical() << "MapWidget::slotDelDevClicked channel: "<< channelId << " not found!";
    }
}

void GisMapWidget::slotDevInfoChanged(int devId, QPointF position, QPoint point1, QPoint point2)
{
    qDebug() << "GisMapWidget::slotDevInfoChanged " << devId
             << " " <<position
             << " " <<point1
             << " " <<point2;

    QMap<int, InviewChannel>::iterator mapIt = m_objChannelsChanged.find(devId);
    if(mapIt != m_objChannelsChanged.end())
    {
        //缓冲了这个通道设备?
        mapIt.value().set_point1(convertPointToString(point1).toStdString());
        mapIt.value().set_point2(convertPointToString(point2).toStdString());
        mapIt.value().set_position(convertPointToString(position).toStdString());
        mapIt.value().set_map_id(ui->toolbox->getMapObj()->getOperatorMap(true).get_id());
    }
    else
    {
        InviewChannel* channel = g_SppClient->getChannelById(devId);
        if(NULL != channel)
        {
            channel->set_point1(convertPointToString(point1).toStdString());
            channel->set_point2(convertPointToString(point2).toStdString());
            channel->set_position(convertPointToString(position).toStdString());
            channel->set_map_id(ui->toolbox->getMapObj()->getOperatorMap(true).get_id());
            m_objChannelsChanged.insert(devId, *channel);
        }
        else
        {
            qCritical() << "GisMapWidget::slotDevInfoChanged " << devId << " not found!";
        }
    }
}

void GisMapWidget::slotGisMapModeSelected()
{
    //为1即可
    int parentId = 1;

    //组织map code
    QString mapCode = organizeMapCode(parentId);
    m_objCurMapToAdd.set_local_map_name(QObject::tr("TK_WorldMap").toStdString());
    m_objCurMapToAdd.set_local_map_code(mapCode.toStdString());
    m_objCurMapToAdd.set_local_map_note("this is gis map");
    m_objCurMapToAdd.set_parent_map_id(parentId);
    m_objCurMapToAdd.set_local_map_type(MSG_MAP_TYPE_NODE);
    if(!addMapInfoToDB(m_objCurMapToAdd))
    {
        SNMessageBoxFlowDownWidget::showMessage(g_SPPClientMapManager->getLastErrorString(),this,SNMessageBoxFlowDownWidget::MessageError);
    }
    else
    {
        m_pAddFirstMapWidget->hide();
    }
}

void GisMapWidget::slotMapSelectFinished(QString mapName, QString mapPath)
{
    //先保存下地图上变动的信息
    saveMapInfoChanged();

    //还没有添加过地图? 根地图的parentId设置为0
    int parentId = m_pMaps->empty() ? 1 : ui->toolbox->getMapObj()->getOperatorMap().get_id();

    //判断选中的文件是否存在
    QFileInfo fileInfo(mapPath);
    QString fileSuffix = fileInfo.suffix();

    QImage image;
    if(!image.load(mapPath))
    {
        //加载图片失败
        qCritical() << "GisMapWidget::slotMapSelectFinished() Load " << mapPath << "failed!";
        SNMessageBoxFlowDownWidget::showMessage(
                    QObject::tr("TK_ImageLoadFaile"),
                    this,
                    SNMessageBoxFlowDownWidget::MessageError);
        return;
    }

    //组织map code
    QString mapCode = organizeMapCode(parentId);
    QString newFileNameFormat = "%1/%2.%3";
    QString newFileName = newFileNameFormat.arg(QDir::tempPath()).arg(mapCode).arg(fileSuffix);

    //如果临时文件存在，先删除
    QFile::remove(newFileName);

    //保存到临时文件夹，并且以mapcode进行重新命名,主要为解决中文图片会上传失败的问题
    QImage newImage = image.scaled(DEFAULT_MAP_WIDTH, DEFAULT_MAP_HEIGHT, Qt::KeepAspectRatio);
    newImage.save(newFileName);

    QFileInfo newImageFile(newFileName);
    if(!newImageFile.exists())
    {
        //保存到临时文件夹中的文件不存在？
        qCritical() << newFileName << " not exist!";
        SNMessageBoxFlowDownWidget::showMessage(
                    QObject::tr("TK_ImageLoadFaile"),
                    this,
                    SNMessageBoxFlowDownWidget::MessageError);
        return ;
    }

	m_objCurMapToAdd.set_local_map_name(mapName.toStdString());
	m_objCurMapToAdd.set_local_map_code(organizeMapCode(parentId).toStdString());
	m_objCurMapToAdd.set_local_map_width(image.width());
	m_objCurMapToAdd.set_local_map_height(image.height());
	m_objCurMapToAdd.set_local_map_note("");
	m_objCurMapToAdd.set_local_map_url(newImageFile.fileName().toStdString());
	m_objCurMapToAdd.set_parent_map_id(parentId);
    m_objCurMapToAdd.set_local_map_type(MSG_MAP_TYPE_CUSTOM);

    if(!g_SPPClientMapManager->uploadMap(newFileName,MapUtil::getRemotRelativeMapPath(QString::fromStdString(newImageFile.fileName().toStdString()))))
    {
		g_SPPClientMapManager->uploadFinished();
        SNMessageBoxFlowDownWidget::showMessage(
                    g_SPPClientMapManager->getLastErrorString(),
                    this,
                    SNMessageBoxFlowDownWidget::MessageError);
        return;
    }

    //开启定时器,查询上传进度
    m_nUploadTimeId = startTimer(100);
}

void GisMapWidget::slotMapSelected(int mapId)
{
    //保存之前参数
    InviewLocalMap selectMap;
    InviewLocalMap lastMap;
    ui->toolbox->getMapObj()->getCurMapAndLastMap(selectMap,lastMap);
    if(selectMap.get_local_map_code() != RootMapCode.toStdString() && lastMap.get_local_map_code() == RootMapCode.toStdString())
    {
        ui->widgetMap->saveLastViewParam();
    }
    m_pToolWidget->setMode(selectMap.get_local_map_type() == MSG_MAP_TYPE_CUSTOM ? LocalMap : GisMap);

    QPointF gisNodePoint;
    int zoomLevel;
    bool bGisNode = false;
    InviewLocalMap tempMap = ui->toolbox->getMapObj()->getOperatorMap(true);
    if(mapId != tempMap.get_id())
    {
        //已被重定向
        InviewLocalMap nodeMap;
        g_SPPClientMapManager->getMapById(mapId,nodeMap);
        bGisNode = true;
        gisNodePoint = QPointF(nodeMap.get_local_map_postion_x(),nodeMap.get_local_map_position_y());
        zoomLevel = nodeMap.get_local_map_zoomlevel();
        mapId = tempMap.get_id();
    }

    //地图变更清除划线
    startDrawLine(false);

    //如果有变化的信息没有保存，先保存一下
    saveMapInfoChanged();

    m_objSubMaps.clear();
    m_objChannels.clear();

    //根据mapId查询当前选中的地图信息和子地图信息
    for(int i = 0; i < m_pMaps->size(); i++)
    {
        InviewLocalMap& tmpMap = m_pMaps->at(i);
        if(tmpMap.get_parent_map_id() == mapId && tmpMap.get_local_map_type() == MSG_MAP_TYPE_CUSTOM)
        {
            //不显示节点
            m_objSubMaps.insert(tmpMap.get_id(), tmpMap);
        }
    }

    //根据地图id，查询地图上的通道
    g_SppClient->getChannelsByMapId(mapId, m_objChannels);

    //当前地图模式
    if(tempMap.get_local_map_type() == MSG_MAP_TYPE_NODE && tempMap.get_local_map_code() == RootMapCode.toStdString())
    {
        //gis逻辑
        changeMapInfo(GisMap);
        if(bGisNode)
        {
            ui->widgetMap->setCenterAndZoomlevel(gisNodePoint,zoomLevel);
        }
    }
    else
    {
        //检测本地地图是否存在
        if(MapUtil::checkLocalMapExist(QString::fromStdString(tempMap.get_local_map_url())))
        {
            //存在? 直接显示
            changeMapInfo();
        }
        else
        {
            //不存在？ 先从服务器上下载
            if(!m_pMapUtil->startDownloadMap(QString::fromStdString(tempMap.get_local_map_url())))
            {
                SNMessageBoxFlowDownWidget::showMessage(g_SPPClientMapManager->getLastErrorString(),this,SNMessageBoxFlowDownWidget::MessageError);
            }
        }
    }
    setCurMapFullName();
}

void GisMapWidget::changeMapInfo(MapMode mode)
{
    QList<GisMapButton> subMapButtons;
    QMap<int, InviewLocalMap>::iterator mapIt = m_objSubMaps.begin();
    while(mapIt != m_objSubMaps.end())
    {
        InviewLocalMap& tmpMap = mapIt.value();
        GisMapButton mapButton;
        mapButton.id = tmpMap.get_id();
        mapButton.buttonName = QString::fromStdString(tmpMap.get_local_map_name());
        mapButton.centerPoint = QPointF(tmpMap.get_local_map_postion_x(),tmpMap.get_local_map_position_y());
        subMapButtons.push_back(mapButton);
        ++mapIt;
    }

    QList<GisMapButton> ipcButtons;
    QMap<int, InviewChannel>::iterator channelIt = m_objChannels.begin();
    while(channelIt != m_objChannels.end())
    {
        InviewChannel& tmpChannel = channelIt.value();
        GisMapButton mapButton;
        mapButton.id = tmpChannel.get_id();
        mapButton.buttonName = QString::fromStdString(tmpChannel.get_channel_name());
        mapButton.centerPoint = convertStringToPointF(QString::fromStdString(tmpChannel.get_position()));
        mapButton.firstPoint = convertStringToPoint(QString::fromStdString(tmpChannel.get_point1()));
        mapButton.secondPoint = convertStringToPoint(QString::fromStdString(tmpChannel.get_point2()));
        ipcButtons.push_back(mapButton);
        ++channelIt;
    }

    if(mode == LocalMap)
    {
        //修改地图中显示的内容
        ui->widgetMap->setMapInfo(MapUtil::getLocalAbsoluteMapPath(QString::fromStdString(ui->toolbox->getMapObj()->getOperatorMap().get_local_map_url())),subMapButtons,ipcButtons);
    }
    else
    {
        ui->widgetMap->setGisMapInfo(subMapButtons,ipcButtons);
    }

    //根据当前按钮状态，调整地图上Item的控制方式
    switchMapState(m_mCurMapState);
}

void GisMapWidget::uploadStateChanged()
{
    TransState transState = g_SPPClientMapManager->getUploadState();
    qDebug() << "GisMapWidget::uploadStateChanged() trans state changed"
             << "m_nFileTransState " << transState.m_nFileTransState
             << "m_nTransSize " << transState.m_nTransSize
             << "m_nFileSize " << transState.m_nFileSize
             << "m_objFileName " << transState.m_objFileName;

    if(transState.m_nFileTransState == SPPFileTransferState_Opening
       || transState.m_nFileTransState == SPPFileTransferState_Opened
       || transState.m_nFileTransState == SPPFileTransferState_OnTrans)
    {
        //传输中
    }
    else if(transState.m_nFileTransState == SPPFileTransferState_Closed
            || transState.m_nFileTransState == SPPFileTransferState_Closing)
    {
        //传输结束,把地图从临时目录拷贝到 maps目录下
        QFileInfo fileInfo(transState.m_objFileName);
        QImage image;
        image.load(transState.m_objFileName);

        QString newFileNameFormat("%1/%2");
        QString newFileName = newFileNameFormat.arg(m_mapLocalDir)
                .arg(fileInfo.fileName());
        image.save(newFileName);
        killTimer(m_nUploadTimeId);

        if(addMapInfoToDB(m_objCurMapToAdd))
        {
            m_pAddFirstMapWidget->isVisible() ? m_pAddFirstMapWidget->hide() : m_pAddFirstMapWidget->hide();

            g_SPPClientMapManager->uploadFinished();

            if(!ui->pushButtonDelete->isVisible())
            {
                //添加地图成功,如果编辑删除按钮已隐藏就显示(首次添加地图)
                ui->pushButtonDelete->show();
                ui->pushButtonEdit->show();
                ui->pushButtonAdd->show();
            }
            SNMessageBoxFlowDownWidget::showMessage(QObject::tr("TK_AddSuccess"), this, SNMessageBoxFlowDownWidget::MessageInfo);
        }
    }
    else
    {
        //传输出错
        killTimer(m_nUploadTimeId);
        SNMessageBoxFlowDownWidget::showMessage(g_SPPClientMapManager->getLastErrorString(),this,SNMessageBoxFlowDownWidget::MessageError);
		g_SPPClientMapManager->uploadFinished();
    }
}

bool GisMapWidget::addMapInfoToDB(InviewLocalMap& map)
{
    //添加地图信息到数据库
    if(!g_SPPClientMapManager->addMap(map))
    {
        qDebug() << "slotMapSelectFinished failed "<< g_SPPClientMapManager->getLastErrorString();
        SNMessageBoxFlowDownWidget::showMessage(g_SPPClientMapManager->getLastErrorString(),this,SNMessageBoxFlowDownWidget::MessageError);
        return false;
    }
    else
    {
        //添加成功，把当前地图修改为新添加的地图
        ui->toolbox->getMapObj()->setCurrentMap(QString::fromStdString(map.get_local_map_code()));
    }
    return true;
}

void GisMapWidget::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_nUploadTimeId)
    {
        uploadStateChanged();
    }
}

QString GisMapWidget::organizeMapCode(int parentId)
{
    InviewLocalMap* parentMapInfo = NULL; //需要添加的地图的父地图
    std::vector<InviewLocalMap> buddyMaps; //和需要添加的地图同级的地图
    if(m_pMaps->empty())
    {
        //根001
        return QString(RootMapCode);
    }
    else
    {
        for(int i=0; i<m_pMaps->size(); i++)
        {
            if(m_pMaps->at(i).get_id() == parentId)
            {
                parentMapInfo = &(m_pMaps->at(i));
            }
            if(m_pMaps->at(i).get_parent_map_id() == parentId)
            {
                buddyMaps.push_back(m_pMaps->at(i));
            }
        }
    }

    if(NULL == parentMapInfo)
    {
        //没有父地图？ 根地图？ 有没有默认根地图？ 后面处理
        qDebug() << "GisMapWidget::organizeMapCode miss parent map";
        return QString();
    }

    int subCodeNum = 1;
    if (buddyMaps.empty())
    {
        //父地图没有任何子地图
        subCodeNum = 1;
    }
    else
    {
        //父地图有任何子地图
        //在2到999之前寻找一个没有被使用的code. 全被使用？ 存在999个子地图够？ 不可能吧？
        for(subCodeNum = 2; subCodeNum <= 999; subCodeNum++)
        {
            bool subCodeNumExist = false;
            for (int i = 0; i < buddyMaps.size(); i++)
            {
                //计算新增Map的code最后三位
                InviewLocalMap& tmpMap = buddyMaps.at(i);
                QString tmpMapCode = QString::fromStdString(tmpMap.get_local_map_code());
                int tmpSubCodeNum = tmpMapCode.right(3).toInt();
                if (tmpSubCodeNum == subCodeNum)
                {
                    subCodeNumExist = true;
                    break;
                }
            }
            if (!subCodeNumExist)
            {
                break;
            }
        }
    }
	return QString("%1%2").arg(parentMapInfo->get_local_map_code().c_str()).arg(subCodeNum, 3, 10, QChar('0'));
}

QPoint GisMapWidget::convertStringToPoint(const QString& str)
{
    //string format x,y
    QStringList pointString = str.split(",");
    if(pointString.size() < 2)
    {
        return QPoint(0,0);
    }
    else
    {
        return QPoint(pointString[0].toInt(),pointString[1].toInt());
    }
}

QPointF GisMapWidget::convertStringToPointF(const QString& str)
{
    //string format x,y
    QStringList pointString = str.split(",");
    if(pointString.size() < 2)
    {
        return QPointF(0,0);
    }
    else
    {
        return QPointF(pointString[0].toDouble(),pointString[1].toDouble());
    }
}

QString GisMapWidget::convertPointToString(QPoint& point)
{
    QString pointStr;
    return pointStr.append(QString::number(point.x())).append(",").append(QString::number(point.y()));
}

QString GisMapWidget::convertPointToString(QPointF& point)
{
    QString pointStr;
    return pointStr.append(QString::number(point.x(),'f',6)).append(",").append(QString::number(point.y(),'f',6));
}

void GisMapWidget::slotSubMapButtonClicked(int subMapId)
{
	//进入子地图
	InviewLocalMap subMap;
	if (g_SPPClientMapManager->getMapById(subMapId, subMap))
	{
        ui->toolbox->getMapObj()->setCurrentMap(QString::fromStdString(subMap.get_local_map_code()));

        //进入子地图清除划线
        startDrawLine(false);
	}
	else
	{
		SNMessageBoxFlowDownWidget::showMessage(
            QObject::tr("TK_SubMapNotFound"),
			this,
			SNMessageBoxFlowDownWidget::MessageError);
	}
}

void GisMapWidget::slotIpcButtonClicked(int ipcId)
{
    //单窗口模式下弹框展示视频
    if(LAYOUT_MODEL_1 == m_nCurLayoutMode)
    {
        if(NULL != m_pObjAlarmVideoWidget)
        {
            QString strUid, strDeviceName;
            InviewChannel* pChannel = g_SppClient->getChannelById(ipcId);
            if(NULL != pChannel)
            {
                strUid = QString::fromStdString(pChannel->get_channel_uid());
                strDeviceName = QString::fromStdString(pChannel->get_channel_name());
            }
            m_pObjAlarmVideoWidget->openAlarmVideoDialog(OperatorTypePlayreal, strUid, QDateTime::currentDateTime().toString(DATETIME_FORMATE_YMD), strDeviceName);
            m_pObjAlarmVideoWidget->setVisible(true);
        }
    }
    else
    {
        QVector<int> vecChannelId;
        vecChannelId.append(ipcId);
        emit devPlay(INVIEW_VIDEO_STREAM_SUB, vecChannelId);
    }

    //停止报警闪烁
    ui->widgetMap->stopAlarm(ipcId);
}

void GisMapWidget::slotConfirmDeleteMapMain()
{
    //主界面上面的删除按钮
    if(m_pMaps->empty())
    {
        SNMessageBoxFlowDownWidget::showMessage(QObject::tr("TK_NoMapToDelete"),this,SNMessageBoxFlowDownWidget::MessageError);
        return;
    }

    if(havaSubMap(ui->toolbox->getMapObj()->getOperatorMap().get_id()))
    {
        SNMessageBoxFlowDownWidget::showMessage(QObject::tr("TK_HaveSubMap"),this,SNMessageBoxFlowDownWidget::MessageError);
        return;
    }

    InviewIDList ids;
    ids.push_back(ui->toolbox->getMapObj()->getOperatorMap().get_id());
    if (!g_SPPClientMapManager->deleteMap(ids))
    {
        SNMessageBoxFlowDownWidget::showMessage(QObject::tr("TK_ImageDeleteFaile"),this,SNMessageBoxFlowDownWidget::MessageError);
    }

    closeVideoByMapId(ui->toolbox->getMapObj()->getOperatorMap(true).get_id());

    ui->toolbox->getMapObj()->setRootAsCurrentMap();

    refreshMapInfo();

    if(m_pMaps->empty())
    {
        //已空，隐藏删除按钮
        ui->pushButtonDelete->hide();
    }
    SNMessageBoxFlowDownWidget::showMessage(QObject::tr("TK_DeleteSuccess"), this, SNMessageBoxFlowDownWidget::MessageInfo);
}

void GisMapWidget::on_pushButtonDelete_clicked()
{
    m_pDelMapDlgMain->showDialog(getTitle(), QObject::tr("TK_MapDeleteConfim"), SNMessageDialog::MessageTypeRequest);
}

void GisMapWidget::on_pushButtonEdit_clicked()
{
    switch (m_mCurMapState)
    {

    //当前是预览状态，点击进入编辑状态
    case MapStatePreview:
        switchMapState(MapStateEdit);
        break;

    //当前是编辑状态，点击进入预览状态
    case MapStateEdit:
        saveMapInfoChanged();
        SNMessageBoxFlowDownWidget::showMessage(QObject::tr("TK_SaveSuccess"), this, SNMessageBoxFlowDownWidget::MessageInfo);
        switchMapState(MapStatePreview);

        if(m_pMaps->empty() && g_SppClient->getCurUserId() == 1)
        {
            //如果切换到预览后地图空了,同时是超级管理员,再次切换到编辑状态
            ui->pushButtonDelete->hide();
            ui->pushButtonEdit->hide();
            switchMapState(MapStateEdit);
        }
        break;

    //默认预览状态
    default:
        switchMapState(MapStatePreview);
        break;
    }
}

bool GisMapWidget::saveMapInfoChanged()
{
    SNLoaddingWidget loaddingWidget(this);
    loaddingWidget.start();

    QMap<int, InviewLocalMap>::iterator mapIt = m_objSubMapsChanged.begin();
    while(mapIt != m_objSubMapsChanged.end())
    { 
        if(g_SPPClientMapManager->updateMap(mapIt.value()))
        {
            //更新地图信息失败
        }
        ++mapIt;
    }

    //地图上的通道信息, 点击保存按钮时统一保存
    QMap<int, InviewChannel>::iterator channelIt = m_objChannelsChanged.begin();
    while(channelIt != m_objChannelsChanged.end())
    {
        InviewChannel& channel = channelIt.value();
        InviewModifyChannelRequestMessage req;
        req.setID(channel.get_id());
        req.setMapID(channel.get_map_id());
        req.setName(channel.get_channel_name());
        req.setPoint1(channel.get_point1());
        req.setPoint2(channel.get_point2());
        req.setPosition(channel.get_position());
        req.setSite(channel.get_site());
        if(g_SPPClientDevManager->updateChannel(req) != 0)
        {
            //更新设备信息失败!
        }
        ++channelIt;
    }

    //添加节点地图
    if(m_objGisMapNodeToAdd.get_parent_map_id() != DM_NO_USE_VALUE && m_pGisMapNodeUnitWidget)
    {
        QPointF point;
        int zoomLevel;
        m_pGisMapNodeUnitWidget->getPointAndZoomlevel(point,zoomLevel);
        m_objGisMapNodeToAdd.set_local_map_position_y(point.y());
        m_objGisMapNodeToAdd.set_local_map_postion_x(point.x());
        m_objGisMapNodeToAdd.set_local_map_zoomlevel(zoomLevel);
        if(!addMapInfoToDB(m_objGisMapNodeToAdd))
        {
            SNMessageBoxFlowDownWidget::showMessage(g_SPPClientMapManager->getLastErrorString(),this,SNMessageBoxFlowDownWidget::MessageError);
        }
        else
        {
            m_pGisMapNodeUnitWidget->openDialog(false);
            ui->widgetMap->setNoticeCenterChange(false);
        }
        m_objGisMapNodeToAdd.set_parent_map_id(DM_NO_USE_VALUE);
    }

    m_objSubMapsChanged.clear();
    m_objChannelsChanged.clear();
    return true;
}

void GisMapWidget::on_mapLayout1_clicked()
{
    m_nCurLayoutMode = LAYOUT_MODEL_1;
    adjustCurWindows();
    startDrawLine(false);
}

void GisMapWidget::on_mapLayout2_clicked()
{
    m_nCurLayoutMode = LAYOUT_MODEL_10;
    adjustCurWindows();
    startDrawLine(false);
}

void GisMapWidget::on_mapLayout3_clicked()
{
    m_nCurLayoutMode = LAYOUT_MODEL_4;
    adjustCurWindows();
    startDrawLine(false);
}

InviewLocalMap GisMapWidget::getMapById(int mapId)
{
    for(int i=0; i<m_pMaps->size(); i++)
    {
        InviewLocalMap& tmpMap = m_pMaps->at(i);
        if(tmpMap.get_id() == mapId)
        {
            return tmpMap;
        }
    }
	return InviewLocalMap();
}

QString GisMapWidget::getMapNameByCode(QString& mapCode)
{
    for(int i=0; i<m_pMaps->size(); i++)
    {
        InviewLocalMap& tmpMap = m_pMaps->at(i);
        if(tmpMap.get_local_map_code().compare(mapCode.toStdString()) == 0)
        {
            return QString::fromStdString(tmpMap.get_local_map_name());
        }
    }
    return QString();
}

void GisMapWidget::setCurMapFullName()
{
    if(ui->toolbox->getMapObj()->getOperatorMap().get_local_map_code().empty())
    {
        return;
    }

    QString mapCode = QString::fromStdString(ui->toolbox->getMapObj()->getOperatorMap().get_local_map_code());
    QStringList mapCodes;
    for(int len = mapCode.length(); len>0; len-=3)
    {
        mapCodes.push_back(mapCode.left(len));
    }

    QString fullPathName;
    while(!mapCodes.empty())
    {
        QString tmpCode = mapCodes.back();
        mapCodes.pop_back();
        QString tmpName = getMapNameByCode(tmpCode);
        if(tmpName.isEmpty())
        {
            break;
        }

        if(!fullPathName.isEmpty())
        {
            fullPathName.append(">>");
        }
        fullPathName.append(tmpName);
    }

    ui->labelMapName->setText(fullPathName);
}

void GisMapWidget::refreshMapInfo()
{
    if(m_pMaps->empty())
    {
        //树上没有地图了，没办法通过树节点的变化刷新地图了。
        //需要手动刷新下地图widget.
        QList<GisMapButton> emptyButtonList;
        ui->widgetMap->setMapInfo(QString(),emptyButtonList,emptyButtonList);
        m_pAddFirstMapWidget->show();
        ui->labelMapName->clear();
        ui->pushButtonAddGisMapNode->hide();
    }
}

void GisMapWidget::slotAlarmNotifyReceived(InviewAlarmMessage alarmMessage,int messageCount)
{
    Q_UNUSED(messageCount);

    if(this->isVisible())
    {
        QMap<int, InviewChannel>::iterator channelIt = m_objChannels.begin();
        while(channelIt != m_objChannels.end())
        {
            if(channelIt->get_channel_uid().compare(alarmMessage.get_uid()) == 0)
            {
                qDebug() << "GisMapWidget::slotAlarmNotifyReceived alarm_uid " << alarmMessage.get_uid().c_str();

                //摄像头监控区域闪烁
                ui->widgetMap->startAlarm(channelIt.value().get_id());

                if(m_pAlarmPopDlg)
                {
                    //推送到该类
                    m_pAlarmPopDlg->apppendAlarmMessage(alarmMessage);
                }

                break;
            }
            else
            {
               ++channelIt;
            }
        }
    }
}

void GisMapWidget::slotDownloadMapFinished(int state)
{
    if(state == 0)
    {
        changeMapInfo();
    }
    else
    {
        SNMessageBoxFlowDownWidget::showMessage(g_SPPClientMapManager->getLastErrorString(),this,SNMessageBoxFlowDownWidget::MessageError);
    }
}

bool GisMapWidget::verifyContents(QString strMapName)
{
    if(strMapName.isEmpty())
    {
        SNMessageBoxFlowDownWidget::showMessage(QObject::tr("TK_MapNameEmpty"),this,SNMessageBoxFlowDownWidget::MessageWarring);
        return false;
    }
    if(strMapName.indexOf(" ") != -1)
    {
        SNMessageBoxFlowDownWidget::showMessage(QObject::tr("TK_IllegalCharacters"),this,SNMessageBoxFlowDownWidget::MessageWarring);
        return false;
    }

    QString res;
    bool ret = APPTextValidator::isStringWithSpecialChar(strMapName, res);
    if(ret)
    {
        SNMessageBoxFlowDownWidget::showMessage(res,this,SNMessageBoxFlowDownWidget::MessageWarring);
        return false;
    }
    bool ret1= APPTextValidator::isStringLengthValid(strMapName, res);
    if(!ret1)
    {
        SNMessageBoxFlowDownWidget::showMessage(res,this,SNMessageBoxFlowDownWidget::MessageWarring);
        return false;
    }
    return true;
}

void GisMapWidget::closeVideoByMapId(int mapId)
{
    //关闭该地图上的设备关联的播放窗口 add by zuoshul20180905
    g_SppClient->getChannelsByMapId(mapId, m_objChannels);
    QMap<int, InviewChannel>::iterator iterator;
    for(iterator = m_objChannels.begin(); iterator != m_objChannels.end(); iterator++)
    {
        InviewChannel tmp = iterator.value();
        if(tmp.get_map_id() == mapId)
        {
            int channelId = tmp.get_id();
            QMap<int,QRect>::iterator  itor;
            for (itor = m_objWindowRectList.begin(); itor != m_objWindowRectList.end(); itor++)
            {
                int nWinId = itor.key();
                int nChannelId = m_pVideoSubWinFactory->getChannelId(nWinId);
                if(channelId == nChannelId)
                {
					PlayCtrlParam param;
					param.nWinId = nWinId;
                    m_pVideoSubWinFactory->slot_stopVideo(param);
                }
            }
        }
    }
}

void GisMapWidget::slot_signalDeviceInfoChanged()
{
    //清除无效设备重新过滤
    QMap<int, InviewChannel>::iterator iterator;
    QList<int> invalidChannelList;
    for(iterator = m_objChannels.begin(); iterator != m_objChannels.end(); iterator++)
    {
        int channelId = iterator.key();
        if(g_SppClient->getChannelById(channelId)->get_channel_uid().empty())
        {
            invalidChannelList.push_back(channelId);
        }
    }
    for(int i = 0;i < invalidChannelList.size();i++)
    {
        slotDelDevClicked(invalidChannelList.at(i));
    }
    ui->widgetMap->doResourceFilter();
}

void GisMapWidget::on_pushButtonAdd_clicked()
{
    //主界面上面添加按钮
    if(ui->toolbox->getMapObj()->getOperatorMap().get_local_map_type() == MSG_MAP_TYPE_NODE)
    {
        QPointF point(PointWuHan.x(),PointWuHan.y());
        slotAddButtonClicked(point);
    }
    else
    {
        QPointF point(0,0);
        slotAddButtonClicked(point);
    }
}

void GisMapWidget::slot_startPlay(const QVector<int>& p_vecChannelId)
{
    if(!p_vecChannelId.isEmpty())
    {
        emit devPlay(INVIEW_VIDEO_STREAM_SUB, p_vecChannelId);
    }
}

void GisMapWidget::slot_doubleClickedPlay(const int& p_nChannelId)
{
    emit doubleClickedPlay(INVIEW_VIDEO_STREAM_SUB, p_nChannelId);
}

void GisMapWidget::slot_changeStream(const int& p_nStreamId, const int& p_nChannelId)
{
    QVector<int> vecChannelId;
    vecChannelId.append(p_nChannelId);
    emit devPlay(p_nStreamId, vecChannelId);
}

//设备与播放窗口连线相关功能代码
void GisMapWidget::slot_winSelected(const QString& p_strChannelUID)
{
    if (p_strChannelUID.isEmpty())
    {
        return;
    }
    InviewChannel* channel = g_SppClient->getChannelByUid(p_strChannelUID.toStdString());
    if(channel == NULL)
    {
        return;
    }

    //获取播放窗口坐标
    QRect rect;
    int winid = m_pVideoSubWinFactory->getSelectWinID();
    m_pVideoSubWinFactory->getSubWinRealPosition(winid,rect);
    QPoint point = pointConver(winid,rect);

    //获取设备坐标
    if(ui->widgetMap->getDevPosition(channel->get_id(),m_devPoint))
    {
        //开始连线
        m_winPoint = point;
        startDrawLine(true);
    }
    else
    {
        for(int i = 0; i < m_pMaps->size(); i++)
        {
            //遍历查找该播放窗口属于哪个地图
            InviewLocalMap& tmpMap = m_pMaps->at(i);
            int mapId = tmpMap.get_id();
            g_SppClient->getChannelsByMapId(mapId, m_objChannels);
            QMap<int, InviewChannel>::iterator it;
            it = m_objChannels.find(channel->get_id());
            if(it != m_objChannels.end())
            {
                //先进入该地图
                slotSubMapButtonClicked(mapId);

                if(ui->widgetMap->getDevPosition(channel->get_id(),m_devPoint))
                {
                    //开始连线
                    m_winPoint = point;
                    startDrawLine(true);
                }
                break;
            }
        }
    }
}

bool GisMapWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == m_lineWidget && event->type() == QEvent::Paint && m_bDrawLine)
    {
        QPainter painter(m_lineWidget);
        /*
        QPen pen;
        QColor color(255, 165, 79);
        color.setAlphaF(0.5);
        pen.setColor(color);
        pen.setWidth(10);
        pen.setStyle(Qt::SolidLine);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawLine(m_devPoint, m_winPoint);

        QColor colorWhilte(255, 255, 255);
        colorWhilte.setAlphaF(0.5);
        pen.setWidth(1);
        pen.setStyle(Qt::DotLine);
        pen.setColor(colorWhilte);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawLine(m_devPoint, m_winPoint);*/

        QLinearGradient linearGradient(m_devPoint, m_winPoint);
        linearGradient.setColorAt(0.0,QColor(255, 232, 80));
        linearGradient.setColorAt(1.0,QColor(255, 128, 58));
        QBrush brush(linearGradient);
        QPen pen;
        pen.setWidth(4);
        pen.setStyle(Qt::DotLine);
        pen.setBrush(brush);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawLine(m_devPoint, m_winPoint);

        /*
        QPen penRound;
        penRound.setWidth(4);
        penRound.setStyle(Qt::SolidLine);
        penRound.setColor(QColor(255, 232, 80));
        painter.setPen(penRound);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawEllipse(m_devPoint,20,20);*/

        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void GisMapWidget::startDrawLine(bool bStart)
{
    if(!bStart)
    {
        m_bDrawLine = false;
        if(m_lineWidget)
        {
            m_lineWidget->update();
        }
    }
    else
    {
        m_bDrawLine = true;
        if(m_lineWidget)
        {
            m_lineWidget->update();
        }
        m_pTimer->start(3000);
    }
}

void GisMapWidget::slotIpcButtonPress(int channelId)
{
    QPointF tmpPoint;
    if(!ui->widgetMap->getDevPosition(channelId,tmpPoint))
    {
        return;
    }

    QMap<int, QRect>::iterator itor;
    for (itor = m_objWindowRectList.begin(); itor != m_objWindowRectList.end(); itor++)
    {
        int nWinId = itor.key();
        int nChannelId = m_pVideoSubWinFactory->getChannelId(nWinId);
        if(channelId == nChannelId)
        {
            QRect rect;
            m_pVideoSubWinFactory->getSubWinRealPosition(nWinId,rect);

            m_winPoint = pointConver(nWinId,rect);
            m_devPoint = tmpPoint;

            //开始连线
            startDrawLine(true);
            break;
        }
    }
}

void GisMapWidget::slot_closeAll()
{
    startDrawLine(false);
}

void GisMapWidget::slot_close(const int& channelId)
{
    startDrawLine(false);
}

QPoint GisMapWidget::pointConver(int winid,QRect rect)
{
    QPoint point;
    point.setX(rect.x());
    point.setY(rect.y());

    if(m_nCurLayoutMode == LAYOUT_MODEL_10)
    {
        point.setX(rect.x());
        point.setY(rect.y() + rect.height() / 2);
    }
    else if(m_nCurLayoutMode == LAYOUT_MODEL_4)
    {
        if(winid == 2 || winid == 3)
        {
            point.setX(rect.x());
            point.setY(rect.y() + rect.height() / 2);
        }
        else if(winid == 4 || winid == 5)
        {
            point.setX(rect.x() + rect.width() / 2);
            point.setY(rect.y());
        }
    }
    return point;
}

void GisMapWidget::slotClearDrawLine()
{
    if(m_lineWidget)
    {
        m_pTimer->stop();
        m_bDrawLine = false;
        m_lineWidget->update();
    }
}

//记忆功能相关代码
int GisMapWidget::getCurLayoutMode()
{
    return (int)m_nCurLayoutMode;
}

QVector< QPair<int, int> > GisMapWidget::getCurrLayoutList()
{
    QVector< QPair<int, int> > channelWndList;
    QMap<int, QRect>::iterator itor;
    for (itor = m_objWindowRectList.begin(); itor != m_objWindowRectList.end(); itor++)
    {
        //自己组装
        int nWinId = itor.key();
        int nChannelId = m_pVideoSubWinFactory->getChannelId(nWinId);
        if(nChannelId != 0)
        {
            QPair<int, int> pair;
            pair.first = nChannelId;
            pair.second = nWinId;
            channelWndList.append(pair);
        }
    }
    return channelWndList;
}

void GisMapWidget::RecoveryPlay()
{
    //恢复上次程序退出时的播放
    PreviewConfigParam previewParam = g_BaseHelper->getMapConfigParam();
    if (!previewParam.strLayoutId.isEmpty())
    {
        int nLayoutMode = previewParam.strLayoutMode.toInt();
        QVector< QPair<int, int> > vecLayoutList = previewParam.vecLayoutList;
        if (0 <= nLayoutMode)
        {
            if(nLayoutMode == LAYOUT_MODEL_1)
            {
                on_mapLayout1_clicked();
            }
            else if(nLayoutMode == LAYOUT_MODEL_10)
            {
                on_mapLayout2_clicked();
            }
            else
            {
                on_mapLayout3_clicked();
            }
            m_pVideoSubWinFactory->slot_sendSelectedLayout(vecLayoutList,false);
        }
        //使用完下次就失效
        previewParam.clear();
        g_BaseHelper->setMapConfigParam(previewParam);
    }
}

void GisMapWidget::initMapTool()
{
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(-1, 1);
    shadow_effect->setColor(QColor(67, 67, 68));
    shadow_effect->setBlurRadius(8);

    m_pToolWidget= new ZoomToolWidget(ui->widgetMap);
    connect(m_pToolWidget, SIGNAL(zoomIn()), ui->widgetMap,  SLOT(slotZoomIn()));
    connect(m_pToolWidget, SIGNAL(zoomOut()), ui->widgetMap,  SLOT(slotZoomOut()));
    connect(ui->widgetMap,SIGNAL(scaled(double)),m_pToolWidget,SLOT(slotPercentChanged(double)));

    //top right
    m_pTopToolWidget = new FunctionToolWidget(ui->widgetMap);
    m_pTopToolWidget->setGraphicsEffect(shadow_effect);
    m_pTopToolWidget->setFixedSize(250,35);
    connect(m_pTopToolWidget, SIGNAL(menuClicked(FunctionToolWidget::ClickStatus)), this,  SLOT(slotToolMenuClicked(FunctionToolWidget::ClickStatus)));
    m_pResourcesSettingDlg = new ResourcesSettingDialog(ui->widgetMap);
    connect(m_pResourcesSettingDlg, SIGNAL(checkStateChanged(ResourcesSettingDialog::ResourcesFilter)), ui->widgetMap,  SLOT(slotResourceFilter(ResourcesSettingDialog::ResourcesFilter)));
    m_pResourcesSettingDlg->pushFilterRules();
    m_pAreaSelectDlg = new AreaSelectDialog(ui->widgetMap);
    connect(ui->widgetMap, SIGNAL(signalOpenSelectDialog(QList<AreaSelectUnitWidget::Param>)), m_pAreaSelectDlg,  SLOT(slotOpenDialog(QList<AreaSelectUnitWidget::Param>)));
    connect(m_pAreaSelectDlg, SIGNAL(startLiveview(QList<int>)), this,  SLOT(slotStartLiveview(QList<int>)));
    connect(m_pAreaSelectDlg, SIGNAL(recoveryDrag()), ui->widgetMap,  SLOT(slotRecoveryDrag()));
    connect(m_pAreaSelectDlg, SIGNAL(startAlarmSearch(QList<int>)), this,  SLOT(slotStartAlarmSearch(QList<int>)));
    m_pMapSearchDlg = new MapSearchDialog(ui->widgetMap);

    //top left
    m_pAlarmPopDlg = new AlarmPopDialog(ui->widgetMap);
    m_pAlarmBtn = new QPushButton(ui->widgetMap);
    m_pAlarmBtn->setIcon(QIcon(":images/default/map/map_alarm.png"));
    m_pAlarmBtn->setStyleSheet("background-color:transparent;border:0px");
    m_pAlarmBtn->setIconSize(QSize(36,36));
    m_pAlarmBtn->setFixedSize(36,36);
    connect(m_pAlarmBtn, SIGNAL(clicked()), this,  SLOT(slotAlarmDlgClicked()));
    connect(m_pResourcesSettingDlg, SIGNAL(checkStateChanged(ResourcesSettingDialog::ResourcesFilter)), this,  SLOT(slotResourceFilter(ResourcesSettingDialog::ResourcesFilter)));

    DevStateChangeSpecialPro* pro = new DevStateChangeSpecialPro(this);
    connect(pro, SIGNAL(signalDeviceInfoChanged()),this, SLOT(slot_signalDeviceInfoChanged()));
}

void GisMapWidget::moveToolWidget()
{
    m_pToolWidget->move(ui->widgetMap->width() - m_pToolWidget->width() - 3,ui->widgetMap->height() - m_pToolWidget->height() - 3);
    m_pTopToolWidget->move(ui->widgetMap->width() - m_pTopToolWidget->width() - 3,3);
    m_pAlarmBtn->move(3,3);
    m_pAddFirstMapWidget->move((ui->widgetMap->width() - m_pAddFirstMapWidget->width()) / 2,(ui->widgetMap->height() - m_pAddFirstMapWidget->height()) / 2);
    if(m_pGisMapNodeUnitWidget)
    {
        m_pGisMapNodeUnitWidget->move((ui->widgetMap->width() - m_pGisMapNodeUnitWidget->width()) / 2,(ui->widgetMap->height() - m_pGisMapNodeUnitWidget->height()) / 2);
    }
}

void GisMapWidget::slotToolMenuClicked(FunctionToolWidget::ClickStatus status)
{
    QPoint point = QPoint(m_pTopToolWidget->pos().rx(),m_pTopToolWidget->pos().ry() + m_pTopToolWidget->height());
    if(status == FunctionToolWidget::ClickStatus::Select_Clicked)
    {
        if(m_pAreaSelectDlg->isHidden())
        {
            ui->widgetMap->doSelectArea();
            m_pResourcesSettingDlg->openDialog(false);
            m_pMapSearchDlg->openDialog(false);
        }
        else
        {
            m_pAreaSelectDlg->closeDialog();
        }
    }
    else if(status == FunctionToolWidget::ClickStatus::Source_Clicked)
    {
        m_pResourcesSettingDlg->openDialog(m_pResourcesSettingDlg->isHidden() ? true : false);
        m_pResourcesSettingDlg->move(ui->widgetMap->width() - m_pResourcesSettingDlg->width() - 1,point.ry() + 1);
        m_pAreaSelectDlg->hide();
        m_pMapSearchDlg->openDialog(false);
    }
    else if(status == FunctionToolWidget::ClickStatus::Search_Clicked)
    {
        QPointF point;
        int zoomLevel;
        ui->widgetMap->getCenterAndZoomLevel(point,zoomLevel);
        m_pMapSearchDlg->openDialog(m_pMapSearchDlg->isHidden() ? true : false,point,zoomLevel);
        m_pMapSearchDlg->move(ui->widgetMap->width() - m_pMapSearchDlg->width() - 1,point.ry() + 1);
        m_pAreaSelectDlg->hide();
        m_pResourcesSettingDlg->openDialog(false);
    }
    else if(status == FunctionToolWidget::ClickStatus::FullScreen_Clicked)
    {
        if(!m_bFullScreen)
        {
            m_bFullScreen = true;
            ui->widgetMap->setWindowFlags (Qt::Window);
            ui->widgetMap->showFullScreen();
            moveToolWidget();
        }
        else
        {
            m_bFullScreen = false;
            ui->widgetMap->setWindowFlags (Qt::SubWindow);
            ui->widgetMap->showNormal();
            adjustCurWindows();
        }
    }
}

void GisMapWidget::slotStartAlarmSearch(QList<int> channelList)
{
    if(m_bFullScreen)
    {
        //先退出全屏
        slotToolMenuClicked(FunctionToolWidget::ClickStatus::FullScreen_Clicked);
    }
    ParamMap param;
    param.insert(QString("type"), QString::number(INVIEW_ALARM_TYPE_FLAG_CHANNEL));
    for(int i = 0;i < channelList.size();i++)
    {
        param.insert(QString::number(channelList.at(i)), QString::number(channelList.at(i)));
    }
    emit(gotoFunctionCard(FUNCTION_ARALMSEARCH, param));
}

void GisMapWidget::slotStartLiveview(QList<int> channelList)
{
    if(m_bFullScreen)
    {
        //先退出全屏
        slotToolMenuClicked(FunctionToolWidget::ClickStatus::FullScreen_Clicked);
    }
    ParamMap param;
    for(int i = 0;i < channelList.size();i++)
    {
        param.insert(QString::number(channelList.at(i)), QString::number(channelList.at(i)));
    }
    emit(gotoFunctionCard(FUNCTION_PREVIEW, param));
}

void GisMapWidget::slotAlarmDlgClicked()
{
    m_pAlarmPopDlg->move(m_pAlarmBtn->pos().x() + 2,m_pAlarmBtn->pos().y() + m_pAlarmBtn->height() + 1);
    m_pAlarmPopDlg->openDialog(m_pAlarmPopDlg->isHidden());
}

void GisMapWidget::slotResourceFilter(ResourcesSettingDialog::ResourcesFilter filter)
{
    if(!filter.mapAlarm)
    {
        m_pAlarmPopDlg->openDialog(false);
        m_pAlarmBtn->hide();
    }
    else
    {
        m_pAlarmBtn->show();
    }
}

void GisMapWidget::on_pushButtonAddGisMapNode_clicked()
{
    //添加之前必须先保存
    saveMapInfoChanged();

    if(m_pAddGisMapNodeDlg == NULL)
    {
        m_pAddGisMapNodeDlg = new GisAddMapNodeDialog(this);
        connect(m_pAddGisMapNodeDlg, SIGNAL(signalMapNodeSetFinished(QString,int)), this,  SLOT(slotMapNodeSetFinished(QString,int)));
    }
    ui->widgetMap->setNoticeCenterChange(true);
    m_pAddGisMapNodeDlg->openDialog();
}

void GisMapWidget::slotMapNodeSetFinished(QString name,int parentId)
{
    m_objGisMapNodeToAdd.set_local_map_name(name.toStdString());
    m_objGisMapNodeToAdd.set_local_map_code(organizeMapCode(parentId).toStdString());
    m_objGisMapNodeToAdd.set_local_map_note("this is node");
    m_objGisMapNodeToAdd.set_parent_map_id(parentId);
    m_objGisMapNodeToAdd.set_local_map_type(MSG_MAP_TYPE_NODE);

    if(m_pGisMapNodeUnitWidget == NULL)
    {
        m_pGisMapNodeUnitWidget = new GisMapNodeUnitWidget(ui->widgetMap);
        connect(ui->widgetMap, SIGNAL(centerPointChanged(QPointF,int)), m_pGisMapNodeUnitWidget,  SLOT(slotMapCenterPointChanged(QPointF,int)));
    }
    m_pGisMapNodeUnitWidget->move((ui->widgetMap->width() - m_pGisMapNodeUnitWidget->width()) / 2,(ui->widgetMap->height() - m_pGisMapNodeUnitWidget->height()) / 2);
    m_pGisMapNodeUnitWidget->openDialog(true);
}
