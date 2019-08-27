#include "mapalarmvideowidget.h"
#include "ui_mapalarmvideowidget.h"
#include "common/InviewRecordDEFs.h"
#include "src/sppclient/sppclient.h"
#include "timeconvert.h"
#include "src/base/basehelper.h"
#include "src/common/snmessageboxflowdownwidget.h"
#include <QDesktopWidget>

#define MAX_SLIDER_RANGE 600 //报警录像预览最大为往后30秒
const QString g_strEndTime = "00:10:00";

MapAlarmVideoWidget::MapAlarmVideoWidget(QWidget *parent) :
    SNDialog(parent),
    ui(new Ui::MapAlarmVideoWidget)
  , m_bFirst(true)
  , m_bEnd(false)
  , m_nPlayState(MEDIAPLAY_STATUS_STOP)
  , m_nSpeed(INVIEW_PLAYBACK_SPEED_NORMAL)
  , m_nType(0)
  , m_nLocateSystemTimeMilliseconds(0)
  , m_ulCurTime(0)
  , m_ulBeginTime(0)
  , m_ulEndTime(0)
  , m_strChannelUid("")
  ,m_pSubWin(NULL)
  ,m_bTalk(false)
  , m_bShowErrorFlag(true)
  ,m_pWidgetType(Qt::Window)
  ,m_pAlarmVideoTimeDlg(NULL)
{
    ui->setupUi(this);
    initForm();
}

MapAlarmVideoWidget::~MapAlarmVideoWidget()
{
    delete ui;
}

void MapAlarmVideoWidget::initForm()
{
    //实时报警或报警检索只支持单窗口
    ui->alarmWidgetPlayer->setMaxWndCount(1);
    ui->alarmWidgetPlayer->setFunctionId(FUNCTION_RTALARM);
    ui->comboBox->clear();
    ui->comboBox->addItem(tr("TK_PlayReal"),OperatorTypePlayreal);
    ui->comboBox->addItem(tr("TK_PlayBack"),OperatorTypePlayback);
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTypeChanged(int)));

    m_pAlarmVideoTimeDlg = new MapAlarmVideoTimeWidget(this);
    connect(m_pAlarmVideoTimeDlg, SIGNAL(timeSelected(QDateTime,QDateTime)), this, SLOT(slotTimeSelected(QDateTime,QDateTime)));
    AppStyleHelper::updateWidgetStyle(ui->pushButtonDate, AppStyleHelper::Style_PushButtonLight);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonTalk, AppStyleHelper::Style_PushButtonLight);
    ui->comboBox->setStyleSheet("QComboBox{selection-background-color: #41A0FF;background-color:transparent;border-style: solid;border: 1px solid transparent;border-radius: 0px;padding: 5px;color: #cccccc;}");
}

void MapAlarmVideoWidget::retranslateUi()
{
    ui->pushButtonClose->setToolTip(QObject::tr("TK_Close"));
    ui->pushButtonDownloadVideo->setToolTip(QObject::tr("TK_Download"));
    ui->pushButtonPause->setToolTip(QObject::tr("TK_Pause"));
    ui->pushButtonPlayVideo->setToolTip(QObject::tr("TK_Play"));
    ui->pushButtonSpeedDown->setToolTip(QObject::tr("TK_SlowPlay"));
    ui->pushButtonSpeedUp->setToolTip(QObject::tr("TK_FastPlay"));

    if(ui->alarmWidgetPlayer->getFullScrFlag())
    {
        ui->pushButtonFullScreen->setToolTip(QObject::tr("TK_ExitFullScreen"));
        ui->pushButtonLiveFullScreen->setToolTip(QObject::tr("TK_ExitFullScreen"));
    }
    else
    {
        ui->pushButtonFullScreen->setToolTip(QObject::tr("TK_FullScreen"));
        ui->pushButtonLiveFullScreen->setToolTip(QObject::tr("TK_FullScreen"));
    }
}

void MapAlarmVideoWidget::resetParam()
{
    retranslateUi();
    connectSignals();
    resetSpeed();

    //初始化对讲状态
    m_bTalk = false;

    m_bFirst = true;
    m_bEnd = false;
    m_nLocateSystemTimeMilliseconds = 0;
    m_ulBeginTime = 0;
    m_ulCurTime = 0;
    m_ulEndTime = 0;
    ui->labelCurrTime->setText("00:00:00");
    ui->label->setText("/");
    ui->labelTotalTime->setText(g_strEndTime);
    slot_refreshCtrlState(MEDIAPLAY_STATUS_STOP);

    ui->horizontalSliderProgress->setValue(0);
    ui->horizontalSliderProgress->setSliderEnabled(false);
    ui->horizontalSliderProgress->setMouseMoveMagnify(true);

#ifdef QtAVPlayer
    ui->pushButtonFullScreen->setVisible(false);
    ui->pushButtonLiveFullScreen->setVisible(false);
#endif
    if(m_pWidgetType == Qt::Window)
    {
         showSNDialog();
    }
}

