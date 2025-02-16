#include "passive_data_thread.h"

PassiveDataThread::PassiveDataThread(QObject *parent)
    : QObject{parent}, m_socket {nullptr}, m_server {nullptr}
{
    const QHostAddress& localhost = QHostAddress(QHostAddress::LocalHost);
    const QList<QHostAddress> listAddress = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : listAddress) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            m_address = address;
            break;
        }
    }
    connect(this, &PassiveDataThread::sendDataSignal, this, &PassiveDataThread::sendData);
}

PassiveDataThread::~PassiveDataThread()
{
    if (m_socket) {
        delete m_socket;
        m_socket = nullptr;
    }

    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }

    if (m_thread.isRunning()) {
        m_thread.quit();
        m_thread.wait();
    }
}

void PassiveDataThread::startThread()
{
    qDebug() << "Start Passive Data Thread";
    if (!m_thread.isRunning()) {
        this->moveToThread(&m_thread);
        connect(&m_thread, &QThread::started, this, &PassiveDataThread::onStarted);
        m_thread.start();
    }
}

void PassiveDataThread::onStarted()
{
    QThread::currentThread()->setObjectName("PassiveData Thread");
    qDebug() << "After started: " << QThread::currentThread();
}

void PassiveDataThread::onNewConnection()
{
    if(m_socket) {
        m_socket->disconnect();
        m_socket->close();
        delete m_socket;
        m_socket = nullptr;
    }

    m_socket = m_server->nextPendingConnection();
    connect(m_socket, &QTcpSocket::disconnected, this, &PassiveDataThread::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &PassiveDataThread::onReadyRead);
    qInfo() << "Passive Data Socket connected!";
}

void PassiveDataThread::restartListening(int port)
{
    m_port = port;
    if (!m_server) {
        m_server = new QTcpServer();
        m_server->moveToThread(QThread::currentThread());
        connect(m_server, &QTcpServer::newConnection, this, &PassiveDataThread::onNewConnection);
    } else {
        m_server->close();
    }

    qDebug() << "Restart listening in :" << QThread::currentThread();

    // listen on port
    bool isListening = m_server->listen(m_address, m_port);
    if(isListening) {
        qDebug() << "PassiveDataThread listening on address: " << m_server->serverAddress().toString()
                 << " , port: " << QString::number(m_server->serverPort());
    } else {
        qDebug() << "PassiveDataThread failed to start.";
    }
}

void PassiveDataThread::sendData(const QByteArray &data)
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(data);
    } else {
        qWarning() << "No active connection to write data.";
    }
}

void PassiveDataThread::onReadyRead()
{
    if (m_socket) {
        QByteArray data = m_socket->readAll();
        qDebug() << "Passive Data received: " << data;
        sendData("Passive Data response: " + data);  // Response to Client
    }
}

void PassiveDataThread::stopListening()
{
    qDebug() << "Stop Listening in PassiveDataThread";
    if (m_server) {
        m_server->close();
    }
    if (m_socket) {
        disconnect(m_socket, nullptr, this, nullptr);
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

void PassiveDataThread::disconnected()
{
    qInfo() << "PassiveDataThread Disconnected";
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}






