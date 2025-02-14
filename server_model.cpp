#include "server_model.h"

ServerModel::ServerModel(QObject *parent)
    : QObject{parent}
{}

void ServerModel::startServer(int port)
{
    m_networkManager.startServer(port);
}

void ServerModel::stopServer()
{
    m_networkManager.stopServer();
}
