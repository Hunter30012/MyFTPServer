#include "active_data_thread.h"

ActiveDataThread::ActiveDataThread(QObject *parent)
    : QObject{parent}, m_socket{nullptr}, m_serverIp{""}, m_serverPort{0}
{
    connect(this, &ActiveDataThread::sendDataSignal, this, &ActiveDataThread::sendData);
}

ActiveDataThread::~ActiveDataThread()
{
    if (m_socket) {
        disconnect(m_socket, nullptr, this, nullptr);
        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(3000);
        }
        delete m_socket;
        m_socket = nullptr;
    }
    m_thread.quit();
    m_thread.wait();
}

void ActiveDataThread::startThread()
{
    qDebug() << "Start Active Data Thread";
    if (!m_thread.isRunning()) {
        this->moveToThread(&m_thread);
        connect(&m_thread, &QThread::started, this, &ActiveDataThread::onStarted);
        m_thread.start();
    }
}

void ActiveDataThread::restartConnection(const QHostAddress &serverIp, int port)
{
    m_serverIp = serverIp;
    m_serverPort = port;

    m_socket = new QTcpSocket();
    qDebug() << "restartConnection: " << QThread::currentThread();
    qInfo() << "Trying to connect to " << m_serverIp.toString() << ":" << m_serverPort;
    m_socket->connectToHost(m_serverIp, m_serverPort);

    if (!m_socket->waitForConnected(5000)) {
        qWarning() << "Connection failed!";
        emit errorOccurred("Connection Timeout!");

    }
    connect(m_socket, &QTcpSocket::connected, this, &ActiveDataThread::connected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ActiveDataThread::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &ActiveDataThread::onError);
}

void ActiveDataThread::onStarted()
{
    QThread::currentThread()->setObjectName("ActiveData Thread");
    qDebug() << "After started: " << QThread::currentThread();
}

void ActiveDataThread::stopConnection()
{
    qDebug() << "Stopping connection in: " << QThread::currentThread();

    if (m_socket) {
        disconnect(m_socket, nullptr, this, nullptr);
        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(3000);
        }
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

void ActiveDataThread::sendData(const QByteArray &data)
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(data);
    } else {
        qWarning() << "Cannot send data, no active connection!";
    }
}

void ActiveDataThread::onReadyRead()
{
    qDebug() << "onReadyRead: " << QThread::currentThread();
    QByteArray data = m_socket->readAll();
    qDebug() << "Received from Server: " << data;
    // handle Data
    emit dataReceived(data);
}

void ActiveDataThread::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    qWarning() << "Socket Error: " << m_socket->errorString();
    emit writeTextSignal(m_socket->errorString());
}

void ActiveDataThread::connected()
{
    qDebug() << QThread::currentThread();
    qDebug() << "Connected from Data thread";
}
