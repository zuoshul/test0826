#ifndef GISMAPNODEUNITWIDGET_H
#define GISMAPNODEUNITWIDGET_H

#include <QWidget>
#include "basewidget.h"

namespace Ui {
class GisMapNodeUnitWidget;
}

class GisMapNodeUnitWidget : public BaseWidget
{
    Q_OBJECT

public:
    explicit GisMapNodeUnitWidget(QWidget *parent = 0);
    ~GisMapNodeUnitWidget();

    void openDialog(bool bShow);

    void getPointAndZoomlevel(QPointF &point,int &zoomLevel);

private slots:
    void slotMapCenterPointChanged(QPointF point,int zoomLevel);

private:
    Ui::GisMapNodeUnitWidget *ui;
    QPointF m_point;
    int m_zoomLevel;
};

#endif // GISMAPNODEUNITWIDGET_H
