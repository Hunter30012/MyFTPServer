#include "server_controller.h"

ServerController::ServerController(int argc, char *argv[], QWidget *parent)
    : QObject{parent}, m_app {argc, argv}, m_window {parent}, m_model {parent}
{
    QList<bool> connectionResults;

    connectModelSignalSlots(connectionResults);
    connectWindowSignalSlots(connectionResults);

    Q_ASSERT(!connectionResults.contains(false));
}

int ServerController::init()
{

    m_window.show();
    return m_app.exec();
}

void ServerController::connectWindowSignalSlots(QList<bool> &connectionResults)
{
    connectionResults.append(connect(m_window.ui->clearButton, &QPushButton::clicked, &m_window, &ServerWindow::clearOutput));
    connectionResults.append(connect(m_window.ui->startButton, &QPushButton::clicked, &m_window, &ServerWindow::startServer));
    connectionResults.append(connect(&m_window, &ServerWindow::startServerSignal, &m_model, &ServerModel::startServer));
    connectionResults.append(connect(m_window.ui->stopButton, &QPushButton::clicked, &m_model, &ServerModel::stopServer));
}

void ServerController::connectModelSignalSlots(QList<bool> &connectionResults)
{
    /**
     * Network Manager
     */
    connectionResults.append(connect(&m_model.m_networkManager, &NetworkManager::stopServerSignal, &m_model.m_networkManager.m_commandThread, &CommandThread::stopListening));
    // command - TCPServer
    connectionResults.append(connect(&m_model.m_networkManager.m_commandThread, &CommandThread::writeTextSignal, &m_window, &ServerWindow::writeTextToOutput));
    connectionResults.append(connect(&m_model.m_networkManager.m_commandThread, &CommandThread::enableStopSignal, &m_window, &ServerWindow::enableStop));
    connectionResults.append(connect(&m_model.m_networkManager.m_commandThread, &CommandThread::disableStopSignal, &m_window, &ServerWindow::disableStop));
    //

    connectionResults.append(connect(&m_model.m_networkManager.m_activeDataThread, &ActiveDataThread::writeTextSignal, &m_window, &ServerWindow::writeTextToOutput));
    connectionResults.append(connect(&m_model.m_networkManager.m_passiveDataThread, &PassiveDataThread::writeTextSignal, &m_window, &ServerWindow::writeTextToOutput));
    connectionResults.append(connect(&m_model.m_networkManager, &NetworkManager::writeTextSignal, &m_window, &ServerWindow::writeTextToOutput));
}
