#ifndef RESOURCESSETTINGDIALOG_H
#define RESOURCESSETTINGDIALOG_H

#include <QWidget>
#include "src/base/basewidget.h"
#include "src/common/snmessagedialog.h"
#include <QCheckBox>
#include <QSettings>

namespace Ui {
class ResourcesSettingDialog;
}

class ResourcesSettingDialog : public BaseWidget
{
    Q_OBJECT

public:
    struct ResourcesFilter
    {
        ResourcesFilter()
        {
            mapAlarm = true;
            showArea = true;
            online = true;
            offline = true;
            showList.clear();
        }
        bool mapAlarm;
        bool showArea;
        bool online;
        bool offline;
        QList<int> showList;
    };

public:
    explicit ResourcesSettingDialog(QWidget *parent = 0);
    ~ResourcesSettingDialog();

    virtual void initForm();

    virtual void retranslateUi();

    virtual void doAfterWidgetShow();

    void openDialog(bool open);

    void getMapConfig(ResourcesSettingDialog::ResourcesFilter &filter);

    void pushFilterRules();

private:
    void readMapConfig();

    void saveMapConfig(ResourcesSettingDialog::ResourcesFilter filter);

    bool isCheckedByType(int type);

    void resetFilterParam();

signals:
    void checkStateChanged(ResourcesSettingDialog::ResourcesFilter filter);

private slots:
    void on_pushButtonClose_clicked();

    void slotStateChanged(int state);

private:
    Ui::ResourcesSettingDialog *ui;
    ResourcesFilter m_filter;
    QList<QCheckBox*> m_devTypeList;
    QSettings* m_pSettings;
};

#endif // RESOURCESSETTINGDIALOG_H
