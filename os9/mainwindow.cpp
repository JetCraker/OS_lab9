#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <iostream>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    const char* pipeName = R"(\\.\pipe\FileMonitorPipe)";
    hPipe = CreateFileA(
        pipeName,              // Ім'я pipe
        GENERIC_READ | GENERIC_WRITE,  // Доступ для читання та запису
        0,                     // Без спільного доступу
        nullptr,               // Без атрибутів безпеки
        OPEN_EXISTING,         // Відкрити існуючий pipe
        0,                     // Атрибути/флаги
        nullptr                // Без шаблонів
        );

    if (hPipe == INVALID_HANDLE_VALUE) {
        QMessageBox::warning(nullptr, "Помилка", "Не вдалось підключитись до сервера");
    }
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

}

MainWindow::~MainWindow()
{
    CloseHandle(hPipe);
    delete ui;
}

void MainWindow::UpDateTable(){
    ui->tableWidget->setRowCount(0);
    for(int i=0;i<list.size();i++){
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        ui->tableWidget->setItem(i,0,new QTableWidgetItem(QString::fromStdString(list[i].getData())));
        ui->tableWidget->setItem(i,1,new QTableWidgetItem(QString::fromStdString(list[i].getName())));
        ui->tableWidget->setItem(i,2,new QTableWidgetItem(QString::number(list[i].getSize())));
    }
}

char* createJsonChar(const QString& type, const QString& path) {
    // Створення JSON-об'єкта
    QJsonObject jsonObj;
    jsonObj["extension"] = type;
    jsonObj["directory"] = path;

    // Створення JSON-документа
    QJsonDocument jsonDoc(jsonObj);

    // Перетворення JSON у рядок
    QByteArray jsonBytes = jsonDoc.toJson(QJsonDocument::Compact);

    // Створення C-style рядка (char*) через strdup
    return strdup(jsonBytes.constData());
}

void MainWindow::on_Send_Message_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(nullptr, "Виберіть директорію", QDir::homePath());
    if (directory.isEmpty()) {
        QMessageBox::warning(nullptr, "Помилка", "Шлях пустий");
    }
    std::string type ;
    switch(ui->comboBox->currentIndex()){
    case 0:
        type += ".pdf";
        break;
    case 1:
        type += ".pptx";
        break;
    case 2:
        type += ".png";
        break;
    case 3:
        type += ".txt";
        break;
    case 4:
        type += ".jpg";
        break;
    case 5:
        type += ".docx";
        break;
    }

    char* msg = createJsonChar(type.c_str(),directory.toStdString().c_str());
    //std::string line(msg);
    //QMessageBox::information(nullptr,"Повідомлення",QString::fromStdString(line));

    DWORD bytesWritten;
    if (!WriteFile(hPipe, msg, strlen(msg), &bytesWritten, nullptr)) {
        QMessageBox::warning(nullptr,"Помилка","Невдалось надіслати повідомлення серверу. Помилка"+QString::number(GetLastError()));
        CloseHandle(hPipe);
        return ;
    }

    if (WaitForSingleObject(hPipe, INFINITE) == WAIT_OBJECT_0) {
        char buffer[65536];
        DWORD bytesRead;
        if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr)) {
            QString line (buffer);
            list.getlist(buffer);
            UpDateTable();
            ui->label->setText("Розмір файлів у дерикторії "+ QString::number(list.getSizeOfList())+" Байти");
        } else {
            QMessageBox::warning(nullptr,"Помилка","Невдалось прочитати повідомлення сервера. Помилка"+QString::number(GetLastError()));
        }
    } else {
        QMessageBox::warning(nullptr,"Помилка","Помилка очікування. Помилка"+QString::number(GetLastError()));
        CloseHandle(hPipe);
        return ;
    }
}


void MainWindow::on_pushButton_clicked()
{
}


void MainWindow::on_Save_clicked()
{
    QFile file;
    QString filename;

    //Вивід у файл
    if(filename.isEmpty()){
        filename = QFileDialog::getSaveFileName(this, tr("Зберегти файл"), "", tr("Текстові файли (*.txt);;Усі файли (*.*)"));

        // Перевірка чи було вибрано файл
        if (filename.isEmpty()) {
            QMessageBox::warning(this, tr("Помилка"), tr("Файл не вибрано для збереження"));
            return;
        }

    }
    //Відкриття файлу
    file.setFileName(filename);
    // Перевірка чи файл відкритий
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Помилка"), tr("Не вдалося відкрити файл для запису"));
        return;
    }


    QTextStream out(&file);
    //Вивід будинків у файл
    for(size_t i=0; i<list.size(); i++){
        QString line = QString::fromStdString(list[i].getName()) +" "+ QString::fromStdString(list[i].getData()) + " " + QString::number(list[i].getSize())+"\n";
        out << line;
    }
    //Закриття файл
    file.close();
}

