#ifndef GISMAPLIVEVIDEOSUBWIN_H
#define GISMAPLIVEVIDEOSUBWIN_H
#include "src/liveview/liveviewvideosubwin.h"
#include "src/sppclient/sppclient.h"

class GisMapLiveVideoSubWin: public LiveViewVideoSubWin
{
    Q_OBJECT
public:
    GisMapLiveVideoSubWin(QWidget *parent = 0);
    virtual ~GisMapLiveVideoSubWin(){}

    virtual void initForm();
    virtual void retranslateUi();

public slots:
    void slot_close();
    void slot_closeAll();

private:
    QMenu* initContextMenu();

private:
    QMenu* m_pContextMenu; //右键菜单
    QMenu* m_pWindowRatioMenu;
    QAction* m_pDefultRatioAction;
    QAction* m_pStrentchAction;
    QAction* m_pCloseAllAction;
    QAction* m_pCloseAction;
    QAction* m_pRestoreAction;
    QAction* m_pHardwareAcceleration;
};

#endif // GISMAPLIVEVIDEOSUBWIN_H
