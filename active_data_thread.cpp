#include "active_data_thread.h"

ActiveDataThread::ActiveDataThread(QObject *parent)
    : QObject{parent}, m_socket{nullptr}
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

void ActiveDataThread::onStarted()
{
    QThread::currentThread()->setObjectName("ActiveData Thread");
    qDebug() << "After started: " << QThread::currentThread();
}

void ActiveDataThread::restartConnection(const QHostAddress &serverIp, int port, const QString& curDir)
{
    m_curDir = curDir;
    m_serverIp = serverIp;
    m_serverPort = port;

    m_socket = new QTcpSocket();
    qDebug() << "restartConnection: " << QThread::currentThread();
    qInfo() << "Trying to connect to " << m_serverIp.toString() << ":" << m_serverPort;
    connect(m_socket, &QTcpSocket::connected, this, &ActiveDataThread::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ActiveDataThread::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &ActiveDataThread::onError);
    m_socket->connectToHost(m_serverIp, m_serverPort);
    if (!m_socket->waitForConnected(5000)) {
        qWarning() << "Start active mode failed!";
        return;
    }
    emit writeTextSignal("Server is running Active Mode", Qt::darkBlue);
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
    qDebug() << "Send data in ActiveDataThread";
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        emit writeTextSignal("Send data to Server", Qt::darkBlue);
        m_socket->write(data);
        m_socket->flush();
    } else {
        emit writeTextSignal("Cannot send data, no active connection!", Qt::red);
    }
}

void ActiveDataThread::onReadyRead()
{
    qDebug() << "onReadyRead: " << QThread::currentThread();
    emit writeTextSignal("Recieved Data from Client", Qt::darkBlue);
    QByteArray data = m_socket->readAll();

    // need to push Queue
    emit dataReceivedSignal(data);
}

void ActiveDataThread::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Error:" << socketError << " " << m_socket->errorString();
    emit writeTextSignal(m_socket->errorString(), Qt::red);
}

void ActiveDataThread::onConnected()
{
    qDebug() << QThread::currentThread();
    qDebug() << "Connected from Data thread";
    emit writeTextSignal("Established connection!", Qt::darkBlue);
    QJsonArray serverResponse = FTPManager::createServerResponse(FTPManager::ResponseType::ActiveConnected , m_curDir);
    qDebug() << "Send test data!";
    this->sendData(DataConverter::JsonArrayToByteArray(serverResponse));
}
