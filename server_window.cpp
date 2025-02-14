#include "server_window.h"
#include "./ui_serverwindow.h"

ServerWindow::ServerWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
    setWindowTitle("My FTP Server");
    ui->stopButton->setDisabled(true);
}

ServerWindow::~ServerWindow()
{
    delete ui;
}

void ServerWindow::writeTextToOutput(QString text, QColor color)
{
    QString addColor = (color.isValid()) ? "style='color:" + color.name() + ";'" : "";
    ui->textBrowser->append("<span " + addColor + " > [" + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss ") + "] - " + text + "</span>");
}

void ServerWindow::startServer()
{
    bool ret;
    QString portText = ui->portLineEdit->text();
    int port = portText.toInt(&ret);
    qDebug() << "Port Number: " + QString::number(port);
    if(ret) {
        emit startServerSignal(port);
    } else {
        writeTextToOutput("Please enter the valid port!", Qt::red);
    }
}

void ServerWindow::enableStop()
{
    ui->startButton->setDisabled(true);
    ui->stopButton->setDisabled(false);
}

void ServerWindow::disableStop()
{
    ui->stopButton->setDisabled(true);
    ui->startButton->setDisabled(false);
}
