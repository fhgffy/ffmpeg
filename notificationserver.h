#ifndef NOTIFICATIONSERVER_H
#define NOTIFICATIONSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QImage>
class NotificationServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit NotificationServer(QObject *parent = nullptr);
    bool startServer(int port = 9999);

signals:
    // 解析出报警信息后发送此信号
    void sigAlarmReceived(QString type, QString content, QString time, QImage img);

private slots:
    void onNewConnection();
    void onReadyRead();
};

#endif // NOTIFICATIONSERVER_H
