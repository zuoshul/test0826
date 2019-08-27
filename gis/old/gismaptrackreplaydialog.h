#ifndef MAPTRACKREPLAYDIALOG_H
#define MAPTRACKREPLAYDIALOG_H

#include <QWidget>
#include <QTimerEvent>
#include <QTimer>
#include "mapgraphwidget.h"
#include "src/common/sndialog.h"
#include "src/common/Data.h"
#include "maputil.h"

namespace Ui {
class MapTrackReplayDialog;
}

class MapTrackReplayDialog : public SNDialog
{
    Q_OBJECT
    typedef enum
    {
        moveForward,
        moveBackward
    }MoveDirection;

    typedef enum
    {
        statePlay,
        statePause
    }ReplayState;

public:
    explicit MapTrackReplayDialog(QWidget *parent = 0);
    ~MapTrackReplayDialog();

    virtual void doAfterWidgetShow();

    virtual void doAfterWidgetHide();

    void showDialog(QString mapUrl,
                    QList<MapButton>& ipcs,
                    QStringList flagImages,
                    int duration = 5,
                    int repeat = 1);

private:
    virtual void initForm();

    virtual void retranslateUi();

    void changeMapInfo();

    //把起始点为startPoint， 结束点为endPoint的线段
    //分成spliltTo段，并返回各个点的坐标
    void getSegmentCoordinate(
            const QPoint& startPoint,
            const QPoint& endPoint,
            int spliltTo,
            QList<QPoint>& coordinates);


private slots:
    void on_pushButtonClose_clicked();

    void on_pushButtonPlay_clicked();

    void on_pushButtonPause_clicked();

    void on_pushButtonPrevious_clicked();

    void on_pushButtonNext_clicked();

    void slotDownloadMapFinished(int state);

    void slotMove();

    void moveTo(int trackId);

    void move(MoveDirection direction);

    void switchState(ReplayState state);

    void on_pushButtonReFreash_clicked();

private:
    Ui::MapTrackReplayDialog *ui;
    QString m_objMapUrl;
    QList<MapButton> m_objIpcs;
    QStringList m_flagImages;
    QList<QPoint> m_objTrackCoordinates;
    QTimer m_objTrackTimer;
    int curTrackId;
    QString m_objFlagImage;
    int m_nDuration;
    int m_nRepeat;
    MapUtil* m_pMapUtil;
};

#endif // MAPTRACKREPLAYDIALOG_H
