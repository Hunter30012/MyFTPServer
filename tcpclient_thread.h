#ifndef TCPCLIENT_THREAD_H
#define TCPCLIENT_THREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QColor>

class TcpClientThread : public QObject
{
    Q_OBJECT
    friend class ServerController;
public:
    explicit TcpClientThread(QObject *parent = nullptr);

signals:
    // info
    void writeTextSignal(QString text, QColor color = {});
    void sendDataSignal(const QByteArray& data);

    // void connectedSignal();
    void disconnected();
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &errorMsg);
public slots:
    void startThread(const QString &serverIp, quint16 port);
    void sendData(const QByteArray &data);
    void quit();

private slots:
    void connected();
    void run();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QThread m_thread;
    QTcpSocket *m_socket;
    QString m_serverIp;
    quint16 m_serverPort;
};


#endif // TCPCLIENT_THREAD_H
