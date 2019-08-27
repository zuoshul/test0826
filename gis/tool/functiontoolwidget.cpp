#include "functiontoolwidget.h"
#include "ui_functiontoolwidget.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

FunctionToolWidget::FunctionToolWidget(QWidget *parent) :
    BaseWidget(parent),
    ui(new Ui::FunctionToolWidget)
{
    ui->setupUi(this);
    initForm();
}

FunctionToolWidget::~FunctionToolWidget()
{
    delete ui;
}

void FunctionToolWidget::initForm()
{
    AppStyleHelper::updateWidgetStyle(ui->pushButtonResources, AppStyleHelper::Style_PushButtonLight);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonScreen, AppStyleHelper::Style_PushButtonLight);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonSearch, AppStyleHelper::Style_PushButtonLight);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonSelect, AppStyleHelper::Style_PushButtonLight);
    retranslateUi();
}

void FunctionToolWidget::retranslateUi()
{
    //ui->labelTemp->setText(QObject::tr("TK_Temperature"));
    ui->retranslateUi(this);
}

void FunctionToolWidget::doAfterWidgetShow()
{

}

void FunctionToolWidget::on_pushButtonSelect_clicked()
{
    emit menuClicked(Select_Clicked);
}

void FunctionToolWidget::on_pushButtonResources_clicked()
{
    emit menuClicked(Source_Clicked);
}

void FunctionToolWidget::on_pushButtonSearch_clicked()
{
    emit menuClicked(Search_Clicked);
}

void FunctionToolWidget::on_pushButtonScreen_clicked()
{
    emit menuClicked(FullScreen_Clicked);
}
