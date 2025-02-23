#include "server_model.h"

ServerModel::ServerModel(QObject *parent)
    : QObject{parent}
{}

void ServerModel::startServer(int port, const QString& dir)
{
    m_curDir = dir;
    m_networkManager.startServer(port, dir);
}

void ServerModel::stopServer()
{
    m_networkManager.stopServer();
}
