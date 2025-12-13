#ifndef VALIDATORS_H
#define VALIDATORS_H

#include <QString>
#include <QRegularExpression>
#include <QHostAddress>

class Validators
{
public:
    // 验证IP地址格式
    static bool isValidIP(const QString &ip);

    // 验证端口号 (1-65535)
    static bool isValidPort(const QString &port);

    // 验证端口号 (int类型)
    static bool isValidPort(int port);

    // 验证用户名 (4-20位字母数字)
    static bool isValidUsername(const QString &username);

    // 验证密码 (6-20位，至少包含字母和数字)
    static bool isValidPassword(const QString &password);

    // 获取验证错误消息
    static QString getValidationMessage(const QString &field, const QString &value);

    // 完整表单验证
    static bool validateLoginForm(const QString &ip, const QString &port,
                                  const QString &username, const QString &password,
                                  QString &errorMessage);
};

#endif // VALIDATORS_H
