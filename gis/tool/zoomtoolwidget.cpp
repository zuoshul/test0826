#include "zoomtoolwidget.h"
#include "ui_zoomtoolwidget.h"
#include <QGraphicsDropShadowEffect>

ZoomToolWidget::ZoomToolWidget(QWidget *parent) :
    BaseWidget(parent),
    ui(new Ui::ZoomToolWidget)
{
    ui->setupUi(this);
    initForm();
}

ZoomToolWidget::~ZoomToolWidget()
{
    delete ui;
}

void ZoomToolWidget::initForm()
{
    setFixedSize(35,100);
    setStyleSheet("background-color:rgba(67, 67, 68, 96%);border-radius:3px;padding:2px 4px;");
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(-1, 1);              //阴影的偏移量
    shadow_effect->setColor(QColor(67, 67, 68)); //阴影的颜色
    shadow_effect->setBlurRadius(8);
    setGraphicsEffect(shadow_effect);

    ui->labelPercent->setFixedSize(29,20);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonZoomIn,AppStyleHelper::Style_PushButtonLight);
    AppStyleHelper::updateWidgetStyle(ui->pushButtonZoomOut,AppStyleHelper::Style_PushButtonLight);

    ui->line->setFrameShape(QFrame::HLine);
    ui->line->setFrameShadow(QFrame::Sunken);
    AppStyleHelper::updateWidgetStyle(ui->line,AppStyleHelper::Style_Frame_CutLine);
    ui->line_2->setFrameShape(QFrame::HLine);
    ui->line_2->setFrameShadow(QFrame::Sunken);
    AppStyleHelper::updateWidgetStyle(ui->line_2,AppStyleHelper::Style_Frame_CutLine);

    retranslateUi();
}

void ZoomToolWidget::retranslateUi()
{
    ui->retranslateUi(this);
}

void ZoomToolWidget::doAfterWidgetShow()
{

}

void ZoomToolWidget::on_pushButtonZoomIn_clicked()
{
    emit zoomIn();
}

void ZoomToolWidget::on_pushButtonZoomOut_clicked()
{
    emit zoomOut();
}

void ZoomToolWidget::setMode(MapMode mode)
{
    ui->line_2->setVisible(mode != GisMap);
    ui->line->setVisible(mode != GisMap);
    ui->labelPercent->setVisible(mode != GisMap);
}

void ZoomToolWidget::slotPercentChanged(double percent)
{
    ui->labelPercent->setText(QString::number(percent,'f',0));
}
