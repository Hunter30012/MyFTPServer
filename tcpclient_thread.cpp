#include "tcpclient_thread.h"

TcpClientThread::TcpClientThread(QObject *parent)
    : QObject{parent}, m_socket{nullptr}, m_serverIp{""}, m_serverPort{0}
{
    connect(&m_thread, &QThread::started, this, &TcpClientThread::run);
    connect(this, &TcpClientThread::sendDataSignal, this, &TcpClientThread::sendData);
    this->moveToThread(&m_thread);
}

void TcpClientThread::startThread(const QString &serverIp, quint16 port)
{
    m_serverIp = serverIp;
    m_serverPort = port;
    m_thread.start();
}

void TcpClientThread::run()
{
    m_socket = new QTcpSocket();
    connect(m_socket, &QTcpSocket::connected, this, &TcpClientThread::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClientThread::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClientThread::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &TcpClientThread::onError);

    QThread::currentThread()->setObjectName("ActiveData Thread");
    qInfo() << "Client trying to connect to " << m_serverIp << ":" << m_serverPort;
    m_socket->connectToHost(m_serverIp, m_serverPort);

    if (!m_socket->waitForConnected(5000)) {
        qWarning() << "Connection failed!";
        emit errorOccurred("Connection Timeout!");
    }
}

void TcpClientThread::sendData(const QByteArray &data)
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(data);
    } else {
        qWarning() << "Cannot send data, no active connection!";
    }
}

void TcpClientThread::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    qDebug() << "Received from Server: " << data;
    // handle Data
    emit dataReceived(data);
}

void TcpClientThread::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    qWarning() << "Socket Error: " << m_socket->errorString();
    emit writeTextSignal(m_socket->errorString());
}

void TcpClientThread::quit()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    m_thread.quit();
    m_thread.wait();
}

void TcpClientThread::connected()
{
    qDebug() << QThread::currentThread();
    qDebug() << "Connected form data thread";
}
