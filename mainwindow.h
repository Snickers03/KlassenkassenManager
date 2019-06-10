#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include "sure.h"
#include "addstudent.h"
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

private slots:

    void on_quitButton_clicked();

    void on_balanceButtonBox_accepted();

    void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_chooseAllButton_clicked();

    void on_addStudent_triggered();

    void on_deleteStudent_triggered();

    void on_actionZahlung_triggered();

private:
    Ui::MainWindow *ui;
    Sure *sure;

    int i = 1;  //change later!!!
    Student stud[50];
};

#endif // MAINWINDOW_H
