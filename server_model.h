#ifndef SERVER_MODEL_H
#define SERVER_MODEL_H

#include <QObject>
#include "network_manager.h"

class ServerModel : public QObject
{
    Q_OBJECT
    friend class ServerController;
public:
    explicit ServerModel(QObject *parent = nullptr);

signals:
    void writeTextSignal(QString text, QColor color = {});

public slots:
    void startServer(int port, const QString& dir);
    void stopServer();

private:
    QString m_curDir;

    NetworkManager m_networkManager;
};

#endif // SERVER_MODEL_H