bool MapAlarmVideoWidget::openAlarmVideoDialog(const int& p_nType,const QString& p_strUid,const QString& p_strAlarmTime,const QString& p_strDeviceName,const bool& p_bDelayPlay)
{
    //设置参数
    if(!isVisible())
    {
        ui->comboBox->setCurrentIndex(0);
    }
    m_strChannelUid = p_strUid;
    m_nType = p_nType;
    setTitle(p_strDeviceName);

    InviewChannel* pChannel = g_SppClient->getChannelByUid(p_strUid.toStdString());
    if (NULL == pChannel || pChannel->get_id() <= 0)
    {
        this->close();
        return false;
    }

    int nChannelId = pChannel->get_id();
    if (OperatorTypePlayreal == p_nType)
    {
        setModel(OperatorTypePlayreal);
    }
    else
    {
        //播放录像
        setModel(OperatorTypePlayback);
    }

    resetParam();

    //界面刷新后要延时300ms，否则可能会看到背景图片上叠加正在连接的动画，
    //因为界面还未刷新完毕就开始播放了
    if(p_bDelayPlay)
    {
        QThread::msleep(300);
    }

    if (OperatorTypePlayreal == p_nType)
    {
         emit playWndVideo(1, nChannelId, OperatorTypePlayreal, QString(), QString(), QString());
    }
    else
    {
        //录像播放的时间范围为报警时间前后15S
        QDateTime beginTime = QDateTime::fromString(p_strAlarmTime, "yyyy-MM-dd HH:mm:ss");
        g_TimeConvertInstance->getLongTime(m_ulBeginTime, beginTime);
        //由于I帧开始播放，播放时间往往会延后，这里选择往前2秒开始播放，报警录像范围为报警点的前后15秒
        m_ulBeginTime -= 300;
        m_ulCurTime = m_ulBeginTime;
        m_ulEndTime = m_ulCurTime + 600;

        QDateTime endTime;
        g_TimeConvertInstance->getStructTime(m_ulBeginTime, beginTime);
        g_TimeConvertInstance->getStructTime(m_ulEndTime, endTime);
        QString strBeginTime = beginTime.toString("yyyy-MM-dd HH:mm:ss");
        QString strEndTime = endTime.toString("yyyy-MM-dd HH:mm:ss");
        m_pAlarmVideoTimeDlg->setTimeParam(beginTime,endTime);

        emit playWndVideo(1, nChannelId, OperatorTypePlayback, strBeginTime, strBeginTime, strEndTime);
    }
    activateWindow();
    return true;
}

void MapAlarmVideoWidget::setModel(SNOperatorType type)
{
    bool bReal = false;
    if(type == OperatorTypePlayreal)
    {
        bReal = true;
    }
    else if(type == OperatorTypePlayback)
    {
        bReal = false;
    }
    else
    {
        ui->widgetCtrl->setVisible(false);
        return;
    }
    ui->widgetCtrl->setVisible(true);
    ui->pushButtonTalk->setVisible(bReal);
    ui->pushButtonPlayVideo->setVisible(!bReal);
    ui->pushButtonPause->setVisible(!bReal);
    ui->pushButtonSpeedDown->setVisible(!bReal);
    ui->pushButtonSpeedUp->setVisible(!bReal);
    ui->pushButtonDownloadVideo->setVisible(!bReal);
    ui->pushButtonFullScreen->setVisible(!bReal);
    ui->pushButtonLiveFullScreen->setVisible(bReal);
    ui->label->setVisible(!bReal);
    ui->horizontalSliderProgress->setVisible(!bReal);
    ui->labelCurrTime->setVisible(!bReal);
    ui->labelSpeed->setVisible(!bReal);
    ui->labelTotalTime->setVisible(!bReal);
    //ui->pushButtonDate->setVisible(!bReal);
    ui->pushButtonDate->setVisible(false);
}

void MapAlarmVideoWidget::setTitleIcon(QString strStyle)
{
    ui->labelIcon->setStyleSheet(strStyle);
}

