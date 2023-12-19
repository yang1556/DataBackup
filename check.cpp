#include "check.h"
#include<QFileInfo>
#include<QDirIterator>
#include<QDebug>
#include<QApplication>

Check::Check()
{

}
bool Check::check(QList<QString> files, QString directory) {
    if (files.empty()) return true;
    auto root = QFileInfo(files[0]).path();
    qDebug()<<root;
    for (const auto& file : files) {
        if (!QFileInfo(file).exists()) {
            return false;
        }
        if (QFileInfo(file).isDir()) {
            QDirIterator iter(file, QDirIterator::Subdirectories);
            QDirIterator iter_2(directory+QFileInfo(file).filePath().replace(root,""),QDirIterator::Subdirectories);
            while (iter.hasNext()&&iter_2.hasNext()) {
                iter.next();
                iter_2.next();
                QFileInfo info = iter.fileInfo();
                QFileInfo info2 = iter_2.fileInfo();
                //qDebug()<<info2.path();
                if (info.fileName() == "." || info.fileName() == "..") continue;
                auto relativePath = info.absoluteFilePath().replace(root, "");
                QString root2=(QApplication::applicationDirPath()+"/TEMP");
                root2=root2.replace("/debug","");
                auto relativePath_2 = info2.absoluteFilePath().replace(root2, "");
                auto path = directory +relativePath;
                qDebug()<<relativePath<<relativePath_2<<(relativePath==relativePath_2)<<QApplication::applicationDirPath()+"/TEMP"<<info.absoluteFilePath()<<root<<info2.absoluteFilePath();
                if (relativePath!=relativePath_2) {
                    return false;
                } else if (info.isFile()&&Check::getMD5ByFilename(info.absoluteFilePath()) != Check::getMD5ByFilename(path)) {
                    return false;
                }
            }
        } else {
            auto relativePath = QString(file).replace(root, "");
            auto path = directory + relativePath;
            if (!QFileInfo(path).exists()) {
                return false;
            } else if (Check::getMD5ByFilename(file) != Check::getMD5ByFilename(path)) {
                return false;
            }
        }
    }
    return true;

}
