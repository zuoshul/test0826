#include <QFileDialog>
#include "gisaddnewmapdialog.h"
#include "ui_gisaddnewmapdialog.h"
#include "src/base/apptextvalidator.h"
#include "sppclientmapmanager.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

GisAddNewMapDialog::GisAddNewMapDialog(QWidget *parent) :
    SNDialog(parent),
    ui(new Ui::GisAddNewMapDialog),
    m_height(LOCAL_ADD_HEIGHT),
    m_addFirstMap(false),
    m_pThread(NULL),
    m_pZipObj(NULL),
    m_processDialog(NULL)
{
    ui->setupUi(this);
    initForm();
}

GisAddNewMapDialog::~GisAddNewMapDialog()
{
    delete ui;
}

void GisAddNewMapDialog::initForm()
{
    connect(ui->comboBoxGisMapLoadType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotGisMapLoadTypeChanged(int)));

    ui->comboBoxMapType->addItem(QObject::tr("TK_GisMap"),MSG_MAP_TYPE_NODE); //GIS地图,消息定义的宏名字完全突出不了GIS
    ui->comboBoxMapType->addItem(QObject::tr("TK_LocalMap"),MSG_MAP_TYPE_CUSTOM); //本地地图
    ui->comboBoxGisMapDataType->addItem(QObject::tr("TK_OSM"),MSG_GIS_TYPE_OSM);
    ui->comboBoxGisMapDataType->addItem(QObject::tr("TK_Baidu"),MSG_GIS_TYPE_BAIDU);
    ui->comboBoxGisMapDataType->addItem(QObject::tr("TK_Google"),MSG_GIS_TYPE_GOOGLE);
    ui->comboBoxGisMapLoadType->addItem(QObject::tr("TK_Online"),MSG_LOAD_TYPE_ONLINE);
    ui->comboBoxGisMapLoadType->addItem(QObject::tr("TK_Offline"),MSG_LOAD_TYPE_OFFLINE);

    initCustomStyle();

    retranslateUi();
}

void GisAddNewMapDialog::initCustomStyle()
{
    AppStyleHelper::updateWidgetStyle(ui->pushButtonCancel, AppStyleHelper::Style_PushButtonDefault);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonConfirm, AppStyleHelper::Style_PushButtonDefault);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonSelectMap, AppStyleHelper::Style_PushButtonDefault);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonSelectOfflinePath, AppStyleHelper::Style_PushButtonDefault);
    AppStyleHelper::updateWidgetStyle(ui->line, AppStyleHelper::Style_Frame_CutLine);
}

void GisAddNewMapDialog::retranslateUi()
{
    ui->pushButtonCancel->setText(QObject::tr("TK_Cancel"));
    ui->pushButtonConfirm->setText(QObject::tr("TK_Confirm"));
    ui->retranslateUi(this);
}

bool GisAddNewMapDialog::verifyContent()
{
    return verifyLineEditText(ui->lineEditLocalMapName) && verifyLineEditText(ui->lineEditLocalMapPath) && verifyLineEditText(ui->lineEditOfflineGisMapPath);
}

bool GisAddNewMapDialog::doVerifyLineEditText(QLineEdit* lineEdit, QString& toolTipStr)
{
    bool checkRs = true;
    if(lineEdit->text().isEmpty())
    {
        if(lineEdit == ui->lineEditLocalMapName && lineEdit->isVisible())
        {
            checkRs = false;
            toolTipStr.append(QObject::tr("TK_MapNameEmpty"));
        }
        if(lineEdit == ui->lineEditLocalMapPath && lineEdit->isVisible())
        {
            checkRs = false;
            toolTipStr.append(QObject::tr("TK_MapPathEmpty"));
        }
        if(lineEdit == ui->lineEditOfflineGisMapPath && lineEdit->isVisible())
        {
            checkRs = false;
            toolTipStr.append(tr("TK_SelectOfflineMapPak"));
        }
    }
    else
    {
        if(lineEdit == ui->lineEditLocalMapName && lineEdit->isVisible())
        {
            if(ui->lineEditLocalMapName->text().indexOf(" ") != -1)
            {
                checkRs = false;
                toolTipStr.append(QObject::tr("TK_IllegalCharacters"));
            }
            QString res;
            if(APPTextValidator::isStringWithSpecialChar(ui->lineEditLocalMapName->text(), res))
            {
                checkRs = false;
                toolTipStr.append(res);
            }
            if(!APPTextValidator::isStringLengthValid(ui->lineEditLocalMapName->text(), res))
            {
                checkRs = false;
                toolTipStr.append(res);
            }
        }   
    }
    return checkRs;
}

void GisAddNewMapDialog::on_pushButtonClose_clicked()
{
    this->close();
}

void GisAddNewMapDialog::on_pushButtonConfirm_clicked()
{
    if(verifyContent())
    {
        if(ui->comboBoxMapType->currentData().toInt() == MSG_MAP_TYPE_NODE && ui->comboBoxMapType->isVisible())
        {
            if(ui->comboBoxGisMapLoadType->currentData().toInt() == MSG_LOAD_TYPE_OFFLINE)
            {
                QString path = QDir::homePath() + QString("/") + MAPGRAPHICS_CACHE_FOLDER_NAME;
                unzipTiles(ui->lineEditOfflineGisMapPath->text(),path);
                m_processDialog->changeProcess(1,0,0);
                m_processDialog->show();
                return;
            }
            else
            {
                emit gisMapModeSelected();
            }
        }
        else
        {
            emit(signalMapSelectFinished(ui->lineEditLocalMapName->text(),ui->lineEditLocalMapPath->text()));
        }
        close();
    }
}

