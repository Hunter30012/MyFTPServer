#ifndef SERVER_CONTROLLER_H
#define SERVER_CONTROLLER_H

#include <QObject>
#include <QApplication>
#include "server_model.h"
#include "server_window.h"

class ServerController : public QObject
{
    Q_OBJECT
public:
    explicit ServerController(int argc, char *argv[], QWidget *parent = nullptr);
    int init();
signals:

private:
    void connectWindowSignalSlots(QList<bool>& connectionResults);
    void connectModelSignalSlots(QList<bool>& connectionResults);

    QApplication m_app;
    ServerModel m_model;
    ServerWindow m_window;
};

#endif // SERVER_CONTROLLER_H