void MapAlarmVideoWidget::connectSignals()
{
    disconnect(ui->pushButtonClose, SIGNAL(clicked()), this, SLOT(slot_close()));
    disconnect(ui->pushButtonDownloadVideo, SIGNAL(clicked()), this, SLOT(slot_download()));
    disconnect(ui->pushButtonPause, SIGNAL(clicked()), this, SLOT(slot_pause()));
    disconnect(ui->pushButtonPlayVideo, SIGNAL(clicked()), this, SLOT(slot_play()));
    disconnect(ui->pushButtonFullScreen, SIGNAL(clicked()), this, SLOT(slot_fullScreen()));
    disconnect(ui->pushButtonLiveFullScreen, SIGNAL(clicked()), this, SLOT(slot_fullScreen()));
    disconnect(ui->pushButtonSpeedDown, SIGNAL(clicked()), this, SLOT(slot_speedDown()));
    disconnect(ui->pushButtonSpeedUp, SIGNAL(clicked()), this, SLOT(slot_speedUp()));
    disconnect(ui->horizontalSliderProgress, SIGNAL(sliderClicked(const float&)), this, SLOT(slot_getTime(const float&)));

    disconnect(ui->alarmWidgetPlayer, SIGNAL(sendMousePos(const int&)), this, SLOT(slot_sendMousePos(const int&)));
    disconnect(this, SIGNAL(speedPlay(const int&, const bool&)),
        ui->alarmWidgetPlayer, SIGNAL(speedPlay(const int&, const bool&)));
    disconnect(this, SIGNAL(locateTime(const int&, const unsigned long&, const unsigned long&, const unsigned long&)),
        ui->alarmWidgetPlayer, SIGNAL(locateTime(const int&, const unsigned long&, const unsigned long&, const unsigned long&)));
    disconnect(this, SIGNAL(pause(const bool&)), ui->alarmWidgetPlayer, SIGNAL(pause(const bool&)));
    disconnect(this, SIGNAL(continuePlay(const bool&)), ui->alarmWidgetPlayer, SIGNAL(continuePlay(const bool&)));
    disconnect(this, SIGNAL(playWndVideo(const int&, const int&, const int&,
        const QString&, const QString&, const QString&)),
        ui->alarmWidgetPlayer, SIGNAL(playWndVideo(const int&, const int&, const int&,
            const QString&, const QString&, const QString&)));
    disconnect(this, SIGNAL(stopAllPlayerVideo(const PlayerType&)),
        ui->alarmWidgetPlayer, SIGNAL(stopAllPlayerVideo(const PlayerType&)));
    disconnect(ui->alarmWidgetPlayer, SIGNAL(playbackTime(const unsigned long&, const unsigned long&)),
        this, SLOT(slot_playbackTime(const unsigned long&, const unsigned long&)));
    disconnect(this, SIGNAL(fullScreen(bool)), ui->alarmWidgetPlayer, SLOT(slot_setFullScreen(bool)));
    disconnect(ui->alarmWidgetPlayer, SIGNAL(hideWnd()), this, SLOT(slot_hideWnd()));
    disconnect(ui->alarmWidgetPlayer, SIGNAL(showWnd()), this, SLOT(slot_showWnd()));
    disconnect(ui->alarmWidgetPlayer, SIGNAL(refreshCtrlState(const int&)),
            this, SLOT(slot_refreshCtrlState(const int&)));
    disconnect(ui->alarmWidgetPlayer, SIGNAL(readFileFinish()), this, SLOT(slot_readFileFinish()));
    disconnect(ui->alarmWidgetPlayer, SIGNAL(happenError(const int&)), this, SLOT(slot_happenError(const int&)));
    disconnect(ui->alarmWidgetPlayer, SIGNAL(fullScreen()), this, SLOT(slot_fullScreen()));
	disconnect(ui->alarmWidgetPlayer, SIGNAL(lockscreen()), this, SIGNAL(lockscreen()));

    connect(ui->pushButtonClose, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(ui->pushButtonDownloadVideo, SIGNAL(clicked()), this, SLOT(slot_download()));
    connect(ui->pushButtonPause, SIGNAL(clicked()), this, SLOT(slot_pause()));
    connect(ui->pushButtonPlayVideo, SIGNAL(clicked()), this, SLOT(slot_play()));
    connect(ui->pushButtonFullScreen, SIGNAL(clicked()), this, SLOT(slot_fullScreen()));
    connect(ui->pushButtonLiveFullScreen, SIGNAL(clicked()), this, SLOT(slot_fullScreen()));
    connect(ui->pushButtonSpeedDown, SIGNAL(clicked()), this, SLOT(slot_speedDown()));
    connect(ui->pushButtonSpeedUp, SIGNAL(clicked()), this, SLOT(slot_speedUp()));
    connect(ui->horizontalSliderProgress, SIGNAL(sliderClicked(const float&)), this, SLOT(slot_getTime(const float&)));

	connect(ui->alarmWidgetPlayer, SIGNAL(lockscreen()), this, SIGNAL(lockscreen()));
    connect(ui->alarmWidgetPlayer, SIGNAL(fullScreen()), this, SLOT(slot_fullScreen()));
    connect(ui->alarmWidgetPlayer, SIGNAL(sendMousePos(const int&)), this, SLOT(slot_sendMousePos(const int&)));
    connect(this, SIGNAL(speedPlay(const int&, const bool&)),
            ui->alarmWidgetPlayer, SIGNAL(speedPlay(const int&, const bool&)));
    connect(this, SIGNAL(locateTime(const int&, const unsigned long&, const unsigned long&, const unsigned long&)),
            ui->alarmWidgetPlayer, SIGNAL(locateTime(const int&, const unsigned long&, const unsigned long&, const unsigned long&)));
    connect(this, SIGNAL(pause(const bool&)), ui->alarmWidgetPlayer, SIGNAL(pause(const bool&)));
    connect(this, SIGNAL(continuePlay(const bool&)), ui->alarmWidgetPlayer, SIGNAL(continuePlay(const bool&)));
    connect(this, SIGNAL(playWndVideo(const int&, const int&, const int&,
                                      const QString&, const QString&, const QString&)),
            ui->alarmWidgetPlayer, SIGNAL(playWndVideo(const int&, const int&, const int&,
                                   const QString&, const QString&, const QString&)));
    connect(this, SIGNAL(stopAllPlayerVideo(const PlayerType&)),
            ui->alarmWidgetPlayer, SIGNAL(stopAllPlayerVideo(const PlayerType&)));
    if(m_nType == OperatorTypePlayback)
    {
        connect(ui->alarmWidgetPlayer, SIGNAL(playbackTime(const unsigned long&, const unsigned long&)),
            this, SLOT(slot_playbackTime(const unsigned long&, const unsigned long&)));
    }
    connect(this, SIGNAL(fullScreen(bool)), ui->alarmWidgetPlayer, SLOT(slot_setFullScreen(bool)));
    connect(ui->alarmWidgetPlayer, SIGNAL(hideWnd()), this, SLOT(slot_hideWnd()));
    connect(ui->alarmWidgetPlayer, SIGNAL(showWnd()), this, SLOT(slot_showWnd()));
    connect(ui->alarmWidgetPlayer, SIGNAL(refreshCtrlState(const int&)),
                this, SLOT(slot_refreshCtrlState(const int&)));
    connect(ui->alarmWidgetPlayer, SIGNAL(readFileFinish()), this, SLOT(slot_readFileFinish()));
    connect(ui->alarmWidgetPlayer, SIGNAL(happenError(const int&)), this, SLOT(slot_happenError(const int&)));
    disconnect(ui->alarmWidgetPlayer, SIGNAL(ctrlTalkOpenSuccess(bool)), this, SLOT(ctrlTalkOpenSuccess(bool)));
    connect(ui->alarmWidgetPlayer, SIGNAL(ctrlTalkOpenSuccess(bool)), this, SLOT(ctrlTalkOpenSuccess(bool)));
}

void MapAlarmVideoWidget::setTitle(const QString& p_strTitle)
{
    ui->labelTitle->setText(p_strTitle);
}

void MapAlarmVideoWidget::slot_close()
{
    if(m_pSubWin && m_bTalk)
    {
        emit m_pSubWin->ctrlTalk(1,false);
    }
    if(OperatorTypePlayreal == m_nType)
    {
        emit stopAllPlayerVideo(RealPlayer);
    }
    else
    {
        emit stopAllPlayerVideo(RecordPlayer);
    }
    this->close();
}

void MapAlarmVideoWidget::slot_download()
{
    setTips(tr("TK_StartDownLoadTip"));
    QDateTime beginTime, endTime;
    g_TimeConvertInstance->getStructTime(m_ulBeginTime, beginTime);
    g_TimeConvertInstance->getStructTime(m_ulEndTime, endTime);
    QString strBeginTime = beginTime.toString("yyyy-MM-dd HH:mm:ss");
    QString strEndTime = endTime.toString("yyyy-MM-dd HH:mm:ss");
    emit download(m_strChannelUid, strBeginTime, strEndTime);
}

void MapAlarmVideoWidget::slot_pause()
{
    emit pause(true);
}

void MapAlarmVideoWidget::slot_play()
{
    if(MEDIAPLAY_STATUS_NORMAL == m_nPlayState)
    {
        qDebug() << "It is playing!";
        return;
    }
    else if(MEDIAPLAY_STATUS_STOP == m_nPlayState)
    {
        qDebug() << "now it is stopped, you can not use play button!";
        return;
    }
    else
    {
        resetSpeed();
        //续播
        emit continuePlay(true);
    }
}

void MapAlarmVideoWidget::slot_refreshCtrlState(const int& p_nState)
{
    if(OperatorTypePlayreal == m_nType)
    {
        return;
    }

    m_nPlayState = p_nState;
    if(MEDIAPLAY_STATUS_STOP == m_nPlayState)
    {
        ui->pushButtonPlayVideo->setEnabled(false);
        ui->pushButtonPause->setEnabled(false);
        ui->pushButtonDownloadVideo->setEnabled(false);
        ui->pushButtonSpeedUp->setEnabled(false);
        ui->pushButtonSpeedDown->setEnabled(false);

        ui->horizontalSliderProgress->setRange(0, MAX_SLIDER_RANGE);
        ui->horizontalSliderProgress->setValue(0);

        if(ui->horizontalSliderProgress->getSliderEnabled())
        {
            ui->horizontalSliderProgress->setSliderEnabled(false);
        }
    }
    else if(MEDIAPLAY_STATUS_NORMAL == m_nPlayState)
    {
        ui->pushButtonPlayVideo->setEnabled(true);
        ui->pushButtonPause->setEnabled(true);
        ui->pushButtonPlayVideo->setVisible(false);
        ui->pushButtonPause->setVisible(true);
        ui->pushButtonDownloadVideo->setEnabled(true);
        ui->pushButtonSpeedUp->setEnabled(true);
        ui->pushButtonSpeedDown->setEnabled(true);

        if(!ui->horizontalSliderProgress->getSliderEnabled())
        {
            ui->horizontalSliderProgress->setSliderEnabled(true);
        }
    }
    else if(MEDIAPLAY_STATUS_PAUSE == m_nPlayState)
    {
        ui->pushButtonPlayVideo->setEnabled(true);
        ui->pushButtonPause->setEnabled(true);
        ui->pushButtonPlayVideo->setVisible(true);
        ui->pushButtonPause->setVisible(false);
        ui->pushButtonDownloadVideo->setEnabled(false);
        ui->horizontalSliderProgress->setEnabled(true);
        ui->pushButtonSpeedUp->setEnabled(false);
        ui->pushButtonSpeedDown->setEnabled(false);
    }
    else if(MEDIAPLAY_STATUS_NEXT_FRAME == m_nPlayState)
    {
        ui->pushButtonPlayVideo->setEnabled(true);
        ui->pushButtonPause->setEnabled(true);
        ui->pushButtonPlayVideo->setVisible(true);
        ui->pushButtonPause->setVisible(false);
        ui->pushButtonDownloadVideo->setEnabled(false);
        ui->pushButtonSpeedUp->setEnabled(true);
        ui->pushButtonSpeedDown->setEnabled(true);

        if(!ui->horizontalSliderProgress->getSliderEnabled())
        {
            ui->horizontalSliderProgress->setSliderEnabled(true);
        }
    }
    else if(MEDIAPLAY_STATUS_RATE == m_nPlayState)
    {
        ui->pushButtonPlayVideo->setEnabled(true);
        ui->pushButtonPause->setEnabled(true);
        ui->pushButtonPlayVideo->setVisible(false);
        ui->pushButtonPause->setVisible(true);
        ui->pushButtonDownloadVideo->setEnabled(true);

        if(!ui->horizontalSliderProgress->getSliderEnabled())
        {
            ui->horizontalSliderProgress->setSliderEnabled(true);
        }
    }
}

bool MapAlarmVideoWidget::event(QEvent *event)
{
#ifndef QtAVPlayer
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (!this->isActiveWindow() || !this->isVisible())
        {
            return true;
        }

        QString qstrKey = g_BaseHelper->getHotKeyValue(ke);
        if (qstrKey.trimmed().isEmpty())
        {
            return true;
        }

        g_BaseHelper->getHotKey();

        //视频全屏播放
        if(qstrKey == g_BaseHelper->getFullScreenKey())
        {
            ke->accept();
            slot_fullScreen();
            return true;
        }
        //锁屏
        else if (qstrKey == g_BaseHelper->getLockKey())
        {
            emit lockscreen();
            return true;
        }
        //ESC退出全屏
        else if ( ke->key() == Qt::Key_Escape)
        {
            ke->accept();
            shortcutClose();
            return true;
        }
    }
#endif

   if(m_pWidgetType == Qt::Widget)
   {
       return true;
   }

   return QWidget::event(event);
}

