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

void CommandThread::startThread(int port, const QString& dir)
{
    m_curDir = dir;
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
    // Active mode
    emit startActiveDataThreadSignal();
    // Passive mode
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
    emit writeTextSignal("Connected from IP: " + m_socket->peerAddress().toString() +  ":" + QString::number(clientPort), Qt::darkBlue);
    emit enableStopSignal();
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
        emit writeTextSignal("Recieve command from Client", Qt::darkBlue);
        qDebug() << "Server receive data form client" << QThread::currentThread();
        QByteArray data = m_socket->readAll();

        if(FTPManager::checkIfDataIsJson(data)) {
            parseRequest(data);
        } else {
            emit writeTextSignal("Request from Clinet - Invalid Json format!", Qt::red);
        }
    }
}

void CommandThread::parseRequest(const QByteArray &requestData)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(requestData);
    QJsonObject request = jsonDoc.object();

    if (!request.contains("request_type")) {
        qDebug() << "Invalid request: Missing request_type!";
        return;
    }

    qDebug() << "Received JSON:" << QJsonDocument(request).toJson(QJsonDocument::Compact);

    int requestType = request["request_type"].toInt();
    qDebug() << "Received request type:" << requestType;

    switch (requestType) {
    case 0: // ActiveConnect
        if (request.contains("port_data_thread" ) && request.contains("address_data_thread")) {
            m_isActiveMode = true;
            int portDataThread = request["port_data_thread"].toString().toInt();
            QHostAddress addressDataThread(request["address_data_thread"].toString());
            emit restartActiveDataSignal(addressDataThread, portDataThread, m_curDir);
        }
        break;
    case 1: // PassiveConnect
        m_isActiveMode = false;
        qDebug() << "PassiveConnect request received.";
        emit restartPassiveDataThreadSignal(m_port + 1, m_curDir);
        break;
    case 2: // ChangeDir
        if (request.contains("requestPath")) {
            QString requestPath = request["requestPath"].toString();
            qDebug() << "ChangeDir - Path:" << requestPath;
            m_curDir = requestPath;
            QJsonArray serverResponse = FTPManager::createServerResponse(FTPManager::ResponseType::ChangedDir , requestPath);
            if(m_isActiveMode) {
                emit sendActiveDataSignal(DataConverter::JsonArrayToByteArray(serverResponse));
            } else {
                emit sendPassiveDataSignal(DataConverter::JsonArrayToByteArray(serverResponse));
            }
        }
        break;
    case 3: // Delete
        if (request.contains("filesDelete")) {
            QJsonArray filesArray = request["filesDelete"].toArray();
            QStringList filesToDelete;

            for (int i = 0; i < filesArray.size(); i++) {
                if (filesArray[i].isString()) {
                    filesToDelete.append(filesArray[i].toString());
                }
            }
            bool isDeleteAll = FTPManager::deleteFiles(filesToDelete);
            QJsonArray serverResponse;
            if(isDeleteAll) {
                serverResponse = FTPManager::createServerResponse(FTPManager::ResponseType::Deleted, m_curDir);
            } else {
                serverResponse = FTPManager::createServerResponse(FTPManager::ResponseType::UnDeleted, m_curDir);
            }

            if(m_isActiveMode) {
                emit sendActiveDataSignal(DataConverter::JsonArrayToByteArray(serverResponse));
            } else {
                emit sendPassiveDataSignal(DataConverter::JsonArrayToByteArray(serverResponse));
            }
        }
        break;
    case 4: // DownloadFile
        if (request.contains("localPath") && request.contains("filePathServer") && request.contains("fileNameServer")) {
            QString localPath = request["localPath"].toString();
            QString filePathServer = request["filePathServer"].toString();
            QString fileNameServer = request["fileNameServer"].toString();
            qDebug() << "DownloadFile - LocalPath:" << localPath
                     << ", ServerPath:" << filePathServer
                     << ", FileName:" << fileNameServer;
        }
        break;
    case 5: // UploadFile
        if (request.contains("localDirPath") && request.contains("fileNameLocal") &&
            request.contains("fileSizeLocal") && request.contains("filePathLocal")) {
            QString localDirPath = request["localDirPath"].toString();
            QString fileNameLocal = request["fileNameLocal"].toString();
            QString fileSizeLocal = request["fileSizeLocal"].toString();
            QString filePathLocal = request["filePathLocal"].toString();
            qDebug() << "UploadFile - DirPath:" << localDirPath
                     << ", FileName:" << fileNameLocal
                     << ", Size:" << fileSizeLocal
                     << ", FilePath:" << filePathLocal;
        }
        break;
    default:
        qWarning() << "Unknown request type!";
        break;
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



