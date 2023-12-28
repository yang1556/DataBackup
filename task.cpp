#include "task.h"

Task::Task()
{

}

void Task::init()
{
    if (QFileInfo("hzyconfig.json").exists()) {
        QFile file("hzyconfig.json");
        file.open(QFile::ReadWrite);
        config = QJsonDocument::fromJson(file.readAll());
        file.close();
        QJsonArray tasks = config.array();
        for (auto task : tasks) {
            auto information = task.toObject();
            QList<QString> files;
            for (auto file : information["files"].toArray()) {
                files.append(file.toString());
            }
            taskList.append(Tasks(files,
                                  information["backupFileName"].toString(),
                                  information["frequency"].toString(),
                                  information["password"].toString(),
                                  information["backupFilePath"].toString(),
                                  QDateTime::fromString(information["nextTime"].toString())));
        }
    } else {
        QFile file("hzyconfig.json");
        file.open(QFile::WriteOnly);
        file.write("");
        file.close();
    }
}

void Task::addTask(Tasks task)
{
    taskList.append(task);
    writeJson();
}

void Task::deleteTask(int index)
{
    taskList.removeAt(index);
    writeJson();
}

void Task::clear()
{
    taskList.clear();
    QFile file("hzyconfig.json");
    file.open(QFile::WriteOnly);
    file.write("");
    file.close();
}

void Task::updateTime(int index, QDateTime nextTime)
{
    taskList[index].nextTime = nextTime;
}

const QList<Tasks> &Task::getTaskList()
{
    return taskList;
}
QList<QString> Task::getTaskFiles(){
    QList<QString> files;
    for(auto task : taskList){
        files.append(task.files);
    }
    return files;
}
int Task::getTaskNum(){
    return taskList.size();
}
void Task::writeJson()
{
    qDebug() << "writeJson";
    QFile file("hzyconfig.json");
    QJsonArray tasksJson;
    for (auto task : taskList) {
        QJsonObject taskjson;
        //task.files
        QJsonArray filejson;
        for (auto f : task.files) {
            filejson.append(f);
        }
        taskjson["files"] = filejson;
        taskjson["backupFileName"] = task.bakName;
        taskjson["frequency"] = task.freq;
        taskjson["password"] = task.isCode;
        taskjson["backupFilePath"] = task.localPath;
        taskjson["nextTime"] = task.nextTime.toString();
        tasksJson.append(taskjson);
    }
    if (file.open(QFile::WriteOnly)) {
        QJsonDocument doc;
        doc.setArray(tasksJson);
        file.write(doc.toJson());
        file.close();
    }
}
