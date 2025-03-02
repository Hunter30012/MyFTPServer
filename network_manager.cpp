#include "network_manager.h"

NetworkManager::NetworkManager(QObject *parent)
    : QObject{parent}
{
    // connect to ActiveData Thread
    connect(&m_commandThread, &CommandThread::startActiveDataThreadSignal, &m_activeDataThread, &ActiveDataThread::startThread, Qt::QueuedConnection);
    connect(&m_commandThread, &CommandThread::stopActiveDataSignal, &m_activeDataThread, &ActiveDataThread::stopConnection, Qt::QueuedConnection);
    connect(&m_commandThread, &CommandThread::restartActiveDataSignal, &m_activeDataThread, &ActiveDataThread::restartConnection, Qt::QueuedConnection);
    connect(&m_commandThread, &CommandThread::sendActiveDataSignal, &m_activeDataThread, &ActiveDataThread::sendData, Qt::QueuedConnection);
    connect(&m_commandThread, &CommandThread::downloadActiveFilesSignal, &m_activeDataThread, &ActiveDataThread::downloadFiles, Qt::QueuedConnection);
    connect(&m_activeDataThread, &ActiveDataThread::dataReceivedSignal, this, &NetworkManager::parseJsonUpload, Qt::QueuedConnection);
    connect(this, &NetworkManager::sendActiveDataSignal, &m_activeDataThread, &ActiveDataThread::sendData, Qt::QueuedConnection);

    //connect to PassiveDataThread
    connect(&m_commandThread, &CommandThread::startPassiveDataThreadSignal, &m_passiveDataThread, &PassiveDataThread::startThread, Qt::QueuedConnection);
    connect(&m_commandThread, &CommandThread::stopPassiveDataSignal, &m_passiveDataThread, &PassiveDataThread::stopListening, Qt::QueuedConnection);
    connect(&m_commandThread, &CommandThread::restartPassiveDataThreadSignal, &m_passiveDataThread, &PassiveDataThread::restartListening, Qt::QueuedConnection);
    connect(&m_commandThread, &CommandThread::sendPassiveDataSignal, &m_passiveDataThread, &PassiveDataThread::sendData, Qt::QueuedConnection);
    connect(&m_commandThread, &CommandThread::downloadPassiveFilesSignal, &m_passiveDataThread, &PassiveDataThread::downloadFiles, Qt::QueuedConnection);

    connect(this, &NetworkManager::sendCommandSignal, &m_commandThread, &CommandThread::sendData, Qt::QueuedConnection);
}

bool NetworkManager::isValidPort(const QString &port)
{
    if (port.isEmpty()) {
        return false;
    }
    bool isNumber;
    int portNumber = port.toInt(&isNumber);

    return isNumber && (portNumber >= 1 && portNumber <= 65535);
}

void NetworkManager::startServer(int port, const QString& dir)
{
    qDebug() << "Server Dir: " << dir;
    m_commandThread.startThread(port, dir);
}
void NetworkManager::stopServer()
{
    emit stopServerSignal();
}

void NetworkManager::parseJsonUpload(const QByteArray &data)
{
    bool isJson = FTPManager::checkIfDataIsJson(data);
    if (!isJson) {
        emit writeTextSignal("Request from Client - Invalid Json format!", Qt::red);
        return;
    }
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    QJsonObject request = jsonDoc.object();

    if (!request.contains("request_type")) {
        qDebug() << "Invalid request: Missing request_type!";
        return;
    }
    FTPManager::RequestType requestType = static_cast<FTPManager::RequestType>(request["request_type"].toInt());
    switch (requestType) {
    case FTPManager::RequestType::UploadedFile:
        qDebug() << "Hanlde Uploaded";
        this->handleUploadedRequest(request);
        break;
    case FTPManager::RequestType::UploadingFile:
        this->handleUploadingRequest(request);
        break;
    default:
        emit writeTextSignal("Unknown request type!", Qt::red);
        break;
    }
}

