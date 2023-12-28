#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QFileDialog>
#include<QMessageBox>
#include<QCheckBox>
#include <QDebug>
#include <QListView>
#include <regex>
#include <QDesktopServices>
#include <QProcess>
#include "pack.h"
#include "compressor.h"
#include<QJsonDocument>
#include<QJsonArray>
#include<QJsonObject>
#include"check.h"
#include<QButtonGroup>
#include<QTableWidgetItem>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);

    QButtonGroup* buttonGroup=new QButtonGroup(this);
    buttonGroup->addButton(ui->OnceBox);
    buttonGroup->addButton(ui->DayBox_2);
    buttonGroup->addButton(ui->MinBox);
    buttonGroup->setExclusive(true);


    taskManager.init();
    
    


    //退出按钮
    connect(ui->ExitButton,&QPushButton::clicked,[=](){
        if (!(QMessageBox::information(this,tr("exit tip"),tr("Do you really want exit ?"),tr("Yes"),tr("No")))){
                   QApplication* app;
                   app->exit(0);
             }
    });

    //切换到备份页面
    connect(ui->BackupButton,&QPushButton::clicked,[=](){
        ui->stackedWidget->setCurrentIndex(2);
    });

    //切换到恢复页面
    connect(ui->RestoreButton,&QPushButton::clicked,[=](){
        ui->stackedWidget->setCurrentIndex(3);
    });

    //切换到管理页面
    connect(ui->manageButton, &QPushButton::clicked, [=]() {
        ui->stackedWidget->setCurrentIndex(1);
        int RowCount = ui->tableWidget->rowCount();
        for (int i = RowCount - 1; i >= 0; i--)
        {
            ui->tableWidget->removeRow(i);
        }
        for (const auto& t : taskManager.getTaskList()) { showTasks(t); }
        });
    //删除task
    connect(ui->DeleteTaskButton, &QPushButton::clicked, [=] {
        int index = ui->tableWidget->currentRow();
        if (index != -1)
        {
            taskManager.deleteTask(index);
            ui->tableWidget->removeRow(index);
        }
        });

    //清空task
    connect(ui->ClearTaskButton, &QPushButton::clicked, [=] {
        int RowCount = ui->tableWidget->rowCount();
        for (int i = RowCount - 1; i >= 0; i--)
        {
            ui->tableWidget->removeRow(i);
        }
        taskManager.clear();
        });
    

    //返回主界面
    connect(ui->ReturnButton_1,&QPushButton::clicked,[=](){
        ui->stackedWidget->setCurrentIndex(0);
    });
    connect(ui->ReturnButton_2,&QPushButton::clicked,[=](){
        ui->stackedWidget->setCurrentIndex(0);
    });
    connect(ui->ReturnButton_3, &QPushButton::clicked, [=]() {
        ui->stackedWidget->setCurrentIndex(0);
        });
    //添加目录文件
    connect(ui->AddCatalogButton,&QPushButton::clicked,[=](){
        //打开文件目录
        QString catalog = QFileDialog::getExistingDirectory(this,tr("请选择一个文件夹"),"/home",
                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (catalog != "") {
            // 去重
            for (int i = 0; i < ui->FileList->topLevelItemCount(); ++i) {
                if (ui->FileList->topLevelItem(i)->text(1) == catalog)
                    return;
            }
            QTreeWidgetItem* NewItem = new QTreeWidgetItem;
            NewItem->setText(0, QFileInfo(catalog).fileName());
            NewItem->setText(1, catalog);
            ui->FileList->addTopLevelItem(NewItem);
        }
        if (!ui->FileList->currentItem() && ui->FileList->topLevelItemCount()) {
            ui->FileList->setCurrentItem(ui->FileList->topLevelItem(0));
        }
    });

    //添加文件
    connect(ui->AddFileButton,&QPushButton::clicked,[=]{
        QStringList files = QFileDialog::getOpenFileNames(this,QStringLiteral("选择一个或多个文件"),"/home",QStringLiteral("所有文件 (*.*)"));
        for (const auto& file : files) {
            // 去重
            bool duplication = false;
            for (int i = 0; i < ui->FileList->topLevelItemCount(); ++i) {
                if (ui->FileList->topLevelItem(i)->text(1) == file) {
                    duplication = true;
                    break;
                }
            }
            if (duplication) continue;
            QTreeWidgetItem* fileItem = new QTreeWidgetItem;
            fileItem->setText(0, QFileInfo(file).fileName());
            fileItem->setText(1, file);
            ui->FileList->addTopLevelItem(fileItem);
        }
        if (!ui->FileList->currentItem() && ui->FileList->topLevelItemCount()) {
            ui->FileList->setCurrentItem(ui->FileList->topLevelItem(0));
        }
    });

    //删除
    connect(ui->DeleteButton,&QPushButton::clicked,[=]{
        if(ui->FileList->currentItem()){
            delete ui->FileList->currentItem();
        }
    });

    //清空
    connect(ui->ClearButton,&QPushButton::clicked,[=]{
        while(ui->FileList->currentItem()){
            delete ui->FileList->currentItem();
        }
    });

    //选择备份路径
    connect(ui->BackupPathToolButton,&QToolButton::clicked,[=]{
        QString path = QFileDialog::getExistingDirectory(this,tr("备份到"),"\home",
                                                         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (path != "") {
            ui->lineFilePath->setText(path);
        }
    });

    //选择恢复路径
    connect(ui->RestorePathToolButton,&QToolButton::clicked,[=]{
        QString path = QFileDialog::getExistingDirectory(this,tr("恢复到"),"\home",
                                                         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (path != "") {
            ui->RestorePathLine->setText(path);
        }
    });

    //选择要恢复的文件
    connect(ui->RestoretoolButton,&QToolButton::clicked,[=]{
        auto file = QFileDialog::getOpenFileName(this,QStringLiteral("选择一个备份文件"),"",QStringLiteral("所有文件 (*.*)"));
        if (file != "") {
            ui->RestoreFileNameLine->setText(file);
        }
    });

    //是否使用密码
    connect(ui->PasswordBox,&QCheckBox::stateChanged,[=]{
        ui->linePassword->setEnabled(ui->PasswordBox->checkState());
    });
    connect(ui->PasswordBox_2,&QCheckBox::stateChanged,[=]{
        ui->linePassword_2->setEnabled(ui->PasswordBox_2->checkState());
    });

    //开始备份
    connect(ui->StartBackupButton,&QPushButton::clicked,[=]{
        if (!ui->FileList->topLevelItemCount()) {
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请添加需要备份的文件。"),
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }
        if (ui->lineFileName->text().trimmed() == "") {
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入备份后的文件名。"),
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }
        if (ui->lineFilePath->text().trimmed() == "") {
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入备份保存到的目录。"),
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }
        std::regex filenameExpress("[\\/:*?\"<>|]");
        if (std::regex_search(ui->lineFileName->text().toStdString(), filenameExpress)) {
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入合法的备份文件名。"),
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }
        if (QFileInfo(ui->lineFilePath->text() + "\\" + ui->lineFileName->text()).exists()) {
            if (QMessageBox::Yes != QMessageBox::question(this, QStringLiteral("警告"), QStringLiteral("文件已存在，确认覆盖？"),
                                                          QMessageBox::Yes | QMessageBox::No)) {
                return;
            }
        }
        if (ui->PasswordBox->isChecked() && ui->linePassword->text().trimmed() == "") {
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入密码。"),
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }
        auto rootDirectory = QFileInfo(ui->FileList->topLevelItem(0)->text(1)).path();
        for (int i = 1; i < ui->FileList->topLevelItemCount(); ++i) {
            if (QFileInfo(ui->FileList->topLevelItem(i)->text(1)).path() != rootDirectory) {
                QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("选择的文件或文件夹应位于同一目录下。"),
                                         QMessageBox::Yes, QMessageBox::Yes);
                return;
            }
        }
        QStringList fileliststring;
        for (int i = 0; i < ui->FileList->topLevelItemCount(); ++i) {
            fileliststring.append(ui->FileList->topLevelItem(i)->text(1));
        }
        if (ui->DayBox_2->isChecked()||ui->MinBox->isChecked()) {
            //QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("。"),
            //    QMessageBox::Yes, QMessageBox::Yes);
            QList<QString> files;
            //files = taskManager.getTaskFiles();
            files = fileliststring.toList();
            QDateTime nextTime = ui->MinBox->isChecked()? QDateTime::currentDateTime().addSecs(60): QDateTime::currentDateTime().addDays(1);

            taskManager.addTask(Tasks(files,
                ui->lineFileName->text() + ".bak",
                ui->MinBox->isChecked() ? "每分钟":"每天",
                ui->PasswordBox ? ui->linePassword->text() : "",
                ui->lineFilePath->text() + "/" + ui->lineFileName->text() + ".bak",
                nextTime));
        }

        Pack *packtool = new Pack;
        int errorCode = packtool->pack_to_tar(fileliststring,
                                              ui->lineFileName->text() + ".tar",
                                              ui->PasswordBox->isChecked() ? ui->linePassword->text(): "");
        if (errorCode) {
            QStringList message = {"正常执行", QStringLiteral("目标文件打开失败"), QStringLiteral("打开源文件失败")};
            QMessageBox::information(this, QStringLiteral("提示"), message[errorCode],
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }
        Compressor compressor;
        errorCode = compressor.compress(ui->lineFileName->text().toStdString() + ".tar",
                                        ui->lineFilePath->text().toStdString() + "/",
                                        ui->PasswordBox->isChecked() ? ui->linePassword->text().toStdString() : "");
        if (errorCode) {
            QStringList message = {"正常执行", QStringLiteral("源文件扩展名不是bak"), QStringLiteral("打开源文件失败"), QStringLiteral("打开目标文件失败")};
            QMessageBox::information(this, QStringLiteral("提示"), message[errorCode],
                                     QMessageBox::Yes, QMessageBox::Yes);
            qDebug() << errorCode;
            return;
        }
        QFile tarFile(ui->lineFileName->text() + ".tar");
        //保存tarFile

        //
        tarFile.remove();
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("备份成功。"),
                                 QMessageBox::Yes, QMessageBox::Yes);
    });

    //开始恢复
    connect(ui->StartRestoreButton,&QPushButton::clicked,[=]{
        if (ui->RestoreFileNameLine->text().trimmed() == "") {
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择要恢复的备份文件。"),
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }
        if (ui->RestorePathLine->text().trimmed() == "") {
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请选择要恢复到的目录。"),
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }
        if (ui->PasswordBox_2->isChecked() && ui->linePassword_2->text().trimmed() == "") {
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入密码。"),
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }
        Compressor compressor;
        int errorCode = compressor.decompress(ui->RestoreFileNameLine->text().toStdString(),
                                                ui->RestorePathLine->text().toStdString() + "/",
                                                ui->PasswordBox_2->isChecked() ? ui->linePassword_2->text().toStdString() : "");
        if (errorCode) {
            QStringList message = {"正常执行", QStringLiteral("源文件扩展名不是bak"), QStringLiteral("打开源文件失败"), QStringLiteral("打开目标文件失败"), QStringLiteral("文件过短，频率表不完整"), QStringLiteral("文件结尾不完整"), QStringLiteral("密码错误"), QStringLiteral("解码错误")};
            QMessageBox::information(this, QStringLiteral("提示"), message[errorCode],
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }

        QString tempFilename = ui->RestorePathLine->text() + "/" + QFileInfo(ui->RestoreFileNameLine->text()).fileName();
        tempFilename = tempFilename.left(tempFilename.length() - 3) + "tar";

        errorCode = Pack::unpack_tar(tempFilename, ui->RestorePathLine->text());
        if (errorCode) {
            QStringList message = {QStringLiteral("正常执行"),  QStringLiteral("打开源文件失败"), QStringLiteral("目标文件打开失败"),QStringLiteral("创建目录失败")};
            QMessageBox::information(this, QStringLiteral("提示"), message[errorCode],
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }
        QFile tarFile(tempFilename);
        tarFile.remove();
        /*QDir::setCurrent(currentDirectory.path());*/
        QMessageBox::information(this, QStringLiteral("提示"),QStringLiteral("恢复完成。"),
                                 QMessageBox::Yes, QMessageBox::Yes);
        QDesktopServices::openUrl(QUrl("file:///" + ui->RestorePathLine->text(), QUrl::TolerantMode));
    });

    //检查文件是否有错
    connect(ui->CheckButton,&QPushButton::clicked,[=]{
        QFile old_file_list("config.json");
        old_file_list.open(QFile::ReadWrite);
        QJsonDocument config = QJsonDocument::fromJson(old_file_list.readAll());
        old_file_list.close();
        string password;
        QJsonArray tasks = config.array();
        QList<QString> files;
        for (auto task : tasks) {
            auto information = task.toObject();
            qDebug()<<(information["backupFilename"].toString()==ui->RestoreFileNameLine->text());
            if(information["backupFilename"].toString()==ui->RestoreFileNameLine->text()){
                for (auto file : information["files"].toArray()) {
                    files.append(file.toString());
                }
                password = information["password"].toString().toStdString();
            }
        }
        Compressor decompressor;
        QDir dir;
        dir.mkdir("./TEMP");
        QFileInfo TEMP("./TEMP");
        int errorCode = decompressor.decompress(ui->RestoreFileNameLine->text().toStdString(),
                                                TEMP.absoluteFilePath().toStdString() + "/",
                                                password);
        if (errorCode) {
            QStringList message = {"正常执行", QStringLiteral("源文件扩展名不是bak"), QStringLiteral("打开源文件失败"), QStringLiteral("打开目标文件失败"), QStringLiteral("文件过短，频率表不完整"), QStringLiteral("文件结尾不完整"), QStringLiteral("密码错误"), QStringLiteral("解码错误")};
            QMessageBox::information(this, "提示", message[errorCode],
                                     QMessageBox::Yes, QMessageBox::Yes);
            dir.rmdir("./TEMP");
            return;
        }

        QString tempFilename = "./TEMP/" + QFileInfo(ui->RestoreFileNameLine->text()).fileName();
        tempFilename = tempFilename.left(tempFilename.length() - 3) + "tar";

        errorCode = Pack::unpack_tar(QFileInfo(tempFilename).absoluteFilePath(), "./TEMP");
        if (errorCode) {
            QStringList message = {QStringLiteral("正常执行"),  QStringLiteral("打开源文件失败"), QStringLiteral("目标文件打开失败"),QStringLiteral("创建目录失败")};
            QMessageBox::information(this, QStringLiteral("提示"), message[errorCode],
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }

        QFile tarFile(tempFilename);
        tarFile.remove();


        auto difference = Check::check(files, "./TEMP");
        if (difference) {
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("备份无差异"),
                                     QMessageBox::Yes, QMessageBox::Yes);
        } else {
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("备份有差异"),
                                     QMessageBox::Yes, QMessageBox::Yes);
        }
        //dir.rmdir("./TEMP");
    });
    //定时
    timer.setInterval(5*1000);
    
    connect(&timer, &QTimer::timeout,[=]{
        qDebug() << "触发定时器";
   //执行定时任务
        if (taskManager.getTaskList().count())
        {
            qDebug() << taskManager.getTaskList().count();
            for(auto& t : taskManager.getTaskList())
            {
                //执行
                if (t.nextTime <= QDateTime::currentDateTime())
                {
                    QStringList files;
                    for (QString f : t.files) {
                        files.append(f);
                    }
                    int len = t.bakName.length();
                    QString name = t.bakName.left(len - 4);
                    Pack *packtool = new Pack;
                    int errorCode = packtool->pack_to_tar(files,
                                                          name + ".tar",
                                                          t.isCode);
                    if (errorCode) {
                        QStringList message = {"正常执行", QStringLiteral("目标文件打开失败"), QStringLiteral("打开源文件失败")};
                        QMessageBox::information(this, QStringLiteral("提示"), message[errorCode],
                                                 QMessageBox::Yes, QMessageBox::Yes);
                        return;
                    }
                    int i = t.localPath.lastIndexOf("/");
                    Compressor compressor;
                    errorCode = compressor.compress(name.toStdString() + ".tar",
                                                    t.localPath.left(i+1).toStdString() + "/",
                                                    t.isCode.toStdString());
                    if (errorCode) {
                        QStringList message = {"正常执行", QStringLiteral("源文件扩展名不是bak"), QStringLiteral("打开源文件失败"), QStringLiteral("打开目标文件失败")};
                        QMessageBox::information(this, QStringLiteral("提示"), message[errorCode],
                                                 QMessageBox::Yes, QMessageBox::Yes);
                        qDebug() << errorCode;
                        return;
                    }
                    QFile tarFile(name + ".tar");
                    //保存tarFile

                    //
                    tarFile.remove();
                    // 更新下一次备份时间
                    int index;
                    for (int i = 0; i < taskManager.getTaskNum();i++)
                    {
                        if(t.localPath == taskManager.getTaskList()[i].localPath)
                        {
                            index = i;
                            break;
                        }

                    }
                    taskManager.updateTime(index,t.freq=="每分钟" ?  QDateTime::currentDateTime().addSecs(60): QDateTime::currentDateTime().addDays(1));
                    taskManager.writeJson();
                }
            }
        }
        
    });
    timer.start();
}

MainWindow::~MainWindow()
{
    delete ui;

}
void MainWindow::showTasks(Tasks t) {
    QTableWidgetItem* bakName = new QTableWidgetItem;
    QTableWidgetItem* freq = new QTableWidgetItem;
    QTableWidgetItem* isCode = new QTableWidgetItem;
    QTableWidgetItem* localPath = new QTableWidgetItem;

    bakName->setText(t.bakName);
    freq->setText(t.freq);
    isCode->setText(t.isCode == "" ? "否" : "是");
    localPath->setText(t.localPath);

    int RowCount = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(RowCount);
    ui->tableWidget->setItem(RowCount, 0, bakName);
    ui->tableWidget->setItem(RowCount, 1, freq);
    ui->tableWidget->setItem(RowCount, 2, isCode);
    ui->tableWidget->setItem(RowCount, 3, localPath);

    ui->tableWidget->item(RowCount, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter); //设置居中
    ui->tableWidget->item(RowCount, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->item(RowCount, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    return;
}


