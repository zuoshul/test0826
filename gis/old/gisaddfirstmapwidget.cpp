#include "gisaddfirstmapwidget.h"
#include "ui_gisaddfirstmapwidget.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

GisAddFirstMapWidget::GisAddFirstMapWidget(QWidget *parent) :
    BaseWidget(parent),
    ui(new Ui::GisAddFirstMapWidget)
{
    ui->setupUi(this);
    hide();
    initForm();
}

GisAddFirstMapWidget::~GisAddFirstMapWidget()
{
    delete ui;
}

void GisAddFirstMapWidget::initForm()
{
    QPixmap pix(":/images/default/layoutmanage/null.png");
    setFixedSize(pix.width(),pix.height());
    ui->pushButtonAdd->setStyleSheet("QPushButton{border:none;background-image: url(:/images/default/layoutmanage/null.png);background-color:transparent} \
                                     QPushButton::hover{border:none;background-image: url(:/images/default/layoutmanage/null_hover.png);background-color:transparent;}");
    retranslateUi();
}

void GisAddFirstMapWidget::retranslateUi()
{
    ui->retranslateUi(this);
}

void GisAddFirstMapWidget::doAfterWidgetShow()
{

}

void GisAddFirstMapWidget::showAddMapBtn(bool bShow)
{
    if(bShow)
    {
        show();
    }
    else
    {
        hide();
    }
}

void GisAddFirstMapWidget::on_pushButtonAdd_clicked()
{
    emit(signalAddButtonClicked(QPointF(0,0)));
}
