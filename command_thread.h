#ifndef COMMAND_THREAD_H
#define COMMAND_THREAD_H

#include <QObject>
#include <QTcpServer>
#include <QThread>
#include <QTcpSocket>
#include <QColor>
#include <QNetworkInterface>
#include "ftp_manager.h"

class CommandThread : public QObject
{
    Q_OBJECT
    friend class ServerController;
public:
    explicit CommandThread(QObject *parent = nullptr);
    ~CommandThread();
signals:
    // info
    void writeTextSignal(QString text, QColor color = {});
    // acticve mode
    void startActiveDataThreadSignal();
    void restartActiveDataSignal(const QHostAddress& address, int port, const QString& curDir);
    void stopActiveDataSignal();
    void sendActiveDataSignal(const QByteArray& data);
    void downloadActiveFilesSignal(const QString& localPath, const QStringList& listFiles);

    // passive mode
    void startPassiveDataThreadSignal();
    void restartPassiveDataThreadSignal(int port, const QString dir);
    void stopPassiveDataSignal();
    void sendPassiveDataSignal(const QByteArray& data);
    void downloadPassiveFilesSignal(const QString& localPath, const QStringList& listFiles);

    void sendDataSignal(const QByteArray& data);
    // Control Button
    void enableStopSignal();
    void disableStopSignal();

public slots:
    void startThread(int port, const QString& dir); // connect to Button
    void stopListening();          // connect to Button
    void sendData(const QByteArray& data);
private slots:
    void onStarted();
    void onNewConnection();
    void onReadyRead();
    void disconnected();
    void parseRequest(const QByteArray& requestData);

private:
    QString m_curDir;
    bool m_isActiveMode;
    int m_port;
    QHostAddress m_address;

    QThread m_thread;
    QTcpServer* m_server;
    QTcpSocket* m_socket;
};

#endif // COMMAND_THREAD_H
