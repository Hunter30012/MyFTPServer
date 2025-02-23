#ifndef ACTIVE_DATA_THREAD_H
#define ACTIVE_DATA_THREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QColor>
#include "ftp_manager.h"

class ActiveDataThread : public QObject
{
    Q_OBJECT
    friend class ServerController;
public:
    explicit ActiveDataThread(QObject *parent = nullptr);
    ~ActiveDataThread();
signals:
    // info
    void writeTextSignal(QString text, QColor color = {});
    void sendDataSignal(const QByteArray& data);

    void disconnectedSignal();
    void dataReceivedSignal(const QByteArray &data);
public slots:
    void startThread();
    void restartConnection(const QHostAddress &serverIp, int port, const QString& curDir);
    void stopConnection();

    void sendData(const QByteArray &data);

    // Handle command
    void onConnectedActive(const QString& dir);

private slots:
    void onConnected();
    void onStarted();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QHostAddress m_serverIp;
    int m_serverPort;
    QString m_curDir;

    QThread m_thread;
    QTcpSocket *m_socket;
};


#endif // ACTIVE_DATA_THREAD_H
