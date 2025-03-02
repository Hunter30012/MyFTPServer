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
        UploadedFile,
        UploadingFile,
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

    static QFileInfoList getFilesFromDirectory(const QString& dir);
    static QJsonArray createServerResponse(ResponseType responseStatus,
                                           const QString& dir,
                                           bool isDir = false,
                                           bool isSuccess = true,
                                           const QString& localPath = "",
                                           const QString& saveServerPath = "");
    static QJsonArray createServerDownloadResponse(ResponseType responseStatus,
                                                   const QString& localPath,
                                                   const QString& filePathServer,
                                                   const QStringList &fileList,
                                                   bool isDir,
                                                   quint64 writtenBytes = 0,
                                                   quint64 sizeFile = 0,
                                                   const QByteArray& data = QByteArray());

    static bool checkFileExists(const QString& filePath, const QString& fileName);
    static QString changeFileName(const QString& fileName, const QString& filePath);
    static bool checkIfDataIsJson(const QByteArray& data);
    static bool deleteFiles(const QStringList& filesToDelete);

private:
    static QJsonValue encodePixmapForJson(const QPixmap& p);
    static QPixmap getIconFromFileInfo(const QFileInfo& file);

    FTPManager();
};

#endif // FTP_MANAGER_H
