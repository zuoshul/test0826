#include <QDebug>
#include "gismaptreeview.h"
#include "gismapwidget.h"
#include "gismapgraphwidget.h"
#include "ui_gismaptreeview.h"
#include <QLineEdit>

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

GisMapTreeView::GisMapTreeView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GisMapTreeView),
    m_nCurMapId(DM_NO_USE_VALUE),
    m_lastMapId(DM_NO_USE_VALUE)
{
    ui->setupUi(this);
    initMapTree();
}

GisMapTreeView::~GisMapTreeView()
{
    delete ui;
}

void GisMapTreeView::setCurrentMap(const QString& mapCode)
{
    ui->treeView->setCurrentItemTo(mapCode);
}

void GisMapTreeView::setRootAsCurrentMap()
{
	ui->treeView->setRootAsCurrentItem();
}

void GisMapTreeView::treeItemClicked(const Node node)
{
    if(m_nCurMapId != node.id)
    {
        m_lastMapId = m_nCurMapId;
        m_nCurMapId = node.id;
        emit(signalMapSelected(node.id));
    }
}

InviewLocalMap GisMapTreeView::getOperatorMap(bool redirect)
{
    //redirect为true表示需要智能重定向
    InviewLocalMap map;
    g_SPPClientMapManager->getMapById(m_nCurMapId,map);
    QString name = QString::fromStdString(map.get_local_map_name());
    if(!redirect)
    {
        return map;
    }

    if(map.get_local_map_type() == MSG_MAP_TYPE_NODE && map.get_local_map_code() != RootMapCode.toStdString())
    {
        //满足重定向条件
        Node node;
        ui->treeView->getRootItemData(node);
        g_SPPClientMapManager->getMapById(node.id,map);
        qCritical() << "GisMapTreeView::getOperatorMap()"<<name<<"has redirected to"<<QString::fromStdString(map.get_local_map_name())<<endl;
    }
    return map;
}

void GisMapTreeView::getCurMapAndLastMap(InviewLocalMap &curMap,InviewLocalMap &lastMap)
{
    //不重定向
    g_SPPClientMapManager->getMapById(m_nCurMapId,curMap);

    g_SPPClientMapManager->getMapById(m_lastMapId,lastMap);
}

void GisMapTreeView::slotMapInfoChanged(InviewLocalMap map, InviewOperatorType operatorType)
{
    std::vector<InviewLocalMap> mapVec;
    mapVec.push_back(map);

    if(!mapVec.empty())
    {
        switch (operatorType)
        {
        case InviewOperatorAdd:
            addMapNodes(mapVec);
            break;

        case InviewOperatorDelete:
            deleteMapNodes(mapVec);
            break;

        case InviewOperatorUpdate:
            //ui->treeView->updateNode(vecNodes);
            break;

        default:
            break;
        }
    }

	QStringList orgaCodes;
	g_SppClient->getOrgaCodeList(orgaCodes);
	ui->treeView->expandTreeNodes(orgaCodes);
}

void GisMapTreeView::initMapTree()
{
    std::vector<InviewLocalMap>* maps = NULL;
    maps = g_SPPClientMapManager->listMaps();

    connect(g_SPPClientMapManager,
            SIGNAL(signalMapInfoChanged(InviewLocalMap, InviewOperatorType)),
            this,
            SLOT(slotMapInfoChanged(InviewLocalMap, InviewOperatorType)));

	if (NULL != maps)
	{
		addMapNodes(*maps);
	}

    //默认根节点被选中
    ui->treeView->setClickedPlay(true);
    ui->treeView->setDropFlag(true);
    ui->treeView->setSelectedRowsFlag(false);
	QStringList orgaCodes;
	g_SppClient->getOrgaCodeList(orgaCodes);
	ui->treeView->expandTreeNodes(orgaCodes);

    connect(ui->treeView,
            SIGNAL(treeItemClicked(const Node)),
            this,
            SLOT(treeItemClicked(const Node)));

    initActions();

    connect(ui->treeView, SIGNAL(nameChanged(QModelIndex,QString)),this,SLOT(slotNameChanged(QModelIndex,QString)));

    m_pDelMapDlg = new SNMessageDialog(this);
    connect(m_pDelMapDlg,SIGNAL(signalConfirmed()),this,SLOT(slotConfirmDeleteMap()));
}

