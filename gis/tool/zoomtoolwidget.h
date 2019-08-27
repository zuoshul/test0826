#ifndef ZOOMTOOLWIDGET_H
#define ZOOMTOOLWIDGET_H

#include <QWidget>
#include "src/base/basewidget.h"
#include "src/gis/old/gismapgraphwidget.h"

namespace Ui {
class ZoomToolWidget;
}

class ZoomToolWidget : public BaseWidget
{
    Q_OBJECT

public:
    explicit ZoomToolWidget(QWidget *parent = 0);
    ~ZoomToolWidget();

    virtual void initForm();

    virtual void retranslateUi();

    virtual void doAfterWidgetShow();

    void setMode(MapMode mode);

signals:
    void zoomIn();

    void zoomOut();

private slots:
    void on_pushButtonZoomIn_clicked();

    void on_pushButtonZoomOut_clicked();

    void slotPercentChanged(double percent);

private:
    Ui::ZoomToolWidget *ui;
};

#endif // ZOOMTOOLWIDGET_H
