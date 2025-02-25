#ifndef FTP_MANAGER_H
#define FTP_MANAGER_H

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QFileInfoList>
#include <QDir>
#include <QByteArray>
#include <QFileIconProvider>
#include <QDateTime>
#include <QBuffer>
#include <QPixmap>
#include "data_converter.h"

class FTPManager
{
public:

    enum class RequestType
    {
        ActiveConnect = 0,
        PassiveConnect,
        ChangeDir,
        Delete,
        DownloadFile,
        UploadFile,
    };

    enum class ResponseType
    {
        ActiveConnected = 0,
        PassiveConnected,
        ChangedDir,
        Deleted,
        UnDeleted,
        DownloadedFile,
        DownloadingFile,
        UploadedFile,
        UploadingFile,
    };

    static QJsonArray createServerResponse(ResponseType responseStatus, const QString& dir, quint64 bytesWritten = {});
    static bool checkFileExists(const QString& filePath, const QString& fileName);
    static QString changeFileName(const QString& fileName, const QString& filePath);
    static bool checkIfDataIsJson(const QByteArray& data);

    static QJsonArray createUploadProgressResponse(ResponseType responseStatus, const QString& path, quint64 bytesWritten);
    static bool checkIfBaseDir(const QString& directory, const QString& homeDirectory);
    static bool deleteFiles(const QStringList& filesToDelete);
    static bool renameFile(const QString& filePath, const QString& oldFileName, QString& newFileName);
    static bool createFolder(const QString& newFolderName);

private:
    static QJsonValue encodePixmapForJson(const QPixmap& p);
    static QPixmap getIconFromFileInfo(const QFileInfo& file);
    static QFileInfoList getFilesFromDirectory(const QString& dir);

    FTPManager();
};

#endif // FTP_MANAGER_H
