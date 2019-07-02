#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sure.h"
#include "addstudent.h"
#include "student.h"
#include <QFile>
#include <QTextStream>
#include <QItemSelectionModel>
#include <QModelIndexList>
#include <QMessageBox>
#include <QtSql>
#include "payments.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->balanceSpinBox->setVisible(false);
    ui->balanceButtonBox->setVisible(false);
    ui->balanceTextEdit->setVisible(false);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    stud[0].setVorname("Nic");      //iniatalize example
    stud[0].setName("Schellenbaum");
    stud[0].setBalance(500);

    updateTable(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_quitButton_clicked()
{
    if (saved == true) {
        close();
    }
    else {
        Sure su(this);
        bool res;
        su.setWindowTitle("Schliessen");
        res = su.exec();

        if(res == true) {
            on_actionSpeichern_triggered();
            close();
        }
        else {
            close();
        }
    }
}

void MainWindow::on_addStudent_triggered()
{
    addStudent addStud(this);
    addStud.setWindowTitle("Schüler hinzufügen");
    //double balance;
    QString name, vorname;

    bool res;
    res = addStud.exec();
    if (res == false)
            return;

    stud[i].setVorname(addStud.getVorname());   //set object values
    stud[i].setName(addStud.getName());
    stud[i].setBalance(addStud.getBalance());

    ui->tableWidget->insertRow(ui->tableWidget->rowCount());    //add new cell
    int currentRow = ui->tableWidget->rowCount() - 1;

    ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(stud[i].getName()));
    ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(stud[i].getVorname()));
    ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(QString::number(stud[i].getBalance())));
    ui->tableWidget->item(currentRow, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);


    i++;
    studAmount++;
}

void MainWindow::updateTable(int row)
{
    ui->tableWidget->item(row, 0)->setText(stud[row].getName());
    ui->tableWidget->item(row, 1)->setText(stud[row].getVorname());
    ui->tableWidget->item(row, 2)->setText(QString::number(stud[row].getBalance()));
}

void MainWindow::on_actionZahlung_triggered()
{
    ui->balanceSpinBox->setVisible(true);
    ui->balanceButtonBox->setVisible(true);
    ui->balanceTextEdit->setVisible(true);
}

void MainWindow::on_balanceButtonBox_accepted()
{
    int row;
    double amount = ui->balanceSpinBox->value();
    QString reason = ui->balanceTextEdit->toPlainText();

    QItemSelectionModel *selections = ui->tableWidget->selectionModel();
    QModelIndexList selected = selections->selectedIndexes();

    if (selected.size() == 0) {
        message.critical(this, "Error", "Kein Schüler ausgewählt!");    //error if no student selected
    }

    for (int i = 0; i < selected.size(); i++)
    {
        row = selected[i].row();

        if (!stud[row].changed) {       //ensure balance not changed multiple times
            stud[row].changeBalance(amount);
            stud[row].addPayReason(reason);

            stud[row].changed = true;
            updateTable(row);
        }
    }

    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {     //reset stud.changed
        stud[i].changed = false;
    }

    ui->balanceSpinBox->setVisible(false);   //hide payment elements
    ui->balanceButtonBox->setVisible(false);
    ui->balanceTextEdit->setVisible(false);
}

void MainWindow::on_balanceButtonBox_rejected()
{
    ui->balanceSpinBox->setVisible(false);   //hide payment elements
    ui->balanceButtonBox->setVisible(false);
    ui->balanceTextEdit->setVisible(false);
}


void MainWindow::on_tableWidget_cellDoubleClicked(int row, int)
{
    Payments payments(this, row, stud[row].getName(), stud[row].getVorname());
    payments.setWindowTitle("Überblick Zahlungen");

    bool res = payments.exec();
    if (res == false)
            return;
}

void MainWindow::on_chooseAllButton_clicked()
{
    ui->tableWidget->setFocus();    //select all cells
    ui->tableWidget->selectAll();
}

void MainWindow::on_deleteStudent_triggered()
{
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
    studAmount--;
}


void MainWindow::on_actionSpeichern_triggered()
{
    QString databaseName("studentDatabase.db");                // https://wiki.qt.io/How_to_Store_and_Retrieve_Image_on_SQLite/de
    QFile::remove(databaseName);
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(databaseName);
    database.open();

    QSqlQuery query /*= QSqlQuery(database)*/;
    query.exec("CREATE TABLE IF NOT EXISTS students(id integer primary key, name varchar(50), vorname varchar(50), balance float)");

    for (int i = 0; i < studAmount; i++) {
        QString name(stud[i].getName());
        QString vorname(stud[i].getVorname());
        double balance = stud[i].getBalance();

        query.prepare("INSERT INTO students (id, name, vorname, balance) VALUES(:id, :name, :vorname, :balance)");      //in process https://katecpp.wordpress.com/2015/08/28/sqlite-with-qt/
        query.bindValue(":id", i);
        query.bindValue(":name", name);
        query.bindValue(":vorname", vorname);
        query.bindValue(":balance", balance);
        if(!query.exec()) {
            message.critical(this, "Error", "Query fehlgeschlagen");
        }
    }

    database.close();
    saved = true;
}

void MainWindow::on_action_open_triggered()
{
   // QString databaseName("C:/Users/nicsc/Documents/MaturS/studentDatabase.db");
    QString databaseName("studentDatabase.db");

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(databaseName);
    database.open();

    QSqlQuery query("SELECT name FROM students");
    int idName = query.record().indexOf("name");

    for (int i = 0; query.next() == true; i++) {
        QString name = query.value(idName).toString();
        stud[i].setName(name);
    }

    query.prepare("SELECT vorname FROM students");

    for (int i = 0; query.next() == true; i++) {
        stud[i].setVorname(query.value(idName).toString());
    }

    query.prepare("SELECT balance FROM students");

    for (int i = 0; query.next() == true; i++) {
        stud[i].setBalance(query.value(idName).toDouble());
    }

    for (int i = 0; i < idName; i++) {
        updateTable(i);
    }

}

void MainWindow::on_actionSort_triggered()
{
    ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget->sortByColumn(0,Qt::AscendingOrder);
    ui->tableWidget->setSortingEnabled(false);
}


void MainWindow::on_actionBeenden_triggered()
{
    on_quitButton_clicked();
}


