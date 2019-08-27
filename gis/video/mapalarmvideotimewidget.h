#ifndef MAPALARMVIDEOTIMEWIDGET_H
#define MAPALARMVIDEOTIMEWIDGET_H

#include <QWidget>
#include "src/common/sndialog.h"
#include <QDateTime>

//added by zuoshul 20190821

namespace Ui {
class MapAlarmVideoTimeWidget;
}

class MapAlarmVideoTimeWidget : public SNDialog
{
    Q_OBJECT

public:
    explicit MapAlarmVideoTimeWidget(QWidget *parent = 0);
    ~MapAlarmVideoTimeWidget();

    void initForm();

    void retranslateUi();

    void openDialog();

    bool openDialog(QDateTime start,QDateTime end);

    bool setTimeParam(QDateTime start,QDateTime end);

    void getTimeParam(QDateTime &start,QDateTime &end);

private slots:
    void on_pushButtonClose_clicked();

    void on_pushButtonOk_clicked();

    void on_pushButtonCancel_clicked();

    void slotAdjustDatetime(const QDateTime &datetime);

signals:
    void timeSelected(QDateTime start,QDateTime end);

private:
    void translateCalendar();

private:
    Ui::MapAlarmVideoTimeWidget *ui;
};

#endif // MAPALARMVIDEOTIMEWIDGET_H
