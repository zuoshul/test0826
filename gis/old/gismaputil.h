#ifndef MAPUTIL_H
#define MAPUTIL_H
#include <QObject>
#include <QString>
#include <QTimerEvent>

class MapUtil:public QObject
{
    Q_OBJECT
public:
    MapUtil(QObject* parent=NULL);

    static QString getLocalMapDirPath();

    static QString getLocalAbsoluteMapPath(const QString& fileName);

    static QString getRemotRelativeMapPath(const QString& fileName);

    static bool checkLocalMapExist(const QString& fileName);

    bool startDownloadMap(QString mapUrl);

signals:
    //0 成功， 非0 错误码
    void downloadFinished(int state);

protected:
    virtual void timerEvent(QTimerEvent *event);

private:
    void downloadStateChanged();

private:
    int m_nDownloadTimeId;

};

#endif // MAPUTIL_H
