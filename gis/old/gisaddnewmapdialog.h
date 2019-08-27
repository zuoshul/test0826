#ifndef GISADDNEWMAPDIALOG_H
#define GISADDNEWMAPDIALOG_H

#include <QWidget>
#include "src/common/sndialog.h"
#include "zipandunzip.h"
#include "devprogressdialog.h"
#include <QThread>

#define  LOCAL_ADD_HEIGHT            280
#define  FIRST_ADD_HEIGHT            350
#define  FIRSTANDOFFLINE_ADD_HEIGHT  430

namespace Ui {
class GisAddNewMapDialog;
}

const QString MAPGRAPHICS_CACHE_FOLDER_NAME = ".MapGraphicsCache";

class GisAddNewMapDialog : public SNDialog
{
    Q_OBJECT

public:
    explicit GisAddNewMapDialog(QWidget *parent = 0);
    ~GisAddNewMapDialog();

    //子类用来初始化窗体内的UI组件
    virtual void initForm();

    //子类用来动态切客户端语言
    virtual void retranslateUi();

    virtual bool doVerifyLineEditText(QLineEdit* lineEdit, QString& toolTipStr);

    void openDialog(bool firstMap);

private slots:
    void on_pushButtonClose_clicked();

    void on_pushButtonConfirm_clicked();

    void on_pushButtonCancel_clicked();

    void on_pushButtonSelectMap_clicked();

    void slotMapTypeChanged(int index);

    void slotGisMapLoadTypeChanged(int index);

    void on_pushButtonSelectOfflinePath_clicked();

    void unzipFinished(bool ret);

signals:
    void signalMapSelectFinished(QString mapName, QString mapPath);

    void gisMapModeSelected();

private:
    bool verifyContent();

    void initCustomStyle();

    void addFirstMap(bool first);

    void showGisMode(bool gis);

    void showOfflineGisMap(bool show);

    void resetHeight();

    void unzipTiles(QString sourcePath,QString newPath);

private:
    Ui::GisAddNewMapDialog *ui;
    int m_height;
    bool m_addFirstMap;
    QThread *m_pThread;
    ZipAndUnzip *m_pZipObj;
    DevProgressDialog *m_processDialog;
};

#endif // GISADDNEWMAPDIALOG_H
