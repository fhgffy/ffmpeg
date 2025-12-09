#ifndef CRYPTSTRING_H
#define CRYPTSTRING_H

#include <QString>
#include <map>
#include <string>

using std::map;
using std::string;

class CryptString
{
public:
    CryptString(const QString & data);

    QString generateMD5() const;
private:
    QString _data;
};

class KVQuery
{
public:
    KVQuery();
    void add(const string & key, const string & value);
    void erase(const string & key);
    void clear();

    string toString() const;
    string toCrpytString();
private:
    map<string, string> _map;
};

#endif // CRYPTSTRING_H
