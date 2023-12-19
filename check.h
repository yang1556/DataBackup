#ifndef CHECK_H
#define CHECK_H
#include<QList>
#include<QCryptographicHash>
#include<QFile>
class Check
{
public:
    Check();
    //检查文件是否一致
    static bool check(QList<QString> files, QString directory);
    //获得md5码
    static QByteArray getMD5ByFilename(QString filename) {
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QByteArray md5 = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
        file.close();
        return md5.toHex();
    }
};

#endif // CHECK_H