void GisMapTreeView::addMapNodes(std::vector<InviewLocalMap>& maps)
{
    QVector<Node> mapNodes;
    g_SppClient->organizeMapNode(mapNodes, maps);
    ui->treeView->appendItemData(mapNodes);
}

void GisMapTreeView::deleteMapNodes(std::vector<InviewLocalMap>& maps)
{
    QVector<Node> mapNodes;
    g_SppClient->organizeMapNode(mapNodes, maps);
    ui->treeView->removeNode(mapNodes);
}

#ifndef QT_NO_CONTEXTMENU
void GisMapTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    Node node;
    if(ui->treeView->getNodeAt(event->pos(), node))
    {
        //右键选择节点的ID
        qDebug() << "GisMapTreeView::contextMenuEvent " << node.id;
        if(m_nCurMapId != node.id)
        {
            m_lastMapId = m_nCurMapId;
            m_nCurMapId = node.id;

            //右键后选中该中地图，避免左键选中某节点再右键操作另一节点 zuoshul20180902
            emit(signalMapSelected(m_nCurMapId));
        }

        QMenu menu(this);
        menu.addAction(m_objAddMap);
        menu.addAction(m_objDeleteMap);
        menu.addAction(m_objRename);
        menu.exec(event->globalPos());
    }
    else
    {
        qDebug() << "GisMapTreeView::contextMenuEvent getNodeAt " << event->pos() << "faile!";
    }
}
#endif

void GisMapTreeView::initActions()
{
    m_objAddMap = new QAction(QObject::tr("TK_AddMap"), this);
    connect(m_objAddMap, SIGNAL(triggered()), this, SLOT(slotAddMap()));

    m_objDeleteMap = new QAction(QObject::tr("TK_DeleteMap"), this);
    connect(m_objDeleteMap, SIGNAL(triggered()), this, SLOT(slotDelteMap()));

    m_objRename = new QAction(QObject::tr("TK_RenameMap"), this);
    connect(m_objRename, SIGNAL(triggered()), this, SLOT(slotRenameMap()));
}

void GisMapTreeView::slotAddMap()
{
    QPointF pos(0,0);
    InviewLocalMap map;
    if(g_SPPClientMapManager->getMapById(m_nCurMapId,map))
    {
        if(map.get_local_map_type() == MSG_MAP_TYPE_NODE)
        {
            pos.setX(PointWuHan.x());
            pos.setY(PointWuHan.y());
        }
    }
    emit(signalAddMap(pos));
}

void GisMapTreeView::slotDelteMap()
{
    m_pDelMapDlg->showDialog(QObject::tr("TK_Map"), QObject::tr("TK_MapDeleteConfim"), SNMessageDialog::MessageTypeRequest);
}

void GisMapTreeView::slotConfirmDeleteMap()
{
    if (m_nCurMapId > 0)
    {
        emit(signalDelMap(m_nCurMapId));
    }
}

void GisMapTreeView::slotRenameMap()
{
    if (m_nCurMapId > 0)
    {
        QModelIndex index = ui->treeView->currentIndex();
        ui->treeView->edit(index);
    }
}

void GisMapTreeView::slotNameChanged(QModelIndex index,QString strNewName)
{
    QModelIndex curIndex = ui->treeView->currentIndex();
    if(index == curIndex)
    {
        emit signalRenameMap(m_nCurMapId,strNewName);
    }
}

void GisMapTreeView::switchMapState(MapState mapState)
{
    if(mapState == MapStateEdit)
    {
        //开启右键弹框
        setContextMenuPolicy(Qt::DefaultContextMenu);
    }
    else
    {
        //禁止右键弹框
        setContextMenuPolicy(Qt::NoContextMenu);
    }
}
