#include "cryptstring.h"
#include <QDebug>
#include <QDateTime> // [監控項目依賴] 用於時間戳

// ==========================================
// Part 1: CryptString 實現 (合併版)
// ==========================================

CryptString::CryptString(const QString &data) : _data(data) {}
CryptString::CryptString() : _data("") {}

// [登錄項目功能] 生成隨機鹽
QString CryptString::generateSalt(int length)
{
    const QString possibleCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QString salt;
    salt.reserve(length);
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(possibleCharacters.length());
        salt.append(possibleCharacters.at(index));
    }
    return salt;
}

// [兼容功能] 生成 MD5
// 監控項目調用時 salt 為空，效果等同於直接對 _data 做 MD5，邏輯正確
QString CryptString::generateMD5(const QString &salt) const
{
    QString dataToHash = _data + salt;
    QByteArray arr = QCryptographicHash::hash(dataToHash.toUtf8(), QCryptographicHash::Md5);
    return arr.toHex();
}

// [登錄項目功能] 生成存儲用的加密串
QString CryptString::generateEncryptedString(const QString &salt) const
{
    QString hash = generateMD5(salt);
    // 格式: $1$鹽值$哈希值
    return QString("%1$%2").arg(salt).arg(hash);
}

// [登錄項目功能] 解析加密串
bool CryptString::parseEncryptedString(const QString &encryptedStr, QString &algorithm, QString &salt, QString &hash)
{
    QStringList parts = encryptedStr.split('$', Qt::SkipEmptyParts);
    if (parts.size() < 3) return false;

    algorithm = parts[0];
    salt = parts[1];
    hash = parts[2];
    return true;
}

// [登錄項目功能] 驗證密碼
bool CryptString::verifyPassword(const QString &password, const QString &encryptedStr)
{
    QString algorithm, salt, storedHash;
    if (!parseEncryptedString(encryptedStr, algorithm, salt, storedHash)) {
        return false;
    }

    // 算法標識 "1" 代表我們自定義的加鹽MD5邏輯
    if (algorithm != "1") {
        return false;
    }

    CryptString crypt(password);
    QString computedHash = crypt.generateMD5(salt);
    return (computedHash == storedHash);
}

// ==========================================
// Part 2: KVQuery 實現 (完全來自監控項目)
// ==========================================

KVQuery::KVQuery()
{
    // [重要] 保留了監控項目的密鑰
    _map.insert(std::make_pair("secret", "f6fdffe48c908deb0f4c3bd36c032e72"));

    // [重要] 保留了時間戳生成
    QDateTime currentDateTime = QDateTime::currentDateTime();
    string secstr = std::to_string(currentDateTime.toSecsSinceEpoch());
    _map.insert(std::make_pair("t", secstr));
}

void KVQuery::add(const std::string &key, const std::string &value)
{
    _map.insert(std::make_pair(key, value));
}

void KVQuery::erase(const std::string &key)
{
    _map.erase(key);
}

void KVQuery::clear()
{
    _map.clear();
}

string KVQuery::toString() const
{
    string tmp;
    for(auto & elem : _map) {
        tmp.append(elem.first).append("=").append(elem.second).append("&");
    }
    if (!tmp.empty()) tmp.pop_back(); // 移除最後一個 '&'
    return tmp;
}

string KVQuery::toCrpytString()
{
    // 調用 CryptString 計算簽名
    CryptString cryptStr(QString::fromStdString(toString()));

    // 這裡調用 generateMD5() 使用了默認參數(空鹽值)，
    // 等同於監控項目原來的邏輯：MD5(secret + params + timestamp)
    QString token = cryptStr.generateMD5();

    this->add("token", token.toStdString());
    this->erase("secret"); // 生成URL時移除密鑰，符合安全規範
    return toString();
}
