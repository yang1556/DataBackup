#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "task.h"
#include<QTimer>
#include<QDatetime>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void showTasks(Tasks t);

private:
    Ui::MainWindow *ui;
    Task taskManager;
    QTimer timer;
    bool once, day, week;
};
#endif // MAINWINDOW_H
