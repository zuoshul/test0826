#include "maputil.h"
#include <QDir>
#include <QApplication>
#include <QDebug>
#include "src/sppclient/sppclientmapmanager.h"
#include "src/base/appconfighelper.h"

MapUtil::MapUtil(QObject* parent)
    :QObject(parent)
{

}

QString MapUtil::getLocalMapDirPath(){
    QDir dir(g_appConfigHelper->getAppConfigDirPath().append("/").append("maps"));
    if(!dir.exists())
    {
        dir.mkpath(dir.absolutePath());
    }
    return dir.absolutePath();
}

QString MapUtil::getLocalAbsoluteMapPath(const QString& fileName)
{
    QString absoluteMapPaht = getLocalMapDirPath();
    return absoluteMapPaht.append("/").append(fileName);
}

QString MapUtil::getRemotRelativeMapPath(const QString& fileName)
{
    QString remotePath("map/");
    return remotePath.append(fileName);
}

bool MapUtil::checkLocalMapExist(const QString& fileName)
{
    return QFile::exists(
                getLocalAbsoluteMapPath(fileName)
                );
}

bool MapUtil::startDownloadMap(QString mapUrl)
{
    if(g_SPPClientMapManager->downloadMap(
                getLocalAbsoluteMapPath(mapUrl),
                getRemotRelativeMapPath(mapUrl)))
    {
        m_nDownloadTimeId = startTimer(100);
        return true;
    }
    else
    {//提示错误
        g_SPPClientMapManager->downloadFinished();
        return false;
    }


}

void MapUtil::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_nDownloadTimeId)
    {
        downloadStateChanged();
    }
}

void MapUtil::downloadStateChanged()
{
    TransState transState = g_SPPClientMapManager->getDownlaodState();
    qDebug() << "trans state changed! "
             << "m_nFileTransState " << transState.m_nFileTransState
             << "m_nTransSize " << transState.m_nTransSize
             << "m_nFileSize " << transState.m_nFileSize
             << "m_objFileName " << transState.m_objFileName;

    if(transState.m_nFileTransState == SPPFileTransferState_Opening
       || transState.m_nFileTransState == SPPFileTransferState_Opened
       || transState.m_nFileTransState == SPPFileTransferState_OnTrans)
    {//传输中

    }
    else if(transState.m_nFileTransState == SPPFileTransferState_Closed
                 || transState.m_nFileTransState == SPPFileTransferState_Closing)
    {//传输结束
        killTimer(m_nDownloadTimeId);
        g_SPPClientMapManager->downloadFinished();
        //修改当前显示的地图内容
        //changeMapInfo();
        emit(downloadFinished(0));
    }
    else
    {//传输出错
        killTimer(m_nDownloadTimeId);
        g_SPPClientMapManager->downloadFinished();
        emit(downloadFinished(g_SPPClientMapManager->getLastErrorCode()));
    }
}
