#include "gismaplivevideosubwin.h"
#include "src/base/appconfighelper.h"
#include "src/common/observermanage.h"
#include "src/base/customevent.h"
#include "src/base/basehelper.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

GisMapLiveVideoSubWin::GisMapLiveVideoSubWin(QWidget *parent)
    :LiveViewVideoSubWin(parent),
     m_pContextMenu(NULL),//右键菜单
     m_pCloseAllAction(NULL),
     m_pCloseAction(NULL), 
     m_pRestoreAction(NULL),
     m_pWindowRatioMenu(NULL),
     m_pDefultRatioAction(NULL),
     m_pStrentchAction(NULL),
     m_pHardwareAcceleration(NULL)
{
    initForm();
}

void GisMapLiveVideoSubWin::initForm()
{
    VideoSubWin::initForm();

    if(NULL == m_pContextMenu)
    {
        m_pContextMenu = new QMenu(this);
    }
}

void GisMapLiveVideoSubWin::retranslateUi()
{
    if (NULL != m_pWindowRatioMenu)
    {
        m_pWindowRatioMenu->setTitle(QObject::tr("TK_WindowRatio"));

        if (NULL != m_pDefultRatioAction)
        {
            m_pDefultRatioAction->setText(QObject::tr("TK_DefultRatio"));
        }

        if (NULL != m_pStrentchAction)
        {
            m_pStrentchAction->setText(QObject::tr("TK_Strentch"));
        }
    }

    if (NULL != m_pCloseAction)
    {
        m_pCloseAction->setText(QObject::tr("TK_Close"));
    }

    if (NULL != m_pCloseAllAction)
    {
        m_pCloseAllAction->setText(QObject::tr("TK_CloseAll"));
    }

	if (NULL != m_pRestoreAction)
	{
        m_pRestoreAction->setText(QObject::tr("TK_Restore"));
	}

    if(NULL != m_pHardwareAcceleration)
    {
        m_pHardwareAcceleration->setText(QObject::tr("TK_HardwareAcceleration"));
    }
}

void GisMapLiveVideoSubWin::slot_close()
{
    VideoSubWin::slot_close();
	PlayCtrlParam param;
	param.nWinId = getWinID();
    emit stop(param);

    //发出信号通知用户
    emit close(getWinID());
}

void GisMapLiveVideoSubWin::slot_closeAll()
{
    emit stopAllPlayerVideo(RealPlayer, true);

    //发出信号通知用户
    emit closeAll();
}

