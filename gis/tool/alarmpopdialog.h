#ifndef ALARMPOPDIALOG_H
#define ALARMPOPDIALOG_H

#include <QWidget>
#include "src/base/basewidget.h"
#include "src/common/snmessagedialog.h"
#include "alarmunitwidget.h"
#include "resourcessettingdialog.h"

namespace Ui {
class AlarmPopDialog;
}

class AlarmPopDialog : public BaseWidget
{
    Q_OBJECT

public:
    explicit AlarmPopDialog(QWidget *parent = 0);
    ~AlarmPopDialog();

    virtual void initForm();

    virtual void retranslateUi();

    virtual void doAfterWidgetShow();

    void openDialog(bool open);

    void apppendAlarmMessage(InviewAlarmMessage alarmMessage);

signals:

private slots:

    void on_pushButtonClear_clicked();

private:
    void appendItem(AlarmUnitWidget::Param param);

private:
    Ui::AlarmPopDialog *ui;
};

#endif // ALARMPOPDIALOG_H
