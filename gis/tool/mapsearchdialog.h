#ifndef MAPSEARCHDIALOG_H
#define MAPSEARCHDIALOG_H

#include <QWidget>
#include "src/base/basewidget.h"
#include "src/common/snmessagedialog.h"

namespace Ui {
class MapSearchDialog;
}

class MapSearchDialog : public BaseWidget
{
    Q_OBJECT

public:
    explicit MapSearchDialog(QWidget *parent = 0);
    ~MapSearchDialog();

    virtual void initForm();

    virtual void retranslateUi();

    virtual void doAfterWidgetShow();

    void location(int lon,int lat,int zoom);

    void resetItems(QStringList areaList);

    void openDialog(bool open,QPointF point = QPointF(),int zoomLevel = 4);

private slots:
    void handleNetworkRequestFinished();

private:
    Ui::MapSearchDialog *ui;
};

#endif // MAPSEARCHDIALOG_H
