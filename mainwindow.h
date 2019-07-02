#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMessageBox>
#include "student.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateTable(int row);

    bool setData(const QModelIndex &index, const QVariant &value, int role);

private slots:

    void on_quitButton_clicked();

    void on_balanceButtonBox_accepted();

    void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_chooseAllButton_clicked();

    void on_addStudent_triggered();

    void on_deleteStudent_triggered();

    void on_actionZahlung_triggered();

    void on_actionSpeichern_triggered();

    void on_actionSort_triggered();

    void on_balanceButtonBox_rejected();

    void on_actionBeenden_triggered();

    void on_action_open_triggered();

private:
    Ui::MainWindow *ui;
    QMessageBox message;

    int i = 1;  //change later!!!
    Student stud[50];
    int studAmount = 1;
    bool saved = false;
};

#endif // MAINWINDOW_H
