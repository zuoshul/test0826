#ifndef AREASELECTDIALOG_H
#define AREASELECTDIALOG_H

#include <QWidget>
#include "src/base/basewidget.h"
#include "src/common/snmessagedialog.h"
#include "areaselectunitwidget.h"

namespace Ui {
class AreaSelectDialog;
}

class AreaSelectDialog : public SNDialog
{
    Q_OBJECT

public:
    explicit AreaSelectDialog(QWidget *parent = 0);
    ~AreaSelectDialog();

    virtual void initForm();

    virtual void retranslateUi();

    virtual void doAfterWidgetShow();

    void setTitle(int number);

    void closeDialog();

signals:
    void startLiveview(QList<int> channelList);

    void recoveryDrag();

    void startAlarmSearch(QList<int> channelList);

private slots:
    void on_pushButtonClose_clicked();

    void on_pushButtonAlarmSearch_clicked();

    void on_pushButtonPreview_clicked();

    void slotStateChanged(int state);

    void slotAllCheckedChanged(int state);

    void slotOpenDialog(QList<AreaSelectUnitWidget::Param> list);

private:
    void appendItem(AreaSelectUnitWidget::Param param);

    QList<int> getCheckedChannels();

private:
    Ui::AreaSelectDialog *ui;
};

#endif // AREASELECTDIALOG_H
