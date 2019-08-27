#include "areaselectunitwidget.h"
#include "ui_areaselectunitwidget.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

AreaSelectUnitWidget::AreaSelectUnitWidget(QWidget *parent) :
    BaseWidget(parent),
    ui(new Ui::AreaSelectUnitWidget)
{
    ui->setupUi(this);
    initForm();
}

AreaSelectUnitWidget::~AreaSelectUnitWidget()
{
    delete ui;
}

void AreaSelectUnitWidget::initForm()
{
    ui->checkBoxNum->setChecked(true);
    connect(ui->checkBoxNum,SIGNAL(stateChanged(int)),this,SIGNAL(stateChanged(int)));
    retranslateUi();
}

void AreaSelectUnitWidget::retranslateUi()
{
   //ui->labelTemp->setText(QObject::tr("TK_Temperature"));
    ui->retranslateUi(this);
}

void AreaSelectUnitWidget::doAfterWidgetShow()
{

}

bool AreaSelectUnitWidget::isChecked()
{
    return ui->checkBoxNum->checkState() == Qt::Checked ? true : false;
}

void AreaSelectUnitWidget::setChecked(bool checked)
{
    ui->checkBoxNum->setChecked(checked);
}

void AreaSelectUnitWidget::setParam(Param pa)
{
    m_param = pa;

    QString path = g_SppClient->getIconPath((NodeType)pa.type,INVIEW_DEVICE_STATE_ONLINE);
    QPixmap pix(path);
    ui->labelPic->setPixmap(pix);
    ui->labelDevName->setText(pa.devName);
    ui->labelDevRecorderStatus->setText(pa.recorderStatus);
    ui->checkBoxNum->setText(QString::number(pa.number));
}

AreaSelectUnitWidget::Param AreaSelectUnitWidget::getParam()
{
    return m_param;
}
