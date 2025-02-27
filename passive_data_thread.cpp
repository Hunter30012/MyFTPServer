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

void PassiveDataThread::restartListening(int port, const QString& dir)
{
    m_curDir = dir;
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
    qDebug() << "Passive Data Socket connected!";
    QJsonArray serverResponse = FTPManager::createServerResponse(FTPManager::ResponseType::PassiveConnected , m_curDir);
    this->sendData(DataConverter::JsonArrayToByteArray(serverResponse));
}

void PassiveDataThread::sendData(const QByteArray &data)
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        emit writeTextSignal("Send data to Client", Qt::darkBlue);
        m_socket->write(data);
        m_socket->flush();
    } else {
        qWarning() << "No active connection to write data.";
    }
}

void PassiveDataThread::downloadFiles(const QString& localPath, const QStringList &listFiles)
{
    QFile qFile;
    const qint64 packetSize = 10000;
    quint64 writtenBytes = 0;
    quint64 size = 0;
    QByteArray fileData;

    for (const QString &file : listFiles) {
        qDebug() << "local path: " << localPath;
        qDebug() << "file name: " << file;

        QFileInfo fileInfo(file);

        // Folder
        if (fileInfo.isDir()) {
            QStringList subFiles;
            QFileInfoList filesInfo = FTPManager::getFilesFromDirectory(file);
            for (int i = 0; i < filesInfo.count(); i++) {
                subFiles.append(filesInfo[i].absoluteFilePath());
            }

            QJsonArray serverResponse = FTPManager::createServerDownloadResponse(
                FTPManager::ResponseType::DownloadedFile,
                localPath,
                file,
                subFiles,
                true);
            this->sendData(DataConverter::JsonArrayToByteArray(serverResponse));

            emit writeTextSignal("Sent folder info: " + file, Qt::darkBlue);
            continue;
        }

        // Hanlde File
        writtenBytes = 0;
        qFile.setFileName(file); // example file = "/home/user/file1.txt"
        size = qFile.size();
        qDebug() << "size: " << QString::number(size);

        if (!qFile.open(QIODevice::ReadOnly) || !qFile.isReadable()) {
            emit writeTextSignal("Can not open file: " + file, Qt::red);
            continue;
        }
        // Send file immediately
        if(size < packetSize) {
            writtenBytes = size;
            fileData = qFile.read(size);
            QJsonArray serverResponse = FTPManager::createServerDownloadResponse(
                FTPManager::ResponseType::DownloadedFile,
                localPath,
                file,
                {},
                false,
                writtenBytes,
                size,
                fileData);
            // qDebug() << "Sent JSON: " << serverResponse;
            this->sendData(DataConverter::JsonArrayToByteArray(serverResponse));
            emit writeTextSignal("Transfered file to Client!", Qt::darkBlue);
            continue;
        }
        // Split file into chunks and send
        while (writtenBytes < size) {
            qFile.seek(writtenBytes);
            fileData = qFile.read(packetSize);
            quint64 currentChunkSize = fileData.size();
            writtenBytes += currentChunkSize;

            QJsonArray serverResponse = FTPManager::createServerDownloadResponse(
                FTPManager::ResponseType::DownloadingFile,
                localPath,
                file,
                {},
                false,
                writtenBytes,
                size,
                fileData);
            this->sendData(DataConverter::JsonArrayToByteArray(serverResponse));
            QThread::msleep(50);
        }
        // download big file: Done
        QJsonArray serverResponse = FTPManager::createServerDownloadResponse(
            FTPManager::ResponseType::DownloadedFile,
            localPath,
            file,
            {},
            false,
            writtenBytes,
            size,
            QByteArray());
        this->sendData(DataConverter::JsonArrayToByteArray(serverResponse));
        emit writeTextSignal("Finished sending file: " + file, Qt::darkGreen);
    }
}

void PassiveDataThread::onReadyRead()
{
    // TBD
    if (m_socket) {
        QByteArray data = m_socket->readAll();
        emit writeTextSignal("Recieved Data from Client", Qt::darkBlue);
        emit dataReceivedSignal(data);
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






