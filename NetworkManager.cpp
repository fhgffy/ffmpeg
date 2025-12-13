// NetworkManager.cpp - 主要实现TCP通信
#include "NetworkManager.h"
#include "CryptString.h"
#include <QHostAddress>
#include <QDebug>
#include <QByteArray>
#include <string>
#include <QThread>

// 静态转换函数
std::string NetworkManager::qStringToStdString(const QString &str)
{
    // 使用toUtf8确保正确处理中文，然后转换为std::string
    QByteArray utf8Data = str.toUtf8();
    return std::string(utf8Data.constData(), utf8Data.length());
}

QString NetworkManager::stdStringToQString(const std::string &str)
{
    return QString::fromUtf8(str.c_str(), str.length());
}

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , tcpSocket(nullptr)
    , timeoutTimer(nullptr)
    , isConnecting(false)
    , isRegistering(false)
    , isLoggingIn(false)
    , currentStep(STEP_NONE)
{
    tcpSocket = new QTcpSocket(this);
    timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);

    connect(tcpSocket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &NetworkManager::onError);
    connect(timeoutTimer, &QTimer::timeout, this, &NetworkManager::onTimeout);
}

NetworkManager::~NetworkManager()
{
    disconnectFromServer();
}

void NetworkManager::connectToServer(const QString &ip, int port)
{
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->disconnectFromHost();
        QThread::msleep(100);  // 等待断开完成
    }

    currentIp = ip;
    currentPort = port;
    isConnecting = true;
    currentStep = STEP_CONNECTING;

    qDebug() << "尝试连接到服务器:" << ip << ":" << port;
    tcpSocket->connectToHost(ip, port);
    timeoutTimer->start(5000);  // 5秒超时
}

void NetworkManager::disconnectFromServer()
{
    if (tcpSocket) {
        tcpSocket->disconnectFromHost();
    }
}

void NetworkManager::onConnected()
{
    qDebug() << "成功连接到服务器:" << currentIp << ":" << currentPort;
    timeoutTimer->stop();
    isConnecting = false;

    // 连接成功后，立即发送用户名（第一步）
    if (isRegistering || isLoggingIn) {
        currentStep = STEP_SENDING_USERNAME;

        if (isRegistering) {
            qDebug() << "发送注册第一步：用户名" << currentUsername;
            sendTLVMessage(TASK_REGISTER_SECTION1, currentUsername);  // 使用QString版本
        } else if (isLoggingIn) {
            qDebug() << "发送登录第一步：用户名" << currentUsername;
            sendTLVMessage(TASK_LOGIN_SECTION1, currentUsername);  // 使用QString版本
        }
    } else {
        // 测试连接 - 现在可以直接使用const char*
        qDebug() << "发送测试连接消息";
        sendTLVMessage(TASK_TEST_CONNECTION, "TEST");  // 这会自动调用const char*版本
    }
}

void NetworkManager::onDisconnected()
{
    qDebug() << "与服务器断开连接";
    isConnecting = false;
    isRegistering = false;
    isLoggingIn = false;
    currentStep = STEP_NONE;

    // 清理临时数据
    currentPassword.clear();
    receivedSalt.clear();
}

void NetworkManager::onReadyRead()
{
    qDebug() << "接收到数据，可用字节数:" << tcpSocket->bytesAvailable();

    while (tcpSocket->bytesAvailable() >= sizeof(TLVHeader)) {
        // 读取TLV头部
        TLVHeader header;
        qint64 headerSize = tcpSocket->read((char*)&header, sizeof(TLVHeader));

        if (headerSize != sizeof(TLVHeader)) {
            qDebug() << "读取TLV头部失败";
            return;
        }

        qDebug() << "收到消息，类型:" << header.type << "长度:" << header.length;

        // 检查是否有足够的数据
        if (tcpSocket->bytesAvailable() < header.length) {
            qDebug() << "数据不完整，等待更多数据";
            return;
        }

        // 读取数据部分 - 使用std::string确保与服务端兼容
        QByteArray data = tcpSocket->read(header.length);
        std::string content(data.constData(), data.length());

        qDebug() << "收到完整消息，类型:" << header.type
                 << "内容长度:" << content.length();

        // 根据消息类型处理
        switch (header.type) {
        case TASK_REGISTER_SECTION1_RESP_OK:
            handleRegister1Response(content);
            break;
        case TASK_REGISTER_SECTION1_RESP_ERROR:
            qDebug() << "注册失败: 用户名已存在";
            if (currentCallback) {
                currentCallback(false, "用户名已存在");
                emit registrationFailed("用户名已存在");
            }
            break;
        case TASK_REGISTER_SECTION2_RESP_OK:
            handleRegister2Response(content);
            break;
        case TASK_LOGIN_SECTION1_RESP_OK:
            handleLogin1Response(content);
            break;
        case TASK_LOGIN_SECTION1_RESP_ERROR:
            qDebug() << "登录失败: 用户名不存在";
            if (currentCallback) {
                currentCallback(false, "用户名不存在");
                emit loginFailed("用户名不存在");
            }
            break;
        case TASK_LOGIN_SECTION2_RESP_OK:
            handleLogin2Response(content);
            break;
        case TASK_LOGIN_SECTION2_RESP_ERROR:
            qDebug() << "登录失败: 密码错误";
            if (currentCallback) {
                currentCallback(false, "密码错误");
                emit loginFailed("密码错误");
            }
            break;
        case TASK_TEST_CONNECTION_RESP:
            handleTestConnectionResponse(content);
            break;
        default:
            qDebug() << "未知的消息类型:" << header.type;
        }
    }
}