void MapAlarmVideoWidget::shortcutClose()
{
    if(ui->alarmWidgetPlayer->getFullScrFlag())
    {
        slot_fullScreen();
    }
}

void MapAlarmVideoWidget::slot_fullScreen()
{
    bool bFullScreen = ui->alarmWidgetPlayer->getFullScrFlag();
    bFullScreen = !bFullScreen;
    ui->alarmWidgetPlayer->setFullScrFlag(bFullScreen);
    if(bFullScreen)
    {
        if(OperatorTypePlayback == m_nType)
        {
            ui->pushButtonFullScreen->setToolTip(QObject::tr("TK_ExitFullScreen"));
        }
        else if(OperatorTypePlayreal == m_nType)
        {
            ui->pushButtonLiveFullScreen->setToolTip(QObject::tr("TK_ExitFullScreen"));
        }
    }
    else
    {
        if(OperatorTypePlayback == m_nType)
        {
            ui->pushButtonFullScreen->setToolTip(QObject::tr("TK_FullScreen"));
        }
        else if(OperatorTypePlayreal == m_nType)
        {
            ui->pushButtonLiveFullScreen->setToolTip(QObject::tr("TK_FullScreen"));
        }
    }
    emit fullScreen(bFullScreen);
}

void MapAlarmVideoWidget::slot_hideWnd()
{
    ui->alarmWidgetPlayer->hide();
    setModel(OperatorTypeView);
}

