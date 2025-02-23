#ifndef SERVER_WINDOW_H
#define SERVER_WINDOW_H

#include <QWidget>
#include <QDateTime>
#include <QTcpServer>
#include "./ui_serverwindow.h"

class ServerWindow : public QWidget
{
    Q_OBJECT
    friend class ServerController;
public:
    ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();
signals:
    void startServerSignal(int port, const QString& dir);

public slots:
    void writeTextToOutput(QString text, QColor color);
    void startServer();
    void enableStop();
    void disableStop();
    void clearOutput();

    bool isPortAvailable(int port);

private:
    Ui::ServerWindow *ui;
};
#endif // SERVER_WINDOW_H
