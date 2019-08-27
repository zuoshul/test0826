#ifndef GISMAPTREEVIEW_H
#define GISMAPTREEVIEW_H

#include <QWidget>
#include <QContextMenuEvent>
#include <QAction>
#include "src/common/data.h"
#include "src/sppclient/sppclient.h"
#include "src/sppclient/sppclientmapmanager.h"
#include <QModelIndex>
#include "src/common/snmessagedialog.h"

namespace Ui {
class GisMapTreeView;
}

class GisMapTreeView : public QWidget
{
    Q_OBJECT

public:
    explicit GisMapTreeView(QWidget *parent = 0);

    ~GisMapTreeView();

    //根据mapCode，修改树当前选中的节
    void setCurrentMap(const QString& mapCode);

    void setRootAsCurrentMap();

    void switchMapState(MapState mapState);

    InviewLocalMap getOperatorMap(bool redirect = false);

    void getCurMapAndLastMap(InviewLocalMap &curMap,InviewLocalMap &lastMap);

protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif

public slots:
    void treeItemClicked(const Node node);

    void slotMapInfoChanged(InviewLocalMap map, InviewOperatorType operatorType);

private slots:
    void slotAddMap();

    void slotDelteMap();

    void slotRenameMap();

    void slotNameChanged(QModelIndex index,QString strNewName);

    void slotConfirmDeleteMap();

signals:
    void signalMapSelected(int mapId);

    void signalAddMap(QPointF pos);

    void signalDelMap(int mapId);

    void signalRenameMap(int mapId,QString strNewName);

private:
    void initMapTree();

    void addMapNodes(std::vector<InviewLocalMap>& maps);

    void deleteMapNodes(std::vector<InviewLocalMap>& maps);

    void initActions();

private:
    Ui::GisMapTreeView *ui;
    int m_nCurMapId;
    int m_lastMapId;
    QAction* m_objAddMap;
    QAction* m_objDeleteMap;
    QAction* m_objRename;
    SNMessageDialog *m_pDelMapDlg;
};

#endif // GISMAPTREEVIEW_H
