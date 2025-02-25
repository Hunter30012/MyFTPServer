#include "ftp_manager.h"

FTPManager::FTPManager() {}

QFileInfoList FTPManager::getFilesFromDirectory(const QString &dir)
{
    if (dir.isEmpty()) {
        return QDir::drives();
    }
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
    QFileIconProvider qfileIconProvider;

    serverResponse.append(QJsonObject {
        {"directory" , dir} ,
        {"response_status", static_cast<int>(responseStatus)} ,
        {"bytesWritten", QString::number(bytesWritten)} ,
    });
    QFileInfoList filesInfo = getFilesFromDirectory(dir);

    for (int i = 0; i < filesInfo.count(); i++)
    {
        QPixmap icon = getIconFromFileInfo(filesInfo[i]);
        QString fileType = qfileIconProvider.type(filesInfo[i]);
        QString fileName = filesInfo[i].fileName();

        if (fileName.isEmpty()) {
            fileName = filesInfo[i].absoluteFilePath(); // "C:/" or "D:/"
        }

        QJsonObject json {
                {"fileName", fileName},
                {"fileSize", QString::number(filesInfo[i].size())},
                {"filePath", filesInfo[i].absoluteFilePath()},
                {"isDir", filesInfo[i].isDir()},
                {"lastModified", filesInfo[i].lastModified().toString("M/d/yyyy h:mm AP")},
                {"icon", encodePixmapForJson(icon)},
                {"fileType", fileType},
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

bool FTPManager::deleteFiles(const QStringList &filesToDelete)
{
    bool ret = true;
    for (const QString &path : filesToDelete) {
        QFileInfo fileInfo(path);

        if (fileInfo.exists()) {
            if (fileInfo.isFile()) {
                QFile file(path);
                if (file.remove()) {
                    qDebug() << "Deleted file:" << path;
                } else {
                    ret = false;
                    qDebug() << "Failed to delete file:" << path;
                }
            } else if (fileInfo.isDir()) {
                QDir dir(path);
                if (dir.removeRecursively()) {
                    qDebug() << "Deleted directory:" << path;
                } else {
                    ret  = false;
                    qDebug() << "Failed to delete directory:" << path;
                }
            }
        } else {
            ret = false;
            qDebug() << "Path not found:" << path;
        }
    }
    return ret;
}




