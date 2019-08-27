#ifndef GISADDFIRSTMAPWIDGET_H
#define GISADDFIRSTMAPWIDGET_H

#include <QWidget>
#include "src/base/basewidget.h"
#include "src/common/snmessagedialog.h"

namespace Ui {
class GisAddFirstMapWidget;
}

class GisAddFirstMapWidget : public BaseWidget
{
    Q_OBJECT

public:
    explicit GisAddFirstMapWidget(QWidget *parent = 0);
    ~GisAddFirstMapWidget();

    virtual void initForm();

    virtual void retranslateUi();

    virtual void doAfterWidgetShow();

    void showAddMapBtn(bool bShow);

signals:
    void signalAddButtonClicked(QPointF);

private slots:
    void on_pushButtonAdd_clicked();

private:
    Ui::GisAddFirstMapWidget *ui;
};

#endif // GISADDFIRSTMAPWIDGET_H
