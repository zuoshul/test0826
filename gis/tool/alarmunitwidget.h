#ifndef ALARMUNITWIDGET_H
#define ALARMUNITWIDGET_H

#include <QWidget>
#include "src/base/basewidget.h"
#include "src/common/snmessagedialog.h"
#include "src/sppclient/sppclientdevicemanager.h"
#include "formalarmvideowidget.h"

namespace Ui {
class AlarmUnitWidget;
}

class AlarmUnitWidget : public BaseWidget
{
    Q_OBJECT

public:
    struct Param
    {
        ENUM_INVIEW_ALARM_TYPE type;
        QString devName;
        QString time;
        int number;
        int channelId;

        QString uid;
        QString alarmTime; //原始时间
    };

public:
    explicit AlarmUnitWidget(QWidget *parent = 0);
    ~AlarmUnitWidget();

    virtual void initForm();

    virtual void retranslateUi();

    virtual void doAfterWidgetShow();

    void setParam(Param pa);

    Param getParam();

private slots:
    void on_pushButtonPlay_clicked();

private:
    Ui::AlarmUnitWidget *ui;
    Param m_param;
    FormAlarmVideoWidget* m_pObjAlarmVideoWidget;
};

#endif // ALARMUNITWIDGET_H
