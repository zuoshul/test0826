#include "alarmpopdialog.h"
#include "ui_alarmpopdialog.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

AlarmPopDialog::AlarmPopDialog(QWidget *parent) :
    BaseWidget(parent),
    ui(new Ui::AlarmPopDialog)
{
    ui->setupUi(this);
    hide();
    initForm();
}

AlarmPopDialog::~AlarmPopDialog()
{
    for(int i = 0; i < ui->listWidget->count();i++)
    {
        QListWidgetItem* itemToDel = ui->listWidget->item(i);
        if(NULL != itemToDel)
        {
            QWidget* tmpWidget = ui->listWidget->itemWidget(itemToDel);
            if(NULL != tmpWidget)
            {
                tmpWidget->deleteLater();
            }
        }
        delete itemToDel;
        itemToDel = NULL;
    }
    delete ui;
}

void AlarmPopDialog::initForm()
{
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    retranslateUi();
}

void AlarmPopDialog::retranslateUi()
{
    ui->labelTitle->setText(QObject::tr("TK_RealtimeAlarm"));
    ui->pushButtonClear->setText(QObject::tr("TK_Clear"));
    ui->retranslateUi(this);
}

void AlarmPopDialog::doAfterWidgetShow()
{

}

void AlarmPopDialog::appendItem(AlarmUnitWidget::Param param)
{
    int count = ui->listWidget->count();
    if(count == 0)
    {
        param.number = 1;
    }
    else
    {
        AlarmUnitWidget* tmpWidget = (AlarmUnitWidget*)ui->listWidget->itemWidget(ui->listWidget->item(0));
        param.number = tmpWidget->getParam().number + 1;
    }
    if(count > 99)
    {
        //超过最大缓存出栈
        AlarmUnitWidget* tmpWidget = (AlarmUnitWidget*)ui->listWidget->itemWidget(ui->listWidget->item(count - 1));
        if(NULL != tmpWidget)
        {
            tmpWidget->deleteLater();
        }

        QListWidgetItem* itemToDel = ui->listWidget->takeItem(count - 1);
        if(NULL != itemToDel)
        {
            delete itemToDel;
            itemToDel = NULL;
        }
    }
    QListWidgetItem* item = new QListWidgetItem();
    item->setSizeHint(QSize(ui->listWidget->width() - 5,25));
    AlarmUnitWidget* pUnit = new AlarmUnitWidget(this);
    pUnit->setParam(param);
    ui->listWidget->insertItem(0, item);
    ui->listWidget->setItemWidget(item,pUnit);
}

void AlarmPopDialog::openDialog(bool open)
{
    if(open)
    {
        show();
    }
    else
    {
        close();
    }
}

void AlarmPopDialog::apppendAlarmMessage(InviewAlarmMessage alarmMessage)
{
    AlarmUnitWidget::Param pa;
    InviewChannel* channel = g_SppClient->getChannelByUid(alarmMessage.get_uid());
    if(channel)
    {
        pa.devName = QString::fromStdString(channel->get_channel_name());
        pa.channelId = channel->get_id();
        pa.time = QString::fromStdString(alarmMessage.get_alarm_time()).mid(10);
        pa.type = (ENUM_INVIEW_ALARM_TYPE)alarmMessage.get_alarm_type_code();

        pa.uid = QString::fromStdString(alarmMessage.get_uid());
        pa.alarmTime = QString::fromStdString(alarmMessage.get_alarm_time());
        appendItem(pa);
    }
}

void AlarmPopDialog::on_pushButtonClear_clicked()
{
    for(int i = 0;i < ui->listWidget->count();i++)
    {
        QWidget* tmpWidget = ui->listWidget->itemWidget(ui->listWidget->item(i));
        if(NULL != tmpWidget)
        {
            tmpWidget->deleteLater();
        }

        QListWidgetItem* itemToDel = ui->listWidget->takeItem(i);
        if(NULL != itemToDel)
        {
            delete itemToDel;
            itemToDel = NULL;
        }
    }
    ui->listWidget->clear();
}
