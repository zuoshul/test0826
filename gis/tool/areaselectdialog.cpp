#include "areaselectdialog.h"
#include "ui_areaselectdialog.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

AreaSelectDialog::AreaSelectDialog(QWidget *parent) :
    SNDialog(parent),
    ui(new Ui::AreaSelectDialog)
{
    ui->setupUi(this);
    initForm();
}

AreaSelectDialog::~AreaSelectDialog()
{
    delete ui;
}

void AreaSelectDialog::initForm()
{
    hide();
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->listWidget->setFocusPolicy(Qt::NoFocus);
    ui->listWidget->setSelectionMode(QAbstractItemView::NoSelection);

    connect(ui->checkBoxSelectAll,SIGNAL(stateChanged(int)),this,SLOT(slotAllCheckedChanged(int)));
    AppStyleHelper::updateWidgetStyle(ui->pushButtonAlarmSearch, AppStyleHelper::Style_PushButtonDefault);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonPreview, AppStyleHelper::Style_PushButtonDefault);
    retranslateUi();
}

void AreaSelectDialog::retranslateUi()
{
    ui->pushButtonAlarmSearch->setText(QObject::tr("DB.InviewMenuPrivilege.MenuPrivilegeName.AlarmManage"));
    ui->pushButtonPreview->setText(QObject::tr("DB.InviewMenuPrivilege.MenuPrivilegeName.Preview"));
    ui->checkBoxSelectAll->setText(QObject::tr("TK_SelectAll"));
    ui->retranslateUi(this);
}

void AreaSelectDialog::doAfterWidgetShow()
{

}

void AreaSelectDialog::appendItem(AreaSelectUnitWidget::Param param)
{
    AreaSelectUnitWidget* pUnit = new AreaSelectUnitWidget(this);
    param.number = ui->listWidget->count() + 1;
    pUnit->setParam(param);
    QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(QSize(ui->listWidget->width() - 5,25));
    ui->listWidget->insertItem(0,item);
    ui->listWidget->setItemWidget(item,pUnit);
    connect(pUnit,SIGNAL(stateChanged(int)),this,SLOT(slotStateChanged(int)));
}

void AreaSelectDialog::setTitle(int number)
{
    if(number <= 0)
    {
        return;
    }
    QString title = tr("TK_SelectDev") + QString("-%1").arg(number);
    ui->labelTitle->setText(title);
}

void AreaSelectDialog::slotOpenDialog(QList<AreaSelectUnitWidget::Param> list)
{
    if(list.size() <= 0)
    {
        return;
    }

    ui->listWidget->clear();
    setTitle(list.size());
    disconnect(ui->checkBoxSelectAll,SIGNAL(stateChanged(int)),this,SLOT(slotAllCheckedChanged(int)));
    ui->checkBoxSelectAll->setChecked(true);
    connect(ui->checkBoxSelectAll,SIGNAL(stateChanged(int)),this,SLOT(slotAllCheckedChanged(int)));

    for(int i = 0 ;i < list.size();i++)
    {
        appendItem(list.at(i));
    }
    showSNDialog();
}

void AreaSelectDialog::on_pushButtonClose_clicked()
{
    close();
    emit recoveryDrag();
}

void AreaSelectDialog::closeDialog()
{
    on_pushButtonClose_clicked();
}

void AreaSelectDialog::on_pushButtonAlarmSearch_clicked()
{
    QList<int> channelList = getCheckedChannels();
    if(!channelList.isEmpty())
    {
        close();
        emit startAlarmSearch(channelList);
    }
}

void AreaSelectDialog::on_pushButtonPreview_clicked()
{
    QList<int> channelList = getCheckedChannels();
    if(!channelList.isEmpty())
    {
        close();
        emit startLiveview(channelList);
    }
}

QList<int> AreaSelectDialog::getCheckedChannels()
{
    QList<int> channelList;
    for(int i = 0 ;i < ui->listWidget->count();i++)
    {
        AreaSelectUnitWidget* pUnit = (AreaSelectUnitWidget*)ui->listWidget->itemWidget(ui->listWidget->item(i));
        bool checked = pUnit->isChecked();
        if(checked)
        {
            AreaSelectUnitWidget::Param pa = pUnit->getParam();
            int channelId = pa.channelId;
            channelList.append(channelId);
        }
    }
    return channelList;
}

void AreaSelectDialog::slotStateChanged(int state)
{
    disconnect(ui->checkBoxSelectAll,SIGNAL(stateChanged(int)),this,SLOT(slotAllCheckedChanged(int)));

    if(state == Qt::Unchecked)
    {
        ui->checkBoxSelectAll->setChecked(false);
    }
    else
    {
        bool allChecked = true;
        for(int i = 0 ;i < ui->listWidget->count();i++)
        {
            AreaSelectUnitWidget* pUnit = (AreaSelectUnitWidget*)ui->listWidget->itemWidget(ui->listWidget->item(i));
            bool checked = pUnit->isChecked();
            if(!checked)
            {
                allChecked = false;
                break;
            }
        }
        ui->checkBoxSelectAll->setChecked(allChecked);
    }
    connect(ui->checkBoxSelectAll,SIGNAL(stateChanged(int)),this,SLOT(slotAllCheckedChanged(int)));
}

void AreaSelectDialog::slotAllCheckedChanged(int state)
{
    for(int i = 0 ;i < ui->listWidget->count();i++)
    {
        AreaSelectUnitWidget* pUnit = (AreaSelectUnitWidget*)ui->listWidget->itemWidget(ui->listWidget->item(i));
        pUnit->setChecked(state == Qt::Checked);
    }
}
