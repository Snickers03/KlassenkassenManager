#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMessageBox>
#include "student.h"
#include <QtSql>

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

    void addCell();

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

    void on_balanceButtonBox_rejected();

    void on_actionBeenden_triggered();

    void on_action_open_triggered();

    void on_actionName_triggered();

    void on_actionVorname_triggered();

    void on_actionGuthaben_triggered();

    void on_clearSelectionButton_clicked();

    void on_actionExcel_triggered();

    void on_actionPrintAll_triggered();

    void on_actionPrintOverview_triggered();

    void on_actionPrintSelected_triggered();

private:
    Ui::MainWindow *ui;
    QMessageBox message;

    Student stud[50];
    int studAmount = 1;
    bool saved = false;
    int selectedStudent;    //payments of selected student shown
    int sort = 1;           //1 = order by name, 2= vorname, 3 = balance

    QString databaseName;
    QSqlDatabase database;
    QSqlQuery query;
    QSqlTableModel *model;
};

#endif // MAINWINDOW_H
