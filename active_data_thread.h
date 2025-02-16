#ifndef ACTIVE_DATA_THREAD_H
#define ACTIVE_DATA_THREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QColor>

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

    void disconnected();
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &errorMsg);
public slots:
    void startThread();
    void restartConnection(const QHostAddress &serverIp, int port);
    void stopConnection();

    void sendData(const QByteArray &data);

private slots:
    void connected();
    void onStarted();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QHostAddress m_serverIp;
    int m_serverPort;

    QThread m_thread;
    QTcpSocket *m_socket;
};


#endif // ACTIVE_DATA_THREAD_H
