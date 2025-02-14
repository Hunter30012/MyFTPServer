#include "network_manager.h"

NetworkManager::NetworkManager(QObject *parent)
    : QObject{parent}
{
    connect(&m_commandThread, &TcpServerThread::startActiveDataThread, &m_activeDataThread, &TcpClientThread::startThread);
}

bool NetworkManager::isValidPort(const QString &port)
{
    if (port.isEmpty()) {
        return false;
    }

    bool isNumber;
    int portNumber = port.toInt(&isNumber);

    return isNumber && (portNumber >= 1 && portNumber <= 65535);
}

void NetworkManager::startServer(int port)
{
    m_commandThread.startThread(port);
}

void NetworkManager::stopServer()
{
    emit stopServerSignal();
}
