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
    if (!m_thread.isRunning()) {
        qDebug() << "Start Active Data Thread";
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
    // qDebug() << "Send data in ActiveDataThread";
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(data);
        m_socket->flush();
    } else {
        emit writeTextSignal("Cannot send data, no active connection!", Qt::red);
    }
}

void ActiveDataThread::downloadFiles(const QString& localPath, const QStringList &listFiles)
{
    QFile qFile;
    const qint64 packetSize = 20000;
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
            QThread::msleep(10);
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

void ActiveDataThread::onReadyRead()
{
    if (m_socket) {
        QByteArray data = m_socket->readAll();
        emit dataReceivedSignal(data);
    }
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
