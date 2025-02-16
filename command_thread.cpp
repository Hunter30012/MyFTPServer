#include "command_thread.h"

CommandThread::CommandThread(QObject *parent)
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

    this->moveToThread(&m_thread);
    connect(&m_thread, &QThread::started, this, &CommandThread::onStarted);
    connect(this, &CommandThread::sendDataSignal, this, &CommandThread::sendData);
}

CommandThread::~CommandThread()
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

void CommandThread::startThread(int port)
{
    m_port = port;
    qDebug() << "On startThread";

    if (!m_thread.isRunning()) {
        m_thread.start();
    } else {
        qDebug() << "Thread is running";
        QMetaObject::invokeMethod(this, "onStarted", Qt::QueuedConnection);
    }
}

void CommandThread::onStarted()
{
    if (!m_server) {
        m_server = new QTcpServer();
        m_server->moveToThread(QThread::currentThread());
        connect(m_server, &QTcpServer::newConnection, this, &CommandThread::onNewConnection);
    } else {
        m_server->close();
    }

    QThread::currentThread()->setObjectName("Command Thread");
    qDebug() << QThread::currentThread();

    // listen on port
    bool isListening = m_server->listen(m_address, m_port);
    if(isListening) {
        emit writeTextSignal("Server listening on address: " + m_server->serverAddress().toString() +
                                 " , port: " + QString::number(m_server->serverPort()), Qt::darkGreen);
    } else {
        emit writeTextSignal("Server failed to start.", Qt::darkRed);
    }
    // if Active mode
    emit startActiveDataThreadSignal();
    // if Passive mode
    emit startPassiveDataThreadSignal();
}

void CommandThread::onNewConnection()
{
    if(m_socket) {
        m_socket->disconnect();
        m_socket->close();
        emit writeTextSignal("Disconnect old client!", Qt::red);
        delete m_socket;
        m_socket = nullptr;
    }

    m_socket = m_server->nextPendingConnection();
    connect(m_socket, &QTcpSocket::disconnected, this, &CommandThread::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &CommandThread::onReadyRead);
    qInfo() << "New client connected!";

    int clientPort = m_socket->peerPort();
    emit writeTextSignal("Connected connected from IP: " + m_socket->peerAddress().toString() +  ":" + QString::number(clientPort), Qt::darkBlue);
    emit enableStopSignal();

    // test  - addr va port cua Client de ket noi toi
    // QHostAddress addr_test("169.254.198.94");
    // emit restartActiveDataSignal(addr_test, 5051);

    // passive
    emit restartPassiveDataThreadSignal(5050);
}

void CommandThread::sendData(const QByteArray &data)
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(data);
    } else {
        qWarning() << "No active connection to write data.";
    }
}

void CommandThread::onReadyRead()
{
    if (m_socket) {
        QByteArray data = m_socket->readAll();
        qDebug() << "Received: " << data;
        sendData("Server Response: " + data);  // Response to Client
    }
}

void CommandThread::stopListening()
{
    // active
    // emit stopActiveDataSignal();
    // passive
    emit stopPassiveDataSignal();
    qDebug() << "Stop Listening in Command Thread";
    if (m_server) {
        m_server->close();
    }
    if (m_socket) {
        disconnect(m_socket, nullptr, this, nullptr);
        emit writeTextSignal("Disconected!", Qt::red);
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }

    qDebug() << QThread::currentThread();
    emit writeTextSignal("Stop listening!", Qt::darkBlue);
    emit disableStopSignal();
}

void CommandThread::disconnected()
{
    // active
    // emit stopActiveDataSignal();
    //passive
    emit stopPassiveDataSignal();
    qInfo() << "CommandThread Disconnected";
    emit writeTextSignal("Disconected!", Qt::red);
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}



