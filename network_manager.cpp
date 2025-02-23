#include "network_manager.h"

NetworkManager::NetworkManager(QObject *parent)
    : QObject{parent}
{
    // connect to ActiveData Thread
    connect(&m_commandThread, &CommandThread::startActiveDataThreadSignal, &m_activeDataThread, &ActiveDataThread::startThread, Qt::QueuedConnection);
    connect(&m_commandThread, &CommandThread::stopActiveDataSignal, &m_activeDataThread, &ActiveDataThread::stopConnection, Qt::QueuedConnection);
    connect(&m_commandThread, &CommandThread::restartActiveDataSignal, &m_activeDataThread, &ActiveDataThread::restartConnection, Qt::QueuedConnection);

    connect(&m_commandThread, &CommandThread::connectedActiveSignal, &m_activeDataThread, &ActiveDataThread::onConnectedActive, Qt::QueuedConnection);

    //connect to PassiveDataThread
    connect(&m_commandThread, &CommandThread::startPassiveDataThreadSignal, &m_passiveDataThread, &PassiveDataThread::startThread, Qt::QueuedConnection);
    connect(&m_commandThread, &CommandThread::stopPassiveDataSignal, &m_passiveDataThread, &PassiveDataThread::stopListening, Qt::QueuedConnection);
    connect(&m_commandThread, &CommandThread::restartPassiveDataThreadSignal, &m_passiveDataThread, &PassiveDataThread::restartListening, Qt::QueuedConnection);
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

void NetworkManager::startServer(int port, const QString& dir)
{
    qDebug() << "Server Dir: " << dir;
    m_commandThread.startThread(port, dir);
}
void NetworkManager::stopServer()
{
    emit stopServerSignal();
}
