#include "notificationserver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QBuffer>
#include <QDebug>

NotificationServer::NotificationServer(QObject *parent) : QTcpServer(parent) {}

bool NotificationServer::startServer(int port)
{
    if(this->listen(QHostAddress::Any, port)) {
        qDebug() << "报警回调服务器监听成功，端口:" << port;
        connect(this, &QTcpServer::newConnection, this, &NotificationServer::onNewConnection);
        return true;
    } else {
        qDebug() << "监听失败:" << this->errorString();
        return false;
    }
}

void NotificationServer::onNewConnection()
{
    QTcpSocket *socket = this->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &NotificationServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

void NotificationServer::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if(!socket) return;

    // 读取所有数据（简单处理，假设数据包一次传完，实际生产环境需要处理粘包）
    QByteArray data = socket->readAll();

    // 简单的 HTTP 解析，找到 Body 部分（通常在 \r\n\r\n 之后）
    int bodyIndex = data.indexOf("\r\n\r\n");
    if(bodyIndex != -1) {
        QByteArray body = data.mid(bodyIndex + 4);

        // 解析 JSON
        QJsonDocument doc = QJsonDocument::fromJson(body);
        if(!doc.isNull() && doc.isObject()) {
            QJsonObject obj = doc.object();

            // 提取字段 (参考文档 Section 5)
            QString typeStr = obj.value("type").toString();
            QString serial = obj.value("serial").toString();
            qint64 timeSec = obj.value("time").toVariant().toLongLong();
            QString imgBase64 = obj.value("images").toString();

            // 类型映射
            QString typeCN = "未知事件";
            QString content = "检测到异常";
            if(typeStr == "face") { typeCN = "人脸识别"; content = "识别到注册人员"; }
            else if(typeStr == "tram") { typeCN = "车辆检测"; content = "检测到电瓶车"; }
            // ... 其他类型

            // Base64 转图片
            QImage img;
            img.loadFromData(QByteArray::fromBase64(imgBase64.toLatin1()));

            // 发送信号
            QString timeStr = QDateTime::fromSecsSinceEpoch(timeSec).toString("HH:mm:ss");
            emit sigAlarmReceived(typeCN, content, timeStr, img);
        }
    }

    // 回复 HTTP 200 OK
    QString response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: application/json\r\n"
                       "Content-Length: 17\r\n"
                       "\r\n"
                       "{\"code\":0,\"msg\":\"\"}";
    socket->write(response.toUtf8());
    socket->flush();
    socket->close(); // 短连接，回复完即断开
}
