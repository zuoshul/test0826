#include "mapsearchdialog.h"
#include "ui_mapsearchdialog.h"
#include "src/gis/guts/MapGraphicsNetwork.h"
#include <QNetworkReply>

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

MapSearchDialog::MapSearchDialog(QWidget *parent) :
    BaseWidget(parent),
    ui(new Ui::MapSearchDialog)
{
    ui->setupUi(this);
    initForm();
}

MapSearchDialog::~MapSearchDialog()
{
    delete ui;
}

void MapSearchDialog::initForm()
{
    hide();
    retranslateUi();
}

void MapSearchDialog::retranslateUi()
{
    //ui->labelTemp->setText(QObject::tr("TK_Temperature"));
    ui->retranslateUi(this);
}

void MapSearchDialog::doAfterWidgetShow()
{

}

void MapSearchDialog::openDialog(bool open,QPointF point,int zoomLevel)
{
    if(open)
    {
        show();
        location(point.x(),point.y(),zoomLevel);
    }
    else
    {
        hide();
    }
}

void MapSearchDialog::handleNetworkRequestFinished()
{
    QObject * sender = QObject::sender();
    QNetworkReply * reply = qobject_cast<QNetworkReply *>(sender);

    if (reply == 0)
    {
        qWarning() << "MapSearchDialog::handleNetworkRequestFinished() QNetworkReply cast failure";
        return;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "MapSearchDialog::handleNetworkRequestFinished() Network Error:" << reply->errorString();
        return;
    }

    QByteArray bytes = reply->readAll();
    QString flg = QString("data-name=");
    int pos = bytes.indexOf(flg);
    bytes = bytes.mid(pos + flg.length() + 1);
    pos = bytes.indexOf('\"');
    bytes = bytes.left(pos);
    QStringList list;
    list.append(bytes);
    resetItems(list);

    reply->deleteLater();
}

void MapSearchDialog::resetItems(QStringList areaList)
{
    ui->comboBoxPosition->clear();
    for(int i = 0;i < areaList.size();i++)
    {
        ui->comboBoxPosition->addItem(areaList.at(i));
    }
}

void MapSearchDialog::location(int lon,int lat,int zoom)
{
    MapGraphicsNetwork *network = MapGraphicsNetwork::getInstance();
    QString host = "https://www.openstreetmap.org/geocoder/search_osm_nominatim_reverse?lat=";
    QString url = QString("%1&lon=%2&zoom=%3").arg(lat).arg(lon).arg(zoom);

    QString fetchURL = host + url;
    QNetworkRequest request;
    request.setUrl(QUrl(fetchURL));
    request.setRawHeader("Accept-Language","zh-CN");
    QNetworkReply *reply = network->get(request);

    connect(reply,
            SIGNAL(finished()),
            this,
            SLOT(handleNetworkRequestFinished()));

    qDebug()<<"MapSearchDialog::location() lon "<<lon<<"lat "<<lat<<"zoom "<<zoom<<endl;
}
