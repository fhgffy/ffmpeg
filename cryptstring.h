#ifndef CRYPTSTRING_H
#define CRYPTSTRING_H

#include <QString>
#include <QCryptographicHash>
#include <QByteArray>
#include <QRandomGenerator> // [登錄項目依賴] 用於生成隨機鹽
#include <map>              // [監控項目依賴] 用於 KVQuery
#include <string>           // [監控項目依賴] 用於 KVQuery

using std::map;
using std::string;

// 1. CryptString 類：以【登錄項目】為基礎，兼容監控項目
class CryptString
{
public:
    // [登錄+監控] 構造函數
    CryptString(const QString &data);
    CryptString();

    // [登錄項目專屬] 用於註冊時生成鹽值
    static QString generateSalt(int length = 8);

    // [兼容核心] 生成MD5
    // 登錄項目調用時會傳入 salt；
    // 監控項目調用時不傳參，默認 salt=""，即生成普通MD5
    QString generateMD5(const QString &salt = "") const;

    // [登錄項目專屬] 生成最終加密字符串 ($1$salt$hash)
    QString generateEncryptedString(const QString &salt = "") const;

    // [登錄項目專屬] 用於校驗密碼
    static bool parseEncryptedString(const QString &encryptedStr, QString &algorithm, QString &salt, QString &hash);
    static bool verifyPassword(const QString &password, const QString &encryptedStr);

private:
    QString _data;
};

// 2. KVQuery 類：完全來自【監控項目】，用於API簽名
class KVQuery
{
public:
    KVQuery();
    void add(const string & key, const string & value);
    void erase(const string & key);
    void clear();

    string toString() const;
    string toCrpytString(); // 生成帶Token的查詢字符串
private:
    map<string, string> _map;
};

#endif // CRYPTSTRING_H
