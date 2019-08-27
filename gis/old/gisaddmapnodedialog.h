#ifndef GISADDMAPNODEDIALOG_H
#define GISADDMAPNODEDIALOG_H

#include <QWidget>
#include "src/common/sndialog.h"
#include "SNTreeView.h"

namespace Ui {
class GisAddMapNodeDialog;
}

class GisAddMapNodeDialog : public SNDialog
{
    Q_OBJECT

public:
    explicit GisAddMapNodeDialog(QWidget *parent = 0);
    ~GisAddMapNodeDialog();

    //子类用来初始化窗体内的UI组件
    virtual void initForm();

    //子类用来动态切客户端语言
    virtual void retranslateUi();

    virtual bool doVerifyLineEditText(QLineEdit* lineEdit, QString& toolTipStr);

    void openDialog();

private slots:
    void on_pushButtonConfirm_clicked();

    void on_pushButtonCancel_clicked();

    void on_pushButtonClose_clicked();

signals:
    void signalMapNodeSetFinished(QString,int);

private:
    bool verifyContent();

    void loadCombox();

private:
    Ui::GisAddMapNodeDialog *ui;
    SNTreeView* m_ptreeView;
};

#endif // GISADDMAPNODEDIALOG_H