void NetworkManager::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString errorStr = tcpSocket->errorString();
    qDebug() << "网络错误:" << errorStr;

    timeoutTimer->stop();
    currentStep = STEP_NONE;

    if (currentCallback) {
        currentCallback(false, "网络错误: " + errorStr);
    }
}

void NetworkManager::onTimeout()
{
    qDebug() << "连接超时";

    if (tcpSocket->state() == QAbstractSocket::ConnectingState) {
        tcpSocket->abort();
    }

    currentStep = STEP_NONE;

    if (currentCallback) {
        currentCallback(false, "连接超时");
    }
}

void NetworkManager::sendTLVMessage(int type, const std::string &data)
{
    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "无法发送消息：未连接到服务器";
        if (currentCallback) {
            currentCallback(false, "未连接到服务器");
        }
        return;
    }

    QByteArray message;

    // 创建TLV结构
    TLVHeader header;
    header.type = type;
    header.length = data.length();  // 使用std::string的length()

    // 发送TLV头部
    message.append((char*)&header, sizeof(TLVHeader));
    // 发送数据部分 - 直接使用std::string的数据
    message.append(data.c_str(), data.length());

    qint64 bytesWritten = tcpSocket->write(message);
    bool flushed = tcpSocket->flush();

    qDebug() << "发送TLV消息，类型:" << type
             << "长度:" << header.length
             << "写入字节数:" << bytesWritten
             << "刷新成功:" << flushed;
}

void NetworkManager::sendTLVMessage(int type, const QString &data)
{
    // 转换为std::string确保与服务端兼容
    sendTLVMessage(type, qStringToStdString(data));
}

void NetworkManager::sendTLVMessage(int type, const char *data)
{
    // 将const char*转换为std::string
    if (data == nullptr) {
        sendTLVMessage(type, std::string(""));
    } else {
        sendTLVMessage(type, std::string(data));
    }
}

void NetworkManager::login(const QString &ip, int port,
                          const QString &username, const QString &password,
                          std::function<void(bool, const QString&)> callback)
{
    qDebug() << "开始登录流程";

    // 重置状态
    isLoggingIn = true;
    isRegistering = false;
    currentStep = STEP_NONE;

    // 保存用户凭据
    currentUsername = username;
    currentPassword = password;  // 临时存储，第二步时使用
    currentCallback = callback;

    // 清空之前的数据
    receivedSalt.clear();

    // 第一步：连接到服务器
    connectToServer(ip, port);
}

void NetworkManager::registerUser(const QString &ip, int port,
                                 const QString &username, const QString &password,
                                 std::function<void(bool, const QString&)> callback)
{
    qDebug() << "开始注册流程";

    // 重置状态
    isRegistering = true;
    isLoggingIn = false;
    currentStep = STEP_NONE;

    // 保存用户凭据
    currentUsername = username;
    currentPassword = password;  // 临时存储，第二步时使用
    currentCallback = callback;

    // 清空之前的数据
    receivedSalt.clear();

    // 第一步：连接到服务器
    connectToServer(ip, port);
}

void NetworkManager::testConnection(const QString &ip, int port,
                                   std::function<void(bool, const QString&)> callback)
{
    qDebug() << "测试连接";

    // 重置状态
    isLoggingIn = false;
    isRegistering = false;
    currentStep = STEP_NONE;
    currentCallback = callback;

    // 连接到服务器
    connectToServer(ip, port);
}

std::string NetworkManager::generateEncryptedPassword(const QString &password, const QString &salt)
{
    CryptString crypt(password);
    QString encrypted = crypt.generateEncryptedString(salt);

    // 转换为std::string确保与服务端兼容
    return qStringToStdString(encrypted);
}

void NetworkManager::handleRegister1Response(const std::string &data)
{
    // 服务器返回盐值
    receivedSalt = stdStringToQString(data);
    qDebug() << "收到盐值:" << receivedSalt;

    currentStep = STEP_SENDING_PASSWORD;

    // 第二步：用盐值和密码生成加密密文并发送
    std::string encryptedPassword = generateEncryptedPassword(currentPassword, receivedSalt);
    qDebug() << "生成加密密码，长度:" << encryptedPassword.length();
    sendTLVMessage(TASK_REGISTER_SECTION2, encryptedPassword);

    // 清理临时密码
    currentPassword.clear();
}

void NetworkManager::handleRegister2Response(const std::string &data)
{
    Q_UNUSED(data);
    qDebug() << "注册成功";

    currentStep = STEP_NONE;

    if (currentCallback) {
        currentCallback(true, "注册成功");
        emit registrationSuccess();
    }
}

void NetworkManager::handleLogin1Response(const std::string &data)
{
    // 服务器返回盐值
    receivedSalt = stdStringToQString(data);
    qDebug() << "收到盐值:" << receivedSalt;

    currentStep = STEP_SENDING_PASSWORD;

    // 第二步：用盐值和密码生成加密密文并发送
    std::string encryptedPassword = generateEncryptedPassword(currentPassword, receivedSalt);
    qDebug() << "生成加密密码，长度:" << encryptedPassword.length();
    sendTLVMessage(TASK_LOGIN_SECTION2, encryptedPassword);

    // 清理临时密码
    currentPassword.clear();
}

void NetworkManager::handleLogin2Response(const std::string &data)
{
    Q_UNUSED(data);
    qDebug() << "登录成功";

    currentStep = STEP_NONE;

    if (currentCallback) {
        currentCallback(true, "登录成功");
        emit loginSuccess();
    }
}

void NetworkManager::handleTestConnectionResponse(const std::string &data)
{
    qDebug() << "测试连接成功:" << stdStringToQString(data);

    currentStep = STEP_NONE;

    if (currentCallback) {
        currentCallback(true, "连接成功");
        emit connectionTested(true, "连接成功");
    }
}
