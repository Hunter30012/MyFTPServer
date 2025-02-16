#ifndef COMMAND_THREAD_H
#define COMMAND_THREAD_H

#include <QObject>
#include <QTcpServer>
#include <QThread>
#include <QTcpSocket>
#include <QColor>
#include <QNetworkInterface>

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
    void restartActiveDataSignal(const QHostAddress& address, int port);
    void stopActiveDataSignal();

    // passive mode
    void startPassiveDataThreadSignal();
    void restartPassiveDataThreadSignal(int port);
    void stopPassiveDataSignal();

    void sendDataSignal(const QByteArray& data);
    // Control Button
    void enableStopSignal();
    void disableStopSignal();

public slots:
    void startThread(int port); // connect to Button
    void stopListening();          // connect to Button

private slots:
    void onStarted();

    void onNewConnection();
    void onReadyRead();
    void disconnected();

    void sendData(const QByteArray& data);

private:
    bool isActiveMode;
    int m_port;
    QHostAddress m_address;

    QThread m_thread;
    QTcpServer* m_server;
    QTcpSocket* m_socket;
};

#endif // COMMAND_THREAD_H
