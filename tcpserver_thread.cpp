#include "tcpserver_thread.h"

TcpServerThread::TcpServerThread(QObject *parent)
    : QObject{parent}, m_socket {nullptr}, m_server {nullptr}
{
    this->moveToThread(&m_thread);
    connect(&m_thread, &QThread::started, this, &TcpServerThread::run);
    connect(this, &TcpServerThread::sendDataSignal, this, &TcpServerThread::sendData);
}

void TcpServerThread::startThread(int port)
{
    m_port = port;
    if (!m_thread.isRunning()) {
        m_thread.start();
    }
}

void TcpServerThread::run()
{
    m_server = new QTcpServer();
    connect(m_server, &QTcpServer::newConnection, this, &TcpServerThread::onNewConnection);
    QThread::currentThread()->setObjectName("TcpServer Thread");
    qDebug() << QThread::currentThread();

    // listen on port
    const QHostAddress& localhost = QHostAddress(QHostAddress::LocalHost);
    QHostAddress serverAddress;
    const QList<QHostAddress> listAddress = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : listAddress) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            serverAddress = address;
            break;
        }
    }
    bool isListening = m_server->listen(serverAddress, m_port);
    if(isListening)
    {
        emit writeTextSignal("Server listening on address: " + m_server->serverAddress().toString() + " , port: " + QString::number(m_server->serverPort()), Qt::darkGreen);
    }
    else
    {
        emit writeTextSignal("Server failed to start.", Qt::darkRed);
    }
}

void TcpServerThread::quit()
{
    if(m_server) {
        m_server->close();
    }

    if (m_socket) {
        m_socket->disconnect();
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    qDebug() << QThread::currentThread();
    emit writeTextSignal("Stop listening!", Qt::darkBlue);
    emit disableStopSignal();
    m_thread.quit();
}

void TcpServerThread::onNewConnection()
{
    if(m_socket) {
        m_socket->disconnect();
        m_socket->close();
        emit writeTextSignal("Disconnect old client!", Qt::red);
        delete m_socket;
        m_socket = nullptr;
    }
    m_socket = m_server->nextPendingConnection();

    connect(m_socket, &QTcpSocket::disconnected, this, &TcpServerThread::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpServerThread::onReadyRead);
    qInfo() << "Connected";
    emit writeTextSignal("Connected!", Qt::darkBlue);
    emit enableStopSignal();

    // test
    const QString address("169.254.198.94");
    emit startActiveDataThread(address, m_port + 1);
}

void TcpServerThread::onReadyRead()
{
    if (m_socket) {
        QByteArray data = m_socket->readAll();
        qDebug() << "Received: " << data;
        sendData("Server Response: " + data);  // Response to Client
    }
}

void TcpServerThread::disconnected()
{
    qInfo() << "Disconnected";
    emit writeTextSignal("Disconected!", Qt::red);
    if (m_socket) {
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

void TcpServerThread::sendData(const QByteArray &data)
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(data);
    } else {
        qWarning() << "No active connection to write data.";
    }
}


