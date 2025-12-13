#include "Validator.h"

bool Validators::isValidIP(const QString &ip)
{
    QHostAddress address;
    return address.setAddress(ip) && !address.isNull();
}

bool Validators::isValidPort(const QString &port)
{
    bool ok;
    int portNum = port.toInt(&ok);
    return ok && portNum > 0 && portNum <= 65535;
}

bool Validators::isValidPort(int port)
{
    return port > 0 && port <= 65535;
}

bool Validators::isValidUsername(const QString &username)
{
    // 用户名：4-20位字母数字
    QRegularExpression regex("^[a-zA-Z0-9]{4,20}$");
    return regex.match(username).hasMatch();
}

bool Validators::isValidPassword(const QString &password)
{
    // 密码：6-20位，至少包含字母和数字
    QRegularExpression regex("^(?=.*[a-zA-Z])(?=.*\\d)[a-zA-Z\\d]{6,20}$");
    return regex.match(password).hasMatch();
}

QString Validators::getValidationMessage(const QString &field, const QString &value)
{
    if (field == "IP地址") {
        if (value.isEmpty()) return "IP地址不能为空";
        if (!isValidIP(value)) return "请输入有效的IP地址";
    }
    else if (field == "端口") {
        if (value.isEmpty()) return "端口不能为空";
        if (!isValidPort(value)) return "端口号必须在1-65535之间";
    }
    else if (field == "用户名") {
        if (value.isEmpty()) return "用户名不能为空";
        if (!isValidUsername(value)) return "用户名必须是4-20位字母数字";
    }
    else if (field == "密码") {
        if (value.isEmpty()) return "密码不能为空";
        if (!isValidPassword(value)) return "密码必须是6-20位，且包含字母和数字";
    }
    return "";
}

bool Validators::validateLoginForm(const QString &ip, const QString &port,
                                   const QString &username, const QString &password,
                                   QString &errorMessage)
{
    QStringList fields = {"IP地址", "端口", "用户名", "密码"};
    QStringList values = {ip, port, username, password};

    for (int i = 0; i < fields.size(); ++i) {
        QString msg = getValidationMessage(fields[i], values[i]);
        if (!msg.isEmpty()) {
            errorMessage = QString("%1: %2").arg(fields[i]).arg(msg);
            return false;
        }
    }

    return true;
}
