#include "pack.h"
#include<QString>
#include<QFile>
#include<QFileInfo>
#include<QDirIterator>
#include<QDebug>
#include<QJsonArray>
#include<QJsonObject>
#include<QJsonDocument>

Pack::Pack()
{
    return;
}

int Pack::pack_to_tar(QStringList filelist, QString path,QString password) {
    if (!QFileInfo("config.json").exists()) {
        QFile file("config.json");
        file.open(QFile::WriteOnly);
        file.write("");
        file.close();
    }
    QFile tarfile(path);
    bool flag = tarfile.open(QFile::WriteOnly);
    if (!flag) return 1;//打开备份地址失败
    auto root = QFileInfo(filelist[0]).path();

    QFile file_json("config.json");
    file_json.open(QFile::WriteOnly);
    QJsonArray jsonarray;
    QJsonObject information;
    QJsonArray files;

    for (auto& file : filelist) {

        files.append(file);
        qDebug()<<file;
        if (QFileInfo(file).isFile()) {
            auto ra = QString(file).replace(root, "");//相对地址
            int pathLength = ra.length();//路径长度
            int label = 0;//文件类别
            int filesize = QFileInfo(file).size();//文件大小

            tarfile.write((const char*)&pathLength, sizeof (pathLength));
            tarfile.write(ra.toStdString().c_str());
            tarfile.write((const char*)&label, sizeof (label));
            tarfile.write((const char*)&filesize, sizeof (filesize));
            QFile data(file);
            flag = data.open(QFile::ReadOnly);
            if (!flag) return 2;//打开文件失败
            tarfile.write(data.readAll());
            data.close();
            files.append(file);
        } else {
            //目录文件
            QDirIterator iter(file, QDirIterator::Subdirectories);
            qDebug()<<"2";
            while (iter.hasNext()) {
                iter.next();
                QFileInfo info = iter.fileInfo();
                if (info.fileName() == "..") continue;
                if (info.isDir()) {
                    qDebug()<<info.absoluteFilePath();
                    auto ra = QString(info.absoluteFilePath()).replace(root, "");
                    //qDebug() << ra.toStdString().c_str() << ra.length();
                    int pathLength = ra.length();
                    int filesize = 0;
                    int label = 1;
                    tarfile.write((const char*)&pathLength, sizeof (pathLength));
                    tarfile.write(ra.toStdString().c_str());
                    tarfile.write((const char*)&label, sizeof (label));
                    tarfile.write((const char*)&filesize, sizeof (filesize));
                } else {
                    qDebug()<<info.absoluteFilePath();
                    auto ra = QString(info.absoluteFilePath()).replace(root, "");
                    int pathLength = ra.length();
                    int filesize = info.size();
                    int label = 0;

                    tarfile.write((const char*)&pathLength, sizeof (pathLength));
                    tarfile.write(ra.toStdString().c_str());
                    tarfile.write((const char*)&label, sizeof (label));
                    tarfile.write((const char*)&filesize, sizeof (filesize));

                    QFile data(info.absoluteFilePath());
                    bool success = data.open(QFile::ReadOnly);
                    if (!success) return 2;
                    tarfile.write(data.readAll());
                    data.close();
                }
            }
        }
    }

    information["files"] = files;
    information["backupFilename"] = root+"/"+path.left(path.length() - 3) + "bak";
    information["password"] = password;
    jsonarray.append(information);
    file_json.write(QJsonDocument(jsonarray).toJson());
    file_json.close();
    tarfile.close();
    return 0;
}

int Pack::unpack_tar(QString tarFilename, QString path){
    QFile tarfile(tarFilename);
    bool flag = tarfile.open(QFile::ReadOnly);
    if (!flag) return 1;
    int pathLength;
    int label;
    int filesize;

    QString ra;
    while (tarfile.read((char*)&pathLength, sizeof (pathLength))) {
        char* _relativePath = new char[pathLength + 1];
        tarfile.read(_relativePath, pathLength);
        _relativePath[pathLength] = '\0';
        ra = QString::fromStdString(std::string(_relativePath));
        delete[] _relativePath;
        tarfile.read((char*)&label, sizeof (label));
        tarfile.read((char*)&filesize, sizeof (filesize));
        if (!label) {

            QFile data(path + "/" + ra);
            flag = data.open(QFile::WriteOnly);
            if (!flag) return 2;
            if (filesize) {
                char* content = new char[filesize];
                tarfile.read(content, filesize);
                data.write(content, filesize);
                delete[] content;
            } else {
                data.write("");
            }
            data.close();
        } else {
            QDir dir;
            if (QFileInfo(path + "/" + ra).exists()) continue;
            flag = dir.mkdir(path + "/" + ra);
            if (!flag) return 3;
        }
    }

    return 0;
}
