#ifndef PASSIVE_DATA_THREAD_H
#define PASSIVE_DATA_THREAD_H

#include <QObject>
#include <QTcpServer>
#include <QThread>
#include <QTcpSocket>
#include <QColor>
#include <QNetworkInterface>
#include "ftp_manager.h"

class PassiveDataThread : public QObject
{
    Q_OBJECT
    friend class ServerController;
public:
    explicit PassiveDataThread(QObject *parent = nullptr);
    ~PassiveDataThread();
signals:
    void writeTextSignal(QString text, QColor color = {});

    void sendDataSignal(const QByteArray& data);
    void dataReceivedSignal(const QByteArray &data);
public slots:
    void startThread();
    void restartListening(int port, const QString& dir);
    void stopListening();
    void sendData(const QByteArray& data);
private slots:
    void onStarted();

    void onNewConnection();
    void onReadyRead();
    void disconnected();
private:
    int m_port;
    QHostAddress m_address;
    QString m_curDir;

    QThread m_thread;
    QTcpServer* m_server;
    QTcpSocket* m_socket;
};

#endif // PASSIVE_DATA_THREAD_H
