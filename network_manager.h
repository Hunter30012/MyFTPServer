#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <QObject>
#include "tcpserver_thread.h"
#include "tcpclient_thread.h"

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
    TcpServerThread m_commandThread;

    TcpServerThread m_passiveDataThread;
    TcpClientThread m_activeDataThread;
};

#endif // NETWORK_MANAGER_H
