#ifndef DEVSTATECHANGESPECIALPRO_H
#define DEVSTATECHANGESPECIALPRO_H
#include <QObject>
#include <QTimer>
#include "sppclientdevicemanager.h"

class DevStateChangeSpecialPro : public QObject
{
    Q_OBJECT

public:
    explicit DevStateChangeSpecialPro(QWidget *parent = 0);

    ~DevStateChangeSpecialPro();

private slots:
    void slot_signalDeviceInfoChanged(InviewDevice device, InviewOperatorType operatorType);

    void slotTimeout();
signals:
    void signalDeviceInfoChanged();

private:
    QTimer* m_pTimer;
};

#endif // DEVSTATECHANGESPECIALPRO_H
