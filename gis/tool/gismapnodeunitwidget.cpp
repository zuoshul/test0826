#include "gismapnodeunitwidget.h"
#include "ui_gismapnodeunitwidget.h"

//GIS地图功能比较复杂，在对本模块框架代码完全熟悉之前，不要擅自修改，否则会留下潜在风险 zuoshul20190715

GisMapNodeUnitWidget::GisMapNodeUnitWidget(QWidget *parent) :
    BaseWidget(parent),
    ui(new Ui::GisMapNodeUnitWidget),
    m_point(QPointF(114,31)),
    m_zoomLevel(4)
{
    ui->setupUi(this);
    ui->labelPic->setPixmap(QPixmap(":images/default/map/map_center.png"));
    setStyleSheet("QWidget{border:none;background-color:transparent}");
    ui->labelPos->setText("(233 , 996)");
    ui->labelPos->setStyleSheet("QLabel{color:red}");
    hide();
}

GisMapNodeUnitWidget::~GisMapNodeUnitWidget()
{
    delete ui;
}

void GisMapNodeUnitWidget::openDialog(bool bShow)
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

void GisMapNodeUnitWidget::getPointAndZoomlevel(QPointF &point,int &zoomLevel)
{
    point = m_point;
    zoomLevel = m_zoomLevel;
}

void GisMapNodeUnitWidget::slotMapCenterPointChanged(QPointF point,int zoomLevel)
{
    m_point = point;
    m_zoomLevel = zoomLevel;
    QString lon = QString::number(point.x(),'f',4);
    QString lat = QString::number(point.y(),'f',4);
    QString temp = QString("%1 , %2").arg(lon).arg(lat);
    ui->labelPos->setText(temp);
}