void MapAlarmVideoWidget::slot_showWnd()
{
    ui->alarmWidgetPlayer->setWindowFlags(Qt::SubWindow);
    ui->alarmWidgetPlayer->show();

    ui->widgetCtrl->setWindowFlags(Qt::SubWindow);
    if(OperatorTypePlayback == m_nType)
    {
        setModel(OperatorTypePlayback);
    }
    else
    {
        setModel(OperatorTypePlayreal);
    }
}

void MapAlarmVideoWidget::slot_speedDown()
{
    if(!refreshSlowPlaySpeed())
    {
        return;
    }
    ui->labelSpeed->setText(getSpeedText(m_nSpeed));
    emit speedPlay(m_nSpeed, true);
}

void MapAlarmVideoWidget::slot_speedUp()
{
    if(!refreshFastPlaySpeed())
    {
        return;
    }
    ui->labelSpeed->setText(getSpeedText(m_nSpeed));
    emit speedPlay(m_nSpeed, true);
}

bool MapAlarmVideoWidget::refreshSlowPlaySpeed()
{
    if(!ui->pushButtonSpeedUp->isEnabled())
    {
        ui->pushButtonSpeedUp->setEnabled(true);
    }

    //已经达到最小极限
    if(INVIEW_PLAYBACK_SPEED_EIGHTH == m_nSpeed)
    {
        return false;
    }
    else
    {
        m_nSpeed--;
        if(INVIEW_PLAYBACK_SPEED_EIGHTH == m_nSpeed && ui->pushButtonSpeedDown->isEnabled())
        {
            ui->pushButtonSpeedDown->setEnabled(false);
        }
    }
    return true;
}

