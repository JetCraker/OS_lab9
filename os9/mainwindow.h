#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include "listOfFile.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void UpDateTable();
    ~MainWindow();

private slots:
    void on_Send_Message_clicked();

    void on_pushButton_clicked();

    void on_Save_clicked();

private:
    Ui::MainWindow *ui;
    HANDLE hPipe;
    ListOfFile list;
};
#endif // MAINWINDOW_H
