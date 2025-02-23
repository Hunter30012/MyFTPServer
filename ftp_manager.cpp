#include "ftp_manager.h"

FTPManager::FTPManager() {}

QFileInfoList FTPManager::getFilesFromDirectory(const QString &dir)
{
    QDir directory(dir);
    return directory.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
}

QJsonValue FTPManager::encodePixmapForJson(const QPixmap &p)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    p.save(&buffer, "PNG");
    return QString::fromLatin1(buffer.data());
}

QPixmap FTPManager::getIconFromFileInfo(const QFileInfo &file)
{
    QFileIconProvider qfileIconProvider;
    QIcon icon = qfileIconProvider.icon(file);
    return icon.pixmap(icon.actualSize(QSize(12, 12)));
}

QJsonArray FTPManager::createServerResponse(ResponseType responseStatus, const QString &dir, quint64 bytesWritten)
{
    QJsonArray serverResponse;

    serverResponse.append(QJsonObject {
        {"directory" , dir} ,
        {"response_status", static_cast<int>(responseStatus)} ,
        {"bytesWritten", QString::number(bytesWritten)} ,
    });
    QFileInfoList filesInfo = getFilesFromDirectory(dir);

    for (int i = 0; i < filesInfo.count(); i++)
    {
        QPixmap icon = getIconFromFileInfo(filesInfo[i]);
        QJsonObject json {
                {"fileName", filesInfo[i].fileName()},
                {"fileSize", QString::number(filesInfo[i].size())},
                {"filePath", filesInfo[i].absoluteFilePath()},
                {"isDir", filesInfo[i].isDir()},
                {"lastModified", filesInfo[i].lastModified().toString()},
                {"icon", encodePixmapForJson(icon)},
            };
        serverResponse.append(json);
    }

    return serverResponse;
}

bool FTPManager::checkFileExists(const QString &filePath, const QString &fileName)
{
    return QDir(filePath).exists(fileName);
}

QString FTPManager::changeFileName(const QString &fileName, const QString &filePath)
{
    int fileNumToAppend = 0;
    QString newFileName;
    do
    {
        newFileName = fileName;
        ++fileNumToAppend;
        newFileName = newFileName.insert(newFileName.indexOf("."), "_" + QString::number(fileNumToAppend));
    } while (checkFileExists(filePath, newFileName));

    return newFileName;
}

bool FTPManager::checkIfDataIsJson(const QByteArray &data)
{
    QJsonParseError jsonError;
    QJsonDocument::fromJson(data, &jsonError);
    return (jsonError.error == QJsonParseError::NoError) ? true : false;
}

void FTPManager::parseRequest(const QByteArray &requestData)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(requestData);
    QJsonObject request = jsonDoc.object();

    if (!request.contains("request_type")) {
        qDebug() << "Invalid request: Missing request_type!";
        return;
    }

    int requestType = request["request_type"].toInt();
    qDebug() << "Received request type:" << requestType;

    switch (requestType) {
    case 0: // ActiveConnect
        if (request.contains("port_data_thread")) {
            int portDataThread = request["port_data_thread"].toInt();
            qDebug() << "ActiveConnect - Port:" << portDataThread;
        }
        break;
    case 1: // PassiveConnect
        qDebug() << "PassiveConnect request received.";
        break;
    case 2: // ChangeDir
        if (request.contains("requestPath")) {
            QString requestPath = request["requestPath"].toString();
            qDebug() << "ChangeDir - Path:" << requestPath;
        }
        break;
    case 3: // Delete
        if (request.contains("fileToDelete")) {
            QString fileToDelete = request["fileToDelete"].toString();
            qDebug() << "Delete - File:" << fileToDelete;
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




