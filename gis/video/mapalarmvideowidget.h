#ifndef MAPALARMVIDEOWIDGET_H
#define MAPALARMVIDEOWIDGET_H

#include "src/common/sndialog.h"
#include "src/common/data.h"
#include "alarmsearchvideosubwin.h"
#include "mapalarmvideotimewidget.h"

namespace Ui {
class MapAlarmVideoWidget;
}

class MapAlarmVideoWidget : public SNDialog
{
    Q_OBJECT

public:
    explicit MapAlarmVideoWidget(QWidget *parent = 0);
    ~MapAlarmVideoWidget();

    enum ControllBarLayout{
        Full,
        Sample
    };

public:
    bool openAlarmVideoDialog(const int& p_nType, const QString& p_strUid,
                              const QString& p_strAlarmTime, const QString& p_strDeviceName, const bool& p_bDelayPlay = true);
    void initForm();

    void resetParam();

    void retranslateUi();

    void connectSignals();

    void setTitle(const QString& p_strTitle);

    void setTitleIcon(QString strStyle);

    //设置窗口类型
    void setWidgetType(int p_nWidgetType);

    //设置控制栏布局
    void setControllerBarLayout(int p_nType);

    //设置播放异常显示
    void setErrorDisplay(bool p_bFlag);

    //清除视频资源
    void clearVideoResource();

signals:
    void fullScreen(bool);

    void pause(const bool&);

    void speedPlay(const int&, const bool&);

    void continuePlay(const bool&);
    //播放
    void playWndVideo(const int&, const int&, const int&,
                      const QString&, const QString&, const QString&);
    //停止
    void stopAllPlayerVideo(const PlayerType&);

    void locateTime(const int&, const unsigned long&, const unsigned long&,
                    const unsigned long&); //ms
    //下载
    void download(const QString&, const QString&, const QString&);

    //锁屏
    void lockscreen();

public slots:
    void slot_sendMousePos(const int& p_nY);

    void slot_playbackTime(const unsigned long& p_nTime, const unsigned long& p_nSystem);

    void slot_getTime(const float& p_dTime);

    void slot_hideWnd();

    void slot_showWnd();

    void slot_refreshCtrlState(const int& p_nState);

    void slot_readFileFinish();

    void slot_happenError(const int&);

    void ctrlTalkOpenSuccess(bool bTalk);

protected:
    bool event(QEvent *event);

private slots:
    void slot_close();

    void slot_download();

    void slot_pause();

    void slot_play();

    void slot_fullScreen();

    void slot_speedDown();

    void slot_speedUp();

    void slotTypeChanged(int index);

    void on_pushButtonTalk_clicked();

    void on_pushButtonDate_clicked();

    void slotTimeSelected(QDateTime start,QDateTime end);

private:
    void resetSpeed();

    bool refreshSlowPlaySpeed();

    bool refreshFastPlaySpeed();

    QString getSpeedText(const int& p_nSpeed);

    void shortcutClose();

    void setModel(SNOperatorType type);

    QString second2TimeString(int second);

private:
    Ui::MapAlarmVideoWidget *ui;
    bool m_bFirst;      //是否收到第一帧数据
    bool m_bEnd;        //是否到文件尾
    int  m_nPlayState;  //录像播放状态
    int  m_nSpeed;      //速率
    int  m_nType;
    //用户操作定位时记录的系统时间，之后拿来跟PostEvent里的系统时间进行比较，
    //确保收到的当前时间为操作定位后的，为了达到同步的目的
    unsigned long m_nLocateSystemTimeMilliseconds; //ms
    unsigned long m_ulCurTime;          //录像当前时间，单位为秒
    unsigned long m_ulBeginTime;        //录像起始时间
    unsigned long m_ulEndTime;          //录像结束时间
    QString m_strChannelUid;            //通道UID

    int m_pWidgetType;                  //窗口类型
    bool  m_bShowErrorFlag;             //播放异常显示标志
    AlarmSearchVideoSubWin *m_pSubWin;
    bool m_bTalk;
    MapAlarmVideoTimeWidget *m_pAlarmVideoTimeDlg;
};

#endif // MAPALARMVIDEOWIDGET_H