bool MapAlarmVideoWidget::refreshFastPlaySpeed()
{
    if(!ui->pushButtonSpeedDown->isEnabled())
    {
        ui->pushButtonSpeedDown->setEnabled(true);
    }

    //已经达到最大极限
    if(INVIEW_PLAYBACK_SPEED_8X == m_nSpeed)
    {
        return false;
    }
    else
    {
        m_nSpeed++;
        if(INVIEW_PLAYBACK_SPEED_8X == m_nSpeed && ui->pushButtonSpeedUp->isEnabled())
        {
            ui->pushButtonSpeedUp->setEnabled(false);
        }
    }
    return true;
}

QString MapAlarmVideoWidget::getSpeedText(const int& p_nSpeed)
{
    QString strSpeed = "";
    switch(p_nSpeed)
    {
    case INVIEW_PLAYBACK_SPEED_EIGHTH:
        strSpeed = "1/8x";
        break;
    case INVIEW_PLAYBACK_SPEED_QUARTER:
        strSpeed = "1/4x";
        break;
    case INVIEW_PLAYBACK_SPEED_HALF:
        strSpeed = "1/2x";
        break;
    case INVIEW_PLAYBACK_SPEED_2X:
        strSpeed = "2x";
        break;
    case INVIEW_PLAYBACK_SPEED_4X:
        strSpeed = "4x";
        break;
    case INVIEW_PLAYBACK_SPEED_8X:
        strSpeed = "8x";
        break;
    case INVIEW_PLAYBACK_SPEED_OPPOSITE_2X:
        strSpeed = "-2x";
        break;
    case INVIEW_PLAYBACK_SPEED_OPPOSITE_4X:
        strSpeed = "-4x";
        break;
    case INVIEW_PLAYBACK_SPEED_OPPOSITE_8X:
        strSpeed = "-8x";
        break;
    case INVIEW_PLAYBACK_SPEED_NORMAL:
    default:
        strSpeed = "1x";
        break;
    }
    return strSpeed;
}

void MapAlarmVideoWidget::resetSpeed()
{
    m_nSpeed = INVIEW_PLAYBACK_SPEED_NORMAL;
    ui->labelSpeed->setText("1x");
    ui->pushButtonSpeedDown->setEnabled(true);
    ui->pushButtonSpeedUp->setEnabled(true);
}

void MapAlarmVideoWidget::slot_sendMousePos(const int& p_nY)
{
    if (!ui->alarmWidgetPlayer->getFullScrFlag())
    {
        return;
    }

    int nHeight = 0;
    if(OperatorTypePlayreal == m_nType)
    {
        nHeight = ui->alarmWidgetPlayer->height() - ui->widgetCtrl->height();
        if (p_nY > nHeight)
        {
            if(!ui->widgetCtrl->isVisible())
            {
                QDesktopWidget * desktop = QApplication::desktop();
                //获取程序所在屏幕是第几个屏幕
                int current_screen = desktop->screenNumber(this);
                //获取程序所在屏幕的尺寸
                QRect rect = desktop->screenGeometry(current_screen);
                ui->widgetCtrl->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::Tool);
                if(current_screen == 0)//主屏
                {
                    ui->widgetCtrl->setGeometry(0, nHeight, ui->alarmWidgetPlayer->width(), ui->widgetCtrl->height());
                }
                else //辅屏
                {
                    ui->widgetCtrl->setGeometry(rect.width(), nHeight, rect.width(), ui->widgetCtrl->height());
                }
                ui->widgetCtrl->show();
            }
        }
        else
        {
            if(ui->widgetCtrl->isVisible())
            {
                ui->widgetCtrl->hide();
            }
        }
    }
    else
    {
        nHeight = ui->alarmWidgetPlayer->height() - ui->widgetCtrl->height();
        if (p_nY > nHeight)
        {
            if(!ui->widgetCtrl->isVisible())
            {
                QDesktopWidget * desktop = QApplication::desktop();
                //获取程序所在屏幕是第几个屏幕
                int current_screen = desktop->screenNumber(this);
                //获取程序所在屏幕的尺寸
                QRect rect = desktop->screenGeometry(current_screen);
                ui->widgetCtrl->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::Tool);
                if(current_screen == 0)
                {
                    //主屏
                    ui->widgetCtrl->setGeometry(0, nHeight, ui->alarmWidgetPlayer->width(), ui->widgetCtrl->height());
                }
                else
                {
                    //辅屏
                    ui->widgetCtrl->setGeometry(rect.width(), nHeight, rect.width(), ui->widgetCtrl->height());
                }
                ui->widgetCtrl->show();
            }
        }
        else
        {
            if(ui->widgetCtrl->isVisible())
            {
                ui->widgetCtrl->hide();
            }
        }
    }
}