void GisAddNewMapDialog::on_pushButtonCancel_clicked()
{
    this->close();
}

void GisAddNewMapDialog::openDialog(bool firstMap)
{
    m_height = firstMap ? FIRST_ADD_HEIGHT : LOCAL_ADD_HEIGHT;
    m_addFirstMap = firstMap;

    ui->lineEditLocalMapName->clear();
    ui->lineEditLocalMapPath->clear();
    ui->lineEditOfflineGisMapPath->clear();
    ui->lineEditLocalMapName->setPlaceholderText("");
    ui->lineEditLocalMapPath->setPlaceholderText("");
    ui->lineEditOfflineGisMapPath->setPlaceholderText("");
    ui->comboBoxMapType->setCurrentIndex(0);

    addFirstMap(firstMap);

    showGisMode(firstMap ? true : false);

    resetHeight();

    showSNDialog();
}

void GisAddNewMapDialog::on_pushButtonSelectMap_clicked()
{
    QString fileName = QFileDialog::getOpenFileName( this, QObject::tr("Open File"),"", "*.png *.xpm *.jpg *jpeg");

    if ((!fileName.isEmpty()))
    {
        fileName = fileName.replace("\\", "/");
        ui->lineEditLocalMapPath->setText(fileName);

        if(ui->lineEditLocalMapName->text().isEmpty())
        {
            QFileInfo fileInfo(fileName);
            ui->lineEditLocalMapName->setText(fileInfo.baseName());
        }
     }
}

void GisAddNewMapDialog::slotMapTypeChanged(int index)
{
    Q_UNUSED(index);
    bool gis = (ui->comboBoxMapType->currentData().toInt() == MSG_MAP_TYPE_NODE);
    showGisMode(gis);

    resetHeight();
}

void GisAddNewMapDialog::slotGisMapLoadTypeChanged(int index)
{
    Q_UNUSED(index);
    bool offline = (ui->comboBoxGisMapLoadType->currentData().toInt() == MSG_LOAD_TYPE_OFFLINE);
    showOfflineGisMap(offline);

    resetHeight();
}

void GisAddNewMapDialog::addFirstMap(bool first)
{
    disconnect(ui->comboBoxMapType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotMapTypeChanged(int)));
    if(first)
    {
        connect(ui->comboBoxMapType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotMapTypeChanged(int)));
    }
    ui->comboBoxMapType->setVisible(first);
    ui->labelMapType->setVisible(first);
}

void GisAddNewMapDialog::showGisMode(bool gis)
{
    ui->comboBoxGisMapDataType->setVisible(gis);
    ui->labelGisMapDataType->setVisible(gis);
    ui->comboBoxGisMapLoadType->setVisible(gis);
    ui->labelGisMapLoadType->setVisible(gis);
    ui->lineEditLocalMapName->setVisible(!gis);
    ui->labelLocalMapName->setVisible(!gis);
    ui->lineEditLocalMapPath->setVisible(!gis);
    ui->labelLocalMapPath->setVisible(!gis);
    ui->pushButtonSelectMap->setVisible(!gis);

    bool showOfflinePathSelect = (gis && ui->comboBoxGisMapLoadType->currentData().toInt() == MSG_LOAD_TYPE_OFFLINE);
    showOfflineGisMap(showOfflinePathSelect);
}

void GisAddNewMapDialog::showOfflineGisMap(bool show)
{
    ui->lineEditOfflineGisMapPath->setVisible(show);
    ui->labelOfflineGisMapPath->setVisible(show);
    ui->pushButtonSelectOfflinePath->setVisible(show);

    if(m_addFirstMap)
    {
        m_height = show ? FIRSTANDOFFLINE_ADD_HEIGHT : FIRST_ADD_HEIGHT;
    }
}

void GisAddNewMapDialog::on_pushButtonSelectOfflinePath_clicked()
{
    QString fileName = QFileDialog::getOpenFileName( this, QObject::tr("Open File"),"", "*.zip *.rar");

    if((!fileName.isEmpty()))
    {
        fileName = fileName.replace("\\", "/");
        ui->lineEditOfflineGisMapPath->setText(fileName);
    }
}

void GisAddNewMapDialog::resetHeight()
{
    setFixedHeight(m_height);
}

void GisAddNewMapDialog::unzipTiles(QString sourcePath,QString newPath)
{
    if(m_pThread == NULL)
    {
        m_pThread = new QThread(this);
        m_pZipObj = new ZipAndUnzip(this);
        m_processDialog = new DevProgressDialog(this);
        m_processDialog->showAddProcessBar(0, 1);
        QObject::connect(m_pZipObj,SIGNAL(signalFinished(bool)),this,SLOT(unzipFinished(bool)));
        QObject::connect(m_pThread,SIGNAL(started()),m_pZipObj,SLOT(unzipFileToDirectory()));
        m_pZipObj->moveToThread(m_pThread);
    }

    m_pZipObj->setParam(sourcePath,newPath);
    m_pThread->start();
}

void GisAddNewMapDialog::unzipFinished(bool ret)
{
    m_pThread->quit();
    m_pThread->wait();

    int success = 0;
    int failed = 0;
    if(ret)
    {
        success = 1;
    }
    else
    {
        failed = 1;
    }
    m_processDialog->changeProcess(success + failed,failed,success);
    m_processDialog->close();
    if(ret)
    {
        emit gisMapModeSelected();
        close();
    }
}
