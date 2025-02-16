#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <QObject>
#include "command_thread.h"
#include "active_data_thread.h"
#include "passive_data_thread.h"

class NetworkManager : public QObject
{
    Q_OBJECT
    friend class ServerController;
public:
    explicit NetworkManager(QObject *parent = nullptr);
    static bool isValidPort(const QString& port);
    void startServer(int port);
    void stopServer();
signals:
    void stopServerSignal();

private:
    CommandThread m_commandThread;

    PassiveDataThread m_passiveDataThread;
    ActiveDataThread m_activeDataThread;
};

#endif // NETWORK_MANAGER_H
