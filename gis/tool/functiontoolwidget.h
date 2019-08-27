#ifndef FUNCTIONTOOLWIDGET_H
#define FUNCTIONTOOLWIDGET_H

#include <QWidget>
#include "src/base/basewidget.h"
#include "src/common/snmessagedialog.h"

namespace Ui {
class FunctionToolWidget;
}

class FunctionToolWidget : public BaseWidget
{
    Q_OBJECT

public:
    enum ClickStatus
    {
        No_Clicked,
        Select_Clicked,
        FullScreen_Clicked,
        Source_Clicked,
        Search_Clicked
    };

public:
    explicit FunctionToolWidget(QWidget *parent = 0);
    ~FunctionToolWidget();

    virtual void initForm();

    virtual void retranslateUi();

    virtual void doAfterWidgetShow();

signals:
    void menuClicked(FunctionToolWidget::ClickStatus status);

private slots:
    void on_pushButtonSelect_clicked();

    void on_pushButtonResources_clicked();

    void on_pushButtonSearch_clicked();

    void on_pushButtonScreen_clicked();

private:
    Ui::FunctionToolWidget *ui;
};

#endif // FUNCTIONTOOLWIDGET_H
