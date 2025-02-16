#ifndef PASSIVE_DATA_THREAD_H
#define PASSIVE_DATA_THREAD_H

#include <QObject>
#include <QTcpServer>
#include <QThread>
#include <QTcpSocket>
#include <QColor>
#include <QNetworkInterface>

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

public slots:
    void startThread();
    void restartListening(int port);
    void stopListening();

private slots:
    void onStarted();

    void onNewConnection();
    void onReadyRead();
    void disconnected();

    void sendData(const QByteArray& data);
private:
    int m_port;
    QHostAddress m_address;

    QThread m_thread;
    QTcpServer* m_server;
    QTcpSocket* m_socket;
};

#endif // PASSIVE_DATA_THREAD_H
