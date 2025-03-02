#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <QObject>
#include <QSaveFile>
#include "command_thread.h"
#include "active_data_thread.h"
#include "passive_data_thread.h"

class NetworkManager : public QObject
{
    Q_OBJECT
    friend class ServerController;
public:
    explicit NetworkManager(QObject *parent = nullptr);
    static bool isValidPort(const QString& port);

signals:
    void stopServerSignal();
    void writeTextSignal(QString text, QColor color = {});
    void sendActiveDataSignal(const QByteArray& data);
    void sendPassiveDataSignal(const QByteArray& data);
    void sendCommandSignal(const QByteArray& data);
public slots:
    void startServer(int port, const QString& dir);
    void stopServer();
    void parseJsonUpload(const QByteArray& data);
    void handleUploadedRequest(const QJsonObject& json);
    void handleUploadingRequest(const QJsonObject& json);
private:
    const qint64 packetSize = 20000;
    QSaveFile m_saveFile;

    CommandThread m_commandThread;
    PassiveDataThread m_passiveDataThread;
    ActiveDataThread m_activeDataThread;
};

#endif // NETWORK_MANAGER_H
