#ifndef AREASELECTUNITWIDGET_H
#define AREASELECTUNITWIDGET_H

#include <QWidget>
#include "src/base/basewidget.h"
#include "src/common/snmessagedialog.h"
#include "src/sppclient/sppclientdevicemanager.h"

namespace Ui {
class AreaSelectUnitWidget;
}

class AreaSelectUnitWidget : public BaseWidget
{
    Q_OBJECT

public:
    struct Param
    {
        ENUM_INVIEW_DEVICE_TYPE type;
        QString devName;
        QString recorderStatus;
        int number;
        int channelId;
    };

    bool isChecked();

    void setChecked(bool checked);

public:
    explicit AreaSelectUnitWidget(QWidget *parent = 0);
    ~AreaSelectUnitWidget();

    virtual void initForm();

    virtual void retranslateUi();

    virtual void doAfterWidgetShow();

    void setParam(Param pa);

    Param getParam();

signals:
    void stateChanged(int);

private:
    Ui::AreaSelectUnitWidget *ui;
    Param m_param;
};

#endif // AREASELECTUNITWIDGET_H
