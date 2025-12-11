#include "faceapimanager.h"
#include "cryptstring.h" // 复用你现有的MD5类
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QBuffer>
#include <QJsonArray>

FaceApiManager::FaceApiManager(QObject *parent) : QObject(parent)
{
    m_net = new QNetworkAccessManager(this);
}

QString FaceApiManager::generateAuth(const QString &u, const QString &p) {
    // 文档：auth = md5(用户名+密码)
    CryptString crypt(u + p);
    return crypt.generateMD5();
}

QString FaceApiManager::imgToBase64(const QImage &img) {
    QByteArray ba;
    QBuffer buf(&ba);
    img.save(&buf, "JPG");
    return QString(ba.toBase64());
}

void FaceApiManager::registerFace(const QString &ip, const QString &user, const QString &pwd, const QImage &img)
{
    QString url = QString("http://%1:3002/face/register").arg(ip);

        qDebug() << ">>> [方案1] 正在尝试注册地址:" << url;
    QNetworkRequest req((QUrl(url)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json.insert("auth", generateAuth(user, pwd));
    json.insert("data", imgToBase64(img));

    QNetworkReply *reply = m_net->post(req, QJsonDocument(json).toJson());
    // faceapimanager.cpp -> registerFace 函数内

    connect(reply, &QNetworkReply::finished, this, [=](){
        // 打印 HTTP 状态码（如 200, 404, 500）
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << ">>> [HTTP状态码]" << statusCode;

        if(reply->error() == QNetworkReply::NoError) {
            QJsonObject res = QJsonDocument::fromJson(reply->readAll()).object();
            // ... (原成功处理逻辑不变)
            if(res["code"].toInt() == 0)
                emit sigRegisterResult(true, "注册成功");
            else
                emit sigRegisterResult(false, "业务失败:" + res["msg"].toString());
        } else {
            // 【关键修改】这里不要只发“网络错误”，而是发送具体的错误信息
            QString errStr = reply->errorString();
            qDebug() << ">>> [网络错误详情]" << errStr; // 在控制台打印
            emit sigRegisterResult(false, "网络错误: " + errStr); // 弹窗显示具体原因
        }
        reply->deleteLater();
    });
}

void FaceApiManager::setCallback(const QString &ip, const QString &user, const QString &pwd, const QString &myIp, int myPort)
{
    QString url = QString("http://%1:3002/face/cb/setUrl").arg(ip);
    QNetworkRequest req((QUrl(url)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 构造回调地址：http://我的IP:9999
    QString myBaseUrl = QString("http://%1:%2").arg(myIp).arg(myPort);
    QByteArray urlBase64 = myBaseUrl.toUtf8().toBase64(); // 接口要求base64

    QJsonObject json;
    json.insert("auth", generateAuth(user, pwd));
    json.insert("url", QString(urlBase64));
    json.insert("uploadPath", "/alarm"); // 路径随便写，反正我们socket读全部

    QNetworkReply *reply = m_net->post(req, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [=](){
        if(reply->error() == QNetworkReply::NoError) emit sigConfigResult(true, "配置成功");
        else emit sigConfigResult(false, "配置失败");
        reply->deleteLater();
    });
}
