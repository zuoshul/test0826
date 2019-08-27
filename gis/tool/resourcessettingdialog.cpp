#include "resourcessettingdialog.h"
#include "ui_resourcessettingdialog.h"
#include "sppclientdevicemanager.h"
#include <QGridLayout>

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

ResourcesSettingDialog::ResourcesSettingDialog(QWidget *parent) :
    BaseWidget(parent),
    ui(new Ui::ResourcesSettingDialog),
    m_pSettings(NULL)
{
    ui->setupUi(this);
    hide();

    QString curPath = QString("%1/mapconfig.ini").arg(qApp->applicationDirPath());
    m_pSettings = new QSettings(curPath, QSettings::IniFormat);
    if(!QFile::exists(curPath))
    {
        resetFilterParam();
        saveMapConfig(m_filter);
    }
    else
    {
        readMapConfig();
    }
    initForm();
}

ResourcesSettingDialog::~ResourcesSettingDialog()
{
    delete ui;
}

void ResourcesSettingDialog::initForm()
{
    m_devTypeList.clear();
    QGridLayout* layout = new QGridLayout(ui->widgetDevFlow);
    layout->setContentsMargins(0,0,0,0);
    InviewDeviceTypeNodeList deviceTypeNodeList;
    g_SppClient->getDeviceTypeNodeList(deviceTypeNodeList);
    int row = 0;
    int col = 0;
    for(int i = 0; i < deviceTypeNodeList.size(); i++)
    {
        NODE& node = deviceTypeNodeList.at(i);
        InviewDeviceType* type = (InviewDeviceType*)node.pData;
        if(!type)
        {
            continue;
        }
        int typeId = type->get_id();
        if(typeId == INVIEW_DEVICE_TYPE_DVR || typeId == INVIEW_DEVICE_TYPE_NVR)
        {
            continue;
        }
        QCheckBox* pCheck = new QCheckBox(this);
        pCheck->setText(node.strNodeName);
        pCheck->setAccessibleName(QString::number(typeId));
        layout->addWidget(pCheck,row,col);
        if(col != 0)
        {
            col = 0;
            row++;
        }
        else
        {
            col = 1;
        }
        m_devTypeList.append(pCheck);
        pCheck->setChecked(isCheckedByType(typeId));
    }
    ui->checkBoxAlarm->setChecked(m_filter.mapAlarm);
    ui->checkBoxOffline->setChecked(m_filter.offline);
    ui->checkBoxOnline->setChecked(m_filter.online);
    ui->checkBoxShowArea->setChecked(m_filter.showArea);

    QList<QCheckBox*> list = this->findChildren<QCheckBox*>();
    for(int i = 0;i < list.size();i++)
    {
        connect(list.at(i),SIGNAL(stateChanged(int)),this,SLOT(slotStateChanged(int)));
    }

    retranslateUi();
}

void ResourcesSettingDialog::retranslateUi()
{
    ui->labelDevType->setText(QObject::tr("TK_DevType"));
    ui->labelDevStatus->setText(QObject::tr("TK_OnlineState"));
    ui->labelOther->setText(QObject::tr("TK_Other"));
    ui->checkBoxAlarm->setText(tr("TK_MapAlarm"));
    ui->checkBoxShowArea->setText(tr("TK_ShowAgentArea"));
    ui->checkBoxOnline->setText(QObject::tr("TK_Online"));
    ui->checkBoxOffline->setText(QObject::tr("TK_Offline"));
    ui->retranslateUi(this);
}

void ResourcesSettingDialog::doAfterWidgetShow()
{

}

void ResourcesSettingDialog::openDialog(bool open)
{
    if(open)
    {
        show();
    }
    else
    {
        hide();
    }
}

void ResourcesSettingDialog::on_pushButtonClose_clicked()
{
    close();
}

void ResourcesSettingDialog::slotStateChanged(int state)
{
    //QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(sender());
    Q_UNUSED(state);
    m_filter.mapAlarm = ui->checkBoxAlarm->isChecked();
    m_filter.showArea = ui->checkBoxShowArea->isChecked();
    m_filter.offline = ui->checkBoxOffline->isChecked();
    m_filter.online = ui->checkBoxOnline->isChecked();
    m_filter.showList.clear();
    for(int i = 0;i < m_devTypeList.size();i++)
    {
        QCheckBox* pCheck = m_devTypeList.at(i);
        if(pCheck->checkState() == Qt::Checked)
        {
            int typeId = pCheck->accessibleName().toInt();
            m_filter.showList.append(typeId);
        }
    }
    emit checkStateChanged(m_filter);
    saveMapConfig(m_filter);
}

void ResourcesSettingDialog::readMapConfig()
{
    if(m_pSettings == NULL)
    {
        return;
    }
    m_filter.mapAlarm = m_pSettings->value("config/mapAlarm").toInt() == 0 ? true : false;
    m_filter.showArea = m_pSettings->value("config/showArea").toInt() == 0 ? true : false;
    m_filter.offline = m_pSettings->value("config/offline").toInt() == 0 ? true : false;
    m_filter.online = m_pSettings->value("config/online").toInt() == 0 ? true : false;
    QString typeList = m_pSettings->value("config/typeList").toString();
    QStringList list = typeList.split("|");
    m_filter.showList.clear();
    for(int i = 0; i < list.size();i++)
    {
        m_filter.showList.append(list.at(i).toInt());
    }
}

void ResourcesSettingDialog::getMapConfig(ResourcesSettingDialog::ResourcesFilter &filter)
{
    filter = m_filter;
}

void ResourcesSettingDialog::saveMapConfig(ResourcesSettingDialog::ResourcesFilter filter)
{
    if(m_pSettings == NULL)
    {
        return;
    }
    m_pSettings->setValue("config/mapAlarm",filter.mapAlarm ? 0 : -1);
    m_pSettings->setValue("config/showArea",filter.showArea ? 0 : -1);
    m_pSettings->setValue("config/offline",filter.offline ? 0 : -1);
    m_pSettings->setValue("config/online",filter.online ? 0 : -1);
    QList<int> typeList = filter.showList;
    QString listValue;
    for(int i = 0; i < typeList.size();i++)
    {
        listValue += QString::number(typeList.at(i));
        if(i != typeList.size() - 1)
        {
            listValue += "|";
        }
    }
    m_pSettings->setValue("config/typeList",listValue);
}

bool ResourcesSettingDialog::isCheckedByType(int type)
{
    for(int i = 0; i < m_filter.showList.size();i++)
    {
        if(type == m_filter.showList.at(i))
        {
            return true;
        }
    }
    return false;
}

void ResourcesSettingDialog::resetFilterParam()
{
    m_filter.showList.clear();
    InviewDeviceTypeNodeList deviceTypeNodeList;
    g_SppClient->getDeviceTypeNodeList(deviceTypeNodeList);
    for(int i = 0; i < deviceTypeNodeList.size();i++)
    {
        NODE& node = deviceTypeNodeList.at(i);
        InviewDeviceType* type = (InviewDeviceType*)node.pData;
        if(!type)
        {
            continue;
        }
        int typeId = type->get_id();
        if(typeId == INVIEW_DEVICE_TYPE_DVR || typeId == INVIEW_DEVICE_TYPE_NVR)
        {
            continue;
        }
        m_filter.showList.append(typeId);
        m_filter.mapAlarm = true;
        m_filter.offline = true;
        m_filter.online = true;
        m_filter.showArea = true;
    }
}

void ResourcesSettingDialog::pushFilterRules()
{
    //主动推送规则
    emit checkStateChanged(m_filter);
}
