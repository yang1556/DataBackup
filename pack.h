#ifndef PACK_H
#define PACK_H
#include<QString>

class Pack
{
public:
    Pack();
    //打包
    static int pack_to_tar(QStringList files, QString destination,QString password);
    //解包
    static int unpack_tar(QString tarFilename, QString destination);

};

#endif // PACK_H
