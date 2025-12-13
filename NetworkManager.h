// NetworkManager.h - 添加TCP通信相关
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QTimer>
#include <functional>
#include <string>

// 定义消息类型 - 与client.cpp保持一致
enum MessageType {
    // 注册相关
    TASK_REGISTER_SECTION1 = 1,           // 注册第一次握手
    TASK_REGISTER_SECTION1_RESP_OK = 2,   // 注册第一次握手成功
    TASK_REGISTER_SECTION1_RESP_ERROR = 3,// 注册第一次握手失败
    TASK_REGISTER_SECTION2 = 4,           // 注册第二次握手
    TASK_REGISTER_SECTION2_RESP_OK = 5,   // 注册第二次握手成功

    // 登录相关
    TASK_LOGIN_SECTION1 = 6,              // 登录第一次握手
    TASK_LOGIN_SECTION1_RESP_OK = 7,      // 登录第一次握手成功
    TASK_LOGIN_SECTION1_RESP_ERROR = 8,   // 登录第一次握手失败
    TASK_LOGIN_SECTION2 = 9,              // 登录第二次握手
    TASK_LOGIN_SECTION2_RESP_OK = 10,     // 登录第二次握手成功
    TASK_LOGIN_SECTION2_RESP_ERROR = 11,  // 登录第二次握手失败

    // 测试连接
    TASK_TEST_CONNECTION = 15,            // 测试连接
    TASK_TEST_CONNECTION_RESP = 16        // 测试连接响应
};

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    // 登录请求
    void login(const QString &ip, int port,
               const QString &username, const QString &password,
               std::function<void(bool, const QString&)> callback);

    // 注册请求
    void registerUser(const QString &ip, int port,
                      const QString &username, const QString &password,
                      std::function<void(bool, const QString&)> callback);

    // 测试连接
    void testConnection(const QString &ip, int port,
                       std::function<void(bool, const QString&)> callback);

signals:
    void loginSuccess();
    void loginFailed(const QString &error);
    void registrationSuccess();
    void registrationFailed(const QString &error);
    void connectionTested(bool success, const QString &message);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);
    void onTimeout();
    // 移除这行：void sendFirstMessage();  // 添加私有槽函数

private:
    // TCP通信相关
    void connectToServer(const QString &ip, int port);
    void disconnectFromServer();

    // 发送TLV消息
    void sendTLVMessage(int type, const std::string &data);
    void sendTLVMessage(int type, const QString &data);
    void sendTLVMessage(int type, const char *data);  // 添加const char*版本

    // TLV结构体定义
    #pragma pack(push, 1)
    struct TLVHeader {
        int type;      // 消息类型
        int length;    // 数据长度
    };
    #pragma pack(pop)

    // 消息处理
    void handleRegister1Response(const std::string &data);
    void handleRegister2Response(const std::string &data);
    void handleLogin1Response(const std::string &data);
    void handleLogin2Response(const std::string &data);
    void handleTestConnectionResponse(const std::string &data);

    // 生成加密密文 - 返回std::string确保与服务端兼容
    std::string generateEncryptedPassword(const QString &password, const QString &salt);

    // 转换函数确保数据兼容性
    static std::string qStringToStdString(const QString &str);
    static QString stdStringToQString(const std::string &str);

    QTcpSocket *tcpSocket;
    QTimer *timeoutTimer;

    // 当前状态
    QString currentIp;
    int currentPort;
    QString currentUsername;
    QString currentPassword;  // 临时存储，第一步时保存
    QString receivedSalt;     // 服务器返回的盐值

    std::function<void(bool, const QString&)> currentCallback;

    bool isConnecting;
    bool isRegistering;
    bool isLoggingIn;

    // 新增：当前操作步骤
    enum OperationStep {
        STEP_NONE,
        STEP_CONNECTING,
        STEP_SENDING_USERNAME,  // 发送用户名阶段
        STEP_SENDING_PASSWORD   // 发送密码阶段
    };

    OperationStep currentStep;
};

#endif // NETWORKMANAGER_H