QMenu* GisMapLiveVideoSubWin::initContextMenu()
{
    if(NULL == m_pContextMenu)
    {
        return NULL;
    }

    if (NULL == m_pCloseAction)
    {
        m_pCloseAction = new QAction(QObject::tr("TK_Close"), this);
        if (NULL != m_pCloseAction)
        {
            m_pCloseAction->setText(QObject::tr("TK_Close"));
            m_pContextMenu->addAction(m_pCloseAction);

            disconnect(m_pCloseAction, SIGNAL(triggered(bool)), this, SLOT(slot_close()));
            connect(m_pCloseAction, SIGNAL(triggered(bool)), this, SLOT(slot_close()));
        }
    }

    if (NULL == m_pCloseAllAction)
    {
        m_pCloseAllAction = new QAction(tr("TK_CloseAll"), this);
        if (NULL != m_pCloseAllAction)
        {
            m_pCloseAllAction->setText(QObject::tr("TK_CloseAll"));
            m_pContextMenu->addAction(m_pCloseAllAction);

            disconnect(m_pCloseAllAction, SIGNAL(triggered(bool)), this, SLOT(slot_closeAll()));
            connect(m_pCloseAllAction, SIGNAL(triggered(bool)), this, SLOT(slot_closeAll()));
        }

        m_pContextMenu->addSeparator();
    }

    int playStatus = getPlayStatus();

    //读取配置文件中视频比例参数
    AppConfigLiveStream objConfigLiveStream = g_appConfigHelper->getLiveStreamConfig();
    if(NULL == m_pWindowRatioMenu)
    {
        m_pWindowRatioMenu = m_pContextMenu->addMenu(QObject::tr("TK_WindowRatio"));
        if(NULL != m_pWindowRatioMenu)
        {
            QActionGroup* pWindowRatioActionGroup = new QActionGroup(this);
            if(NULL == pWindowRatioActionGroup)
            {
                return NULL;
            }

            pWindowRatioActionGroup->setExclusive(true);
            if (NULL == m_pDefultRatioAction)
            {
                m_pDefultRatioAction = new QAction(QObject::tr("TK_DefultRatio"), this);
                if (NULL != m_pDefultRatioAction)
                {
                    m_pDefultRatioAction->setCheckable(true);
                    m_pDefultRatioAction->setChecked(!objConfigLiveStream.FullScreenProportion);
                    m_pDefultRatioAction->setText(QObject::tr("TK_DefultRatio"));
                    m_pWindowRatioMenu->addAction(m_pDefultRatioAction);
                    pWindowRatioActionGroup->addAction(m_pDefultRatioAction);
                    disconnect(m_pDefultRatioAction, SIGNAL(triggered()), this, SLOT(slot_defultRatio()));
                    connect(m_pDefultRatioAction, SIGNAL(triggered()), this, SLOT(slot_defultRatio()));
                }
            }

            if (NULL == m_pStrentchAction)
            {
                //满屏
                m_pStrentchAction = new QAction(QObject::tr("TK_Strentch"), this);
                if (NULL != m_pStrentchAction)
                {
                    m_pStrentchAction->setCheckable(true);
                    m_pStrentchAction->setChecked(objConfigLiveStream.FullScreenProportion);
                    m_pStrentchAction->setText(QObject::tr("TK_Strentch"));
                    m_pWindowRatioMenu->addAction(m_pStrentchAction);
                    pWindowRatioActionGroup->addAction(m_pStrentchAction);
                    disconnect(m_pStrentchAction, SIGNAL(triggered(bool)), this, SLOT(slot_stretch()));
                    connect(m_pStrentchAction, SIGNAL(triggered(bool)), this, SLOT(slot_stretch()));
                }
            }
        }
    }

    m_pWindowRatioMenu->setEnabled(false);
    if(MEDIAPLAY_STATUS_NORMAL == playStatus)
    {
        m_pWindowRatioMenu->setEnabled(true);
    }

	if (NULL == m_pRestoreAction)
	{
        m_pRestoreAction = new QAction(QObject::tr("TK_Restore"), this);
		if (NULL != m_pRestoreAction)
		{
            m_pRestoreAction->setText(QObject::tr("TK_Restore"));
			m_pContextMenu->addAction(m_pRestoreAction);

			disconnect(m_pRestoreAction, SIGNAL(triggered(bool)), this, SIGNAL(restoreVideo()));
			connect(m_pRestoreAction, SIGNAL(triggered(bool)), this, SIGNAL(restoreVideo()));
		}
	}

    if(NULL != m_pRestoreAction)
    {
        if(m_bIsZoomed)
        {
            m_pRestoreAction->setEnabled(true);
        }
        else
        {
            m_pRestoreAction->setEnabled(false);
        }
    }

    if(NULL == m_pHardwareAcceleration)
    {
        m_pHardwareAcceleration = new QAction(QObject::tr("TK_HardwareAcceleration"), this);
        if(NULL != m_pHardwareAcceleration)
        {
            m_pHardwareAcceleration->setText(QObject::tr("TK_HardwareAcceleration"));
            m_pContextMenu->addAction(m_pHardwareAcceleration);
            disconnect(m_pHardwareAcceleration,SIGNAL(triggered(bool)), this, SLOT(slot_hardwareAcceleration()));
            connect(m_pHardwareAcceleration,SIGNAL(triggered(bool)), this, SLOT(slot_hardwareAcceleration()));
        }
    }

    if (NULL != m_pHardwareAcceleration)
    {
        m_pHardwareAcceleration->setCheckable(true);
        AppConfigLogin objConfigLogin = g_appConfigHelper->getLoginConfig();
        m_pHardwareAcceleration->setChecked(objConfigLogin.HardwareAcceleration);
        m_pHardwareAcceleration->setEnabled(false);
        if (MEDIAPLAY_STATUS_NORMAL == playStatus)
        {
            m_pHardwareAcceleration->setEnabled(true);
        }
    }

    return m_pContextMenu;
}
