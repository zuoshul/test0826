#include <QDebug>
#include "maptrackreplaydialog.h"
#include "ui_maptrackreplaydialog.h"
#include "mapgraphwidget.h"
#include "src/sppclient/sppclientmapmanager.h"
#include "src/common/snmessageboxflowdownwidget.h"

MapTrackReplayDialog::MapTrackReplayDialog(QWidget *parent) :
    SNDialog(parent),
    ui(new Ui::MapTrackReplayDialog),
    m_nDuration(5),
    m_nRepeat(1)
{
    ui->setupUi(this);
    initForm();
}

MapTrackReplayDialog::~MapTrackReplayDialog()
{
    delete ui;
}

void MapTrackReplayDialog::doAfterWidgetShow()
{
    if(m_objMapUrl.isEmpty())
    {
        SNMessageBoxFlowDownWidget::showMessage(
            QObject::tr("TK_HaveNoMap"),
            this,
            SNMessageBoxFlowDownWidget::MessageError);
        return;
    }

    //地图不存在？ 开始下载
    if(!MapUtil::checkLocalMapExist(m_objMapUrl))
    {
        m_pMapUtil->startDownloadMap(m_objMapUrl);
    }
    else
    {
        changeMapInfo();
    }
}

void MapTrackReplayDialog::doAfterWidgetHide()
{

}

void MapTrackReplayDialog::showDialog(
        QString mapUrl,
        QList<MapButton>& ipcs,
        QStringList flagImages,
        int duration,
        int repeat)
{
    m_objMapUrl = mapUrl;
    m_objIpcs = ipcs;
    m_flagImages = flagImages;
    m_nDuration = duration;
    m_nRepeat = repeat;
    this->show();
}

void MapTrackReplayDialog::initForm()
{
    m_pMapUtil = new MapUtil(this);
    connect(m_pMapUtil, SIGNAL(downloadFinished(int)), this, SLOT(slotDownloadMapFinished(int)));

    connect(&m_objTrackTimer, SIGNAL(timeout()), this, SLOT(slotMove()));
}

void MapTrackReplayDialog::changeMapInfo()
{
    QList<MapButton> subMapButtons;
    QList<MapButton> ipcButtons;
    ui->widgetMap->setMapInfo(
                MapUtil::getLocalAbsoluteMapPath(m_objMapUrl),
                subMapButtons,
                ipcButtons);

    ui->widgetMap->addTrack(m_objIpcs);
    ui->widgetMap->switchMapState(MapStatePreview);

    m_objTrackCoordinates.clear();
    curTrackId = 0;
    if(m_objIpcs.count() >= 2)
    {
        for(int i=0; i<m_objIpcs.count()-1; i++)
        {
            getSegmentCoordinate(
                        m_objIpcs.at(i).centerPoint,
                        m_objIpcs.at(i+1).centerPoint,
                        5,
                        m_objTrackCoordinates);

        }
    }
    else
    {
        SNMessageBoxFlowDownWidget::showMessage(
                    QObject::tr("TK_SelectMoreIpc"),
                    this,
                    SNMessageBoxFlowDownWidget::MessageError);
    }
    on_pushButtonPlay_clicked();
}

void MapTrackReplayDialog::retranslateUi()
{
    ui->retranslateUi(this);
}

void MapTrackReplayDialog::on_pushButtonClose_clicked()
{
    ui->widgetMap->clearMapInfo();
    m_objTrackTimer.stop();
    setAutoRefreashWidget(true);
    this->close();
}

void MapTrackReplayDialog::on_pushButtonPlay_clicked()
{
    switchState(statePlay);
}

void MapTrackReplayDialog::on_pushButtonPause_clicked()
{
    switchState(statePause);
}

void MapTrackReplayDialog::on_pushButtonPrevious_clicked()
{
    switchState(statePause);
    move(moveBackward);
}

void MapTrackReplayDialog::on_pushButtonNext_clicked()
{
    switchState(statePause);
    move(moveForward);
}

void MapTrackReplayDialog::slotDownloadMapFinished(int state)
{
    if(state == 0)
    {
        changeMapInfo();
    }
    else
    {
        SNMessageBoxFlowDownWidget::showMessage(
                    g_SPPClientMapManager->getLastErrorString(),
                    this,
                    SNMessageBoxFlowDownWidget::MessageError);
    }
}

void MapTrackReplayDialog::getSegmentCoordinate(
        const QPoint& startPoint,
        const QPoint& endPoint,
        int spliltTo,
        QList<QPoint>& coordinates)
{
    float x1 = startPoint.x();
    float y1 = startPoint.y();
    float x2 = endPoint.x();
    float y2 = endPoint.y();

    int i=0;
    for(; i<=spliltTo; i++)
    {
        int x = x1 + (x2-x1)*((float)i/spliltTo);
        int y = y1 + (y2-y1)*((float)i/spliltTo);
        coordinates.push_back(QPoint(x, y));
    }
}

void MapTrackReplayDialog::moveTo(int trackId)
{
    if(trackId >= 0 && trackId < m_objTrackCoordinates.size())
    {
        qDebug() << "moveTo " << trackId
                 << " Coordinates " << m_objTrackCoordinates.at(curTrackId);

        int index = (trackId + 1) / 6;
        if(index < 0 || index > m_flagImages.size() - 1 || m_flagImages.isEmpty())
        {
            m_objFlagImage = QString(":/images/test/face.png");
        }
        else
        {
            m_objFlagImage = m_flagImages[index];
        }

        ui->widgetMap->moveFlag(m_objFlagImage,m_objTrackCoordinates.at(curTrackId));
    }
}

void MapTrackReplayDialog::move(MoveDirection direction)
{
    //数组下表
    int totalStep = m_objTrackCoordinates.size() - 1;
    if(direction == moveForward)
    {
        if(curTrackId == totalStep)
        {
            //移动到末尾
            SNMessageBoxFlowDownWidget::showMessage(
                QObject::tr("TK_TailAttached"),
                this,
                SNMessageBoxFlowDownWidget::MessageInfo);
            goto MoveFinised;
        }
        curTrackId++;
        moveTo(curTrackId);
    }
    else if(direction == moveBackward)
    {
        if(curTrackId == 0)
        {
            //移动到开始位置
            SNMessageBoxFlowDownWidget::showMessage(
                QObject::tr("TK_HeadAttached"),
                this,
                SNMessageBoxFlowDownWidget::MessageInfo);
            goto MoveFinised;
        }
        curTrackId--;
        moveTo(curTrackId);
    }
    return;

MoveFinised:
    switchState(statePause);
}

void MapTrackReplayDialog::switchState(ReplayState state)
{
    if(state == statePlay)
    {
        m_objTrackTimer.start(200);
        if(!m_objTrackCoordinates.empty() && curTrackId == m_objTrackCoordinates.size() - 1)
        {
            //到达了轨迹的最末尾，重头开始播放
            curTrackId = 0;
        }

        ui->pushButtonPlay->setEnabled(false);
        ui->pushButtonPause->setEnabled(true);
    }
    else if(state == statePause)
    {
        m_objTrackTimer.stop();
        ui->pushButtonPlay->setEnabled(true);
        ui->pushButtonPause->setEnabled(false);
    }
}

void MapTrackReplayDialog::slotMove()
{
    move(moveForward);
}

void MapTrackReplayDialog::on_pushButtonReFreash_clicked()
{
    curTrackId = 0;
    switchState(statePlay);
}