void MapAlarmVideoWidget::slot_playbackTime(const unsigned long& p_nTime, const unsigned long& p_nSystem)
{
    QDateTime curTime;
    g_TimeConvertInstance->getStructTime(m_ulCurTime, curTime);
    QDateTime pTime;
    g_TimeConvertInstance->getStructTime(p_nTime, pTime);
    if(curTime.addSecs(60) <  pTime)
    {
        //预览切回放仍然会有垃圾数据过来，哪怕disconnected
        return;
    }

    //拖拽时间进度条情况下不刷新进度条时间
    if(!m_bEnd && (OperatorTypePlayback == m_nType) && (0 >= m_nLocateSystemTimeMilliseconds
                //用户操作定位后必须保证收到的时间在操作之后，也即是保证时间同步，
                //每个子窗口都已经保证时间同步，这里只要判断有定位操作后的时间过来
                || p_nSystem > m_nLocateSystemTimeMilliseconds))
    {
        unsigned long ulCurTime = 0;
        //用户未操作情况下,多个窗口时每个窗口都会发当前时间过来，取最大的时间为当前时间，防止时间往回跳动
        if (0 >= m_nLocateSystemTimeMilliseconds)
        {
            //由于I帧开始播放，所以实际播放的时间段可能会在请求的时间段外，相差几秒，这里以
            //收到的第一帧数据时间为起始时间，往后30S进行刷新
            if(m_bFirst)
            {
                m_bFirst = false;
                ulCurTime = m_ulBeginTime = p_nTime;
                m_ulEndTime = m_ulBeginTime + MAX_SLIDER_RANGE;
            }
            else
            {
                //获取的时间取大值
                if(m_ulCurTime >= p_nTime)
                {
                    return;
                }
                ulCurTime = MAX(m_ulCurTime, p_nTime);
            }
        }
        //用户操作情况下，选择传过来的时间作为当前时间
        else
        {
            ulCurTime = p_nTime;
        }

        m_nLocateSystemTimeMilliseconds = 0;
        m_ulCurTime = ulCurTime;

        unsigned long nDifference = m_ulCurTime - m_ulBeginTime;
        if(MAX_SLIDER_RANGE < nDifference)
        {
            nDifference = MAX_SLIDER_RANGE;
        }
        else if(0 > nDifference)
        {
            nDifference = 0;
        }
        ui->labelCurrTime->setText(second2TimeString(nDifference));
        ui->horizontalSliderProgress->setRange(0, MAX_SLIDER_RANGE);
        ui->horizontalSliderProgress->setValue(nDifference);
    }
}

void MapAlarmVideoWidget::slot_getTime(const float& p_dTime)
{
    //记录用户操作的系统时间
    g_TimeConvertInstance->getLongTime(m_nLocateSystemTimeMilliseconds, QDateTime::currentDateTime());
    ui->horizontalSliderProgress->setValue((int)p_dTime);

    m_bEnd = false;
    resetSpeed();

    //快进必须重置当前时间
    m_ulCurTime = m_ulBeginTime + (int)p_dTime;

    //实时报警或报警检索只有单窗口播放，所以窗口ID是1
    emit locateTime(1, (m_ulBeginTime + (int)p_dTime), m_ulBeginTime, m_ulEndTime);
}

void MapAlarmVideoWidget::slot_readFileFinish()
{
    //文件尾时需要将时间进度条运行到尽头
    m_bEnd = true;
    ui->horizontalSliderProgress->setValue(MAX_SLIDER_RANGE);
    ui->labelCurrTime->setText(g_strEndTime);

    if(!m_bShowErrorFlag)
    {
        if(OperatorTypePlayreal == m_nType)
        {
            emit stopAllPlayerVideo(RealPlayer);
        }
        else
        {
            emit stopAllPlayerVideo(RecordPlayer);
        }
    }
}

void MapAlarmVideoWidget::slot_happenError(const int&)
{
    //录像回放遇到错误码需要禁用控制按钮
    if (OperatorTypePlayback == m_nType)
    {
        ui->horizontalSliderProgress->setEnabled(false);
        ui->pushButtonPlayVideo->setEnabled(false);
        ui->pushButtonPause->setEnabled(false);
        ui->pushButtonSpeedDown->setEnabled(false);
        ui->pushButtonSpeedUp->setEnabled(false);
        ui->pushButtonDownloadVideo->setEnabled(false);
    }

    if(!m_bShowErrorFlag)
    {
        if(OperatorTypePlayreal == m_nType)
        {
            emit stopAllPlayerVideo(RealPlayer);
        }
        else
        {
            emit stopAllPlayerVideo(RecordPlayer);
        }
    }
}

void MapAlarmVideoWidget::setWidgetType(int p_nWidgetType)
{
    //设置窗口类型

    m_pWidgetType = p_nWidgetType;

    resetParam();

    if(p_nWidgetType == Qt::Window)
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
        setWindowModality(Qt::WindowModal);
    }
    else if(p_nWidgetType == Qt::Widget)
    {
        setWindowFlags(Qt::FramelessWindowHint);
        setWindowModality(Qt::NonModal);
        setModel(OperatorTypeView);
    }
    activateWindow();
}

