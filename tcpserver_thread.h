#ifndef TCPSERVER_THREAD_H
#define TCPSERVER_THREAD_H

#include <QObject>
#include <QTcpServer>
#include <QThread>
#include <QTcpSocket>
#include <QColor>
#include <QNetworkInterface>

class TcpServerThread : public QObject
{
    Q_OBJECT
    friend class ServerController;
public:
    explicit TcpServerThread(QObject *parent = nullptr);

signals:
    // info
    void writeTextSignal(QString text, QColor color = {});

    void startActiveDataThread(const QString& address, int port);

    void sendDataSignal(const QByteArray& data);
    void enableStopSignal();
    void disableStopSignal();

public slots:
    void startThread(int port); // connect to Button
    void quit();

private slots:
    void run();

    void onNewConnection();
    void onReadyRead();
    void disconnected();

    void sendData(const QByteArray& data);

private:
    int m_port;

    QThread m_thread;
    QTcpServer* m_server;
    QTcpSocket* m_socket;
};

#endif // TCPSERVER_THREAD_H