void NetworkManager::handleUploadedRequest(const QJsonObject &json)
{
    QString localPath = json["localPath"].toString();
    QString serverPath = json["serverPath"].toString();
    bool isDir = json["isDir"].toBool();
    QString fileName = json["fileNameLocal"].toString();

    QString serverSavePath = serverPath + "/" + fileName;
    if (!m_saveFile.isOpen()) {
        if (FTPManager::checkFileExists(serverPath, fileName)) {
            fileName = FTPManager::changeFileName(fileName, serverPath);
            serverSavePath = serverPath + "/" + fileName;
        }
    }
    qDebug() << "Save Path in Server: " << serverSavePath;

    // Folder
    if(isDir) {
        bool ret = QDir().mkpath(serverSavePath);
        emit writeTextSignal("Created folder: " + serverSavePath, Qt::darkGreen);
        QJsonArray response = FTPManager::createServerResponse(FTPManager::ResponseType::UploadedFile,
                                                                serverPath,
                                                                true,
                                                                ret,
                                                                localPath,
                                                                serverSavePath);
        emit sendCommandSignal(DataConverter::JsonArrayToByteArray(response));
        return;
    }

    // File
    quint64 sizeFile = json["sizeFile"].toString().toULongLong();
    QByteArray dataPacket = QByteArray::fromBase64(json["dataPacket"].toString().toLatin1());
    // small file
    if(sizeFile < packetSize) {
        m_saveFile.setFileName(serverSavePath);
        bool ret = m_saveFile.open(QIODevice::WriteOnly);
        if(!ret) {
            emit writeTextSignal("Failed to save file: " + fileName, Qt::red);
            return;
        }
        m_saveFile.write(dataPacket);
        m_saveFile.commit();
        QJsonArray response = FTPManager::createServerResponse(FTPManager::ResponseType::UploadedFile,
                                                               serverPath,
                                                               false,
                                                               true,
                                                               localPath,
                                                               serverSavePath);
        emit sendCommandSignal(DataConverter::JsonArrayToByteArray(response));
        emit writeTextSignal("Save file " + fileName + " successfully!", Qt::darkBlue);
    } else {
        // downloaded big file
        bool isSuccess = true;
        m_saveFile.commit();
        QFile finalFile(serverSavePath);
        if (!finalFile.open(QIODevice::ReadOnly)) {
            emit writeTextSignal("Failed to open file", Qt::red);
            return;
        }
        if(finalFile.size() != sizeFile) {
            isSuccess = false;
            emit writeTextSignal("Save file " + fileName + " lost some data!", Qt::red);
        }
        QJsonArray response = FTPManager::createServerResponse(FTPManager::ResponseType::UploadedFile,
                                                               serverPath,
                                                               false,
                                                               isSuccess,
                                                               localPath,
                                                               serverSavePath);
        emit sendCommandSignal(DataConverter::JsonArrayToByteArray(response));
        if(isSuccess)
            emit writeTextSignal("Save file " + fileName + " successfully!", Qt::darkBlue);
    }
}

void NetworkManager::handleUploadingRequest(const QJsonObject &json)
{
    QString localPath = json["localPath"].toString();
    QString serverPath = json["serverPath"].toString();
    QString fileName = json["fileNameLocal"].toString();
    quint64 sizeFile = json["sizeFile"].toString().toULongLong();
    quint64 writtenBytes = json["writtenBytes"].toString().toULongLong();
    QByteArray dataPacket = QByteArray::fromBase64(json["dataPacket"].toString().toLatin1());

    QString serverSavePath = serverPath + "/" + fileName;
    if (!m_saveFile.isOpen()) {
        if (FTPManager::checkFileExists(localPath, fileName)) {
            fileName = FTPManager::changeFileName(fileName, serverPath);
            serverSavePath = localPath + "/" + fileName;
        }
        qDebug() << serverSavePath;
        m_saveFile.setFileName(serverSavePath);
        if (!m_saveFile.open(QIODevice::WriteOnly)) {
            emit writeTextSignal("Failed to open file: " + fileName, Qt::red);
            return;
        }
    }
    m_saveFile.write(dataPacket);
    emit writeTextSignal(QString("Received %1/%2 bytes for file: %3")
                             .arg(writtenBytes).arg(sizeFile).arg(fileName),
                         Qt::darkYellow);
}