void MapAlarmVideoWidget::setControllerBarLayout(int p_nType)
{
    //设置控制栏布局
    if(p_nType == Full)
    {
        ui->horizontalLayout_2->removeWidget(ui->pushButtonFullScreen);
        ui->horizontalLayout_2->removeItem(ui->horizontalSpacer_2);
        ui->horizontalLayout_2->addWidget(ui->pushButtonFullScreen);
        ui->horizontalLayout_2->addItem(ui->horizontalSpacer_2);
        ui->pushButtonDownloadVideo->show();
        ui->label->show();
        ui->labelCurrTime->show();
        ui->labelTotalTime->show();
        ui->horizontalSliderProgress->show();
    }
    else if(p_nType == Sample)
    {
        ui->horizontalLayout_2->removeWidget(ui->pushButtonFullScreen);
        ui->horizontalLayout_2->removeItem(ui->horizontalSpacer_2);
        ui->horizontalLayout_2->addItem(ui->horizontalSpacer_2);
        ui->horizontalLayout_2->addWidget(ui->pushButtonFullScreen);
        ui->pushButtonDownloadVideo->hide();
        ui->label->hide();
        ui->labelCurrTime->hide();
        ui->labelTotalTime->hide();
        ui->horizontalSliderProgress->hide();
    }
    m_nType = OperatorTypePlayback;
}

void MapAlarmVideoWidget::setErrorDisplay(bool p_bFlag)
{
    m_bShowErrorFlag = p_bFlag;
}

void MapAlarmVideoWidget::clearVideoResource()
{
    ui->horizontalSliderProgress->setValue(MAX_SLIDER_RANGE);
    ui->labelCurrTime->setText(g_strEndTime);
    ui->horizontalSliderProgress->setEnabled(false);
    ui->pushButtonPlayVideo->setEnabled(false);
    ui->pushButtonPause->setEnabled(false);
    ui->pushButtonSpeedDown->setEnabled(false);
    ui->pushButtonSpeedUp->setEnabled(false);
    ui->pushButtonDownloadVideo->setEnabled(false);

    if(!m_bShowErrorFlag)
    {
        if(OperatorTypePlayreal == m_nType)
        {
            emit stopAllPlayerVideo(RealPlayer);
        }
        else
        {
            emit stopAllPlayerVideo(RecordPlayer);
        }
    }
}

void MapAlarmVideoWidget::on_pushButtonTalk_clicked()
{
    m_pSubWin = ui->alarmWidgetPlayer->getPlayVideoSubWin(1);
    if(m_pSubWin)
    {
        emit m_pSubWin->ctrlTalk(1,!m_bTalk);
    }
}

void MapAlarmVideoWidget::ctrlTalkOpenSuccess(bool bTalk)
{
    if(bTalk)
    {
        m_bTalk = true;
        ui->pushButtonTalk->setIcon(QIcon(":images/default/liveview/video_interPhone_pressed.png"));
    }
    else
    {
        m_bTalk = false;
        ui->pushButtonTalk->setIcon(QIcon(":images/default/liveview/video_interPhone.png"));
    }
}

void MapAlarmVideoWidget::slotTypeChanged(int index)
{
    Q_UNUSED(index);
    if(m_pSubWin && m_bTalk)
    {
        emit m_pSubWin->ctrlTalk(1,false);
    }
    if(OperatorTypePlayreal == m_nType)
    {
        emit stopAllPlayerVideo(RealPlayer);
    }
    else
    {
        emit stopAllPlayerVideo(RecordPlayer);
    }
    if(ui->comboBox->currentData().toInt() == OperatorTypePlayreal)
    {
        openAlarmVideoDialog(OperatorTypePlayreal,m_strChannelUid,QDateTime::currentDateTime().toString(DATETIME_FORMATE_YMD),ui->labelTitle->text());
    }
    else
    {
        openAlarmVideoDialog(OperatorTypePlayback,m_strChannelUid,QDateTime::currentDateTime().toString(DATETIME_FORMATE_YMD),ui->labelTitle->text());
    }
}

void MapAlarmVideoWidget::on_pushButtonDate_clicked()
{
    m_pAlarmVideoTimeDlg->openDialog();
}

void MapAlarmVideoWidget::slotTimeSelected(QDateTime start,QDateTime end)
{
    InviewChannel* pChannel = g_SppClient->getChannelByUid(m_strChannelUid.toStdString());
    if(pChannel == NULL)
    {
        return;
    }
    QString startTime = start.toString("yyyy-MM-dd HH:mm:ss");
    QString endTime = end.toString("yyyy-MM-dd HH:mm:ss");
    emit playWndVideo(1, pChannel->get_id(), OperatorTypePlayback, startTime, startTime, endTime);
}

QString MapAlarmVideoWidget::second2TimeString(int second)
{
    int h,m,s;
    h = second / 3600;
    m = (second % 3600) / 60;
    s = (second % 3600) % 60;
    QString time = QString::number(h).rightJustified(2,'0');
    time += QString(":");
    time += QString::number(m).rightJustified(2,'0');
    time += QString(":");
    time += QString::number(s).rightJustified(2,'0');
    return time;
}
