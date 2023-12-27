#ifndef TASK_H
#define TASK_H

#include <QWidget>
#include <QList>
#include <QDateTime>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

struct Tasks
{
    Tasks(QList<QString> _files, QString _bakName,
          QString _freq, QString _isCode,
          QString _localPath, QDateTime _nextTime) : files(_files),
        bakName(_bakName), freq(_freq), isCode(_isCode),localPath(_localPath),
        nextTime(_nextTime){}
    QList<QString> files;
    QString bakName;
    QString freq;
    QString isCode;
    QString localPath;
    QDateTime nextTime;
};

class Task
{
public:
    Task();
    void init();
    void addTask(Tasks task);
    void deleteTask(int index);
    void clear();
    void updateTime(int index, QDateTime nextTime);
    const QList<Tasks>& getTaskList();
    void writeJson();
    QList<QString> getTaskFiles();
    int getTaskNum();
private:
    QList<Tasks> taskList;
    QJsonDocument config;
};


#endif // TASK_H
