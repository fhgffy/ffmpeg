#ifndef FACEAPIMANAGER_H
#define FACEAPIMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QImage>

class FaceApiManager : public QObject
{
    Q_OBJECT
public:
    explicit FaceApiManager(QObject *parent = nullptr);

    // 注册人脸
    void registerFace(const QString &ip, const QString &user, const QString &pwd, const QImage &img);

    // 设置回调地址 (告诉摄像头把报警发给谁)
    void setCallback(const QString &ip, const QString &user, const QString &pwd, const QString &myIp, int myPort);

signals:
    void sigRegisterResult(bool success, QString msg);
    void sigConfigResult(bool success, QString msg);

private:
    QNetworkAccessManager *m_net;
    QString generateAuth(const QString &u, const QString &p);
    QString imgToBase64(const QImage &img);
};

#endif // FACEAPIMANAGER_H
