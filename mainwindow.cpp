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
#include <algorithm>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->balanceSpinBox->setVisible(false);
    ui->balanceButtonBox->setVisible(false);
    ui->balanceTextEdit->setVisible(false);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    databaseName = "studentDatabase.db";                                           // https://wiki.qt.io/How_to_Store_and_Retrieve_Image_on_SQLite/de
    database = QSqlDatabase::addDatabase("QSQLITE");            //create database
    database.setDatabaseName(databaseName);
    database.open();
    query = QSqlQuery(database);

    on_action_open_triggered();     //auto-open existing database

    ui->payTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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

    stud[studAmount].setVorname(addStud.getVorname());   //set object values
    stud[studAmount].setName(addStud.getName());
    stud[studAmount].setBalance(addStud.getBalance());

    addCell();
}

void MainWindow::updateTable(int row)
{
    ui->tableWidget->item(row, 0)->setText(stud[row].getName());
    ui->tableWidget->item(row, 1)->setText(stud[row].getVorname());
    ui->tableWidget->item(row, 2)->setText(QString::number(stud[row].getBalance()));
}

void MainWindow::addCell()
{
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());    //add new cell
    int currentRow = ui->tableWidget->rowCount() - 1;

    ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(stud[studAmount].getName()));
    ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(stud[studAmount].getVorname()));
    ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(QString::number(stud[studAmount].getBalance())));
    ui->tableWidget->item(currentRow, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

    studAmount++;
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

    query.exec("CREATE TABLE IF NOT EXISTS payments(id integer primary key, studId integer, date text, reason text, amount float)");

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

            QString name(stud[row].getName());
            QString vorname(stud[row].getVorname());

            query.prepare("SELECT id FROM students WHERE name = :name AND vorname = :vorname");     //get id from students
            query.bindValue(":name", name);
            query.bindValue(":vorname", vorname);
            query.exec();
            query.next();
            int id = query.value(0).toInt();

            query.prepare("INSERT INTO payments(id, studId, date, reason, amount) VALUES(NULL, :studId, strftime('%d/%m/%Y', 'now'), :reason, :amount)");
            query.bindValue(":studId", id);
            query.bindValue(":reason", reason);             //https://stackoverflow.com/questions/32962493/how-to-save-date-type-data-as-string-with-format-dd-mm-yyyy-in-sqlite?rq=1
            query.bindValue(":amount", amount);

            if (!query.exec()) {
                message.critical(this, "Error", "yeet query");
            }
        }
    }

    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {     //reset stud.changed
        stud[i].changed = false;
    }

    ui->balanceSpinBox->setVisible(false);   //hide payment elements
    ui->balanceButtonBox->setVisible(false);
    ui->balanceTextEdit->setVisible(false);

    //model->select();
    //ui->transactionTableView->setModel(model);
}

void MainWindow::on_balanceButtonBox_rejected()
{
    ui->balanceSpinBox->setVisible(false);   //hide payment elements
    ui->balanceButtonBox->setVisible(false);
    ui->balanceTextEdit->setVisible(false);
}


void MainWindow::on_tableWidget_cellDoubleClicked(int row, int)
{
    ui->tableWidget->selectRow(row);
    ui->transactionLabel->setText("Transaktionen von " + stud[row].getName()+ " " + stud[row].getVorname());

    ui->payTable->model()->removeRows(0, ui->payTable->rowCount());        //clear table
    int currentRow = 0;

    QString name(stud[row].getName());
    QString vorname(stud[row].getVorname());

    query.prepare("SELECT id FROM students WHERE name = :name AND vorname = :vorname");     //get id from students
    query.bindValue(":name", name);
    query.bindValue(":vorname", vorname);
    query.exec();
    query.next();
    int id = query.value(0).toInt();

    query.prepare("SELECT * FROM payments WHERE studId = :id");
    query.bindValue(":id", id);
    query.exec();                                                //https://www.techonthenet.com/sql/tables/create_table2.php

    QSqlRecord rec = query.record();
    int idName = rec.indexOf("reason");

    while (query.next()) {
        QString reason = query.value(idName).toString();

        ui->payTable->insertRow(ui->payTable->rowCount());
        currentRow = ui->payTable->rowCount() - 1;
        ui->payTable->setItem(currentRow, 1, new QTableWidgetItem(reason));
    }

    query.prepare("SELECT * FROM payments WHERE studId = :id");
    query.bindValue(":id", id);
    query.exec();                                                //https://www.techonthenet.com/sql/tables/create_table2.php

    rec = query.record();
    idName = rec.indexOf("amount");
    currentRow = 0;

    while (query.next()) {
        double amount = query.value(idName).toDouble();

        ui->payTable->setItem(currentRow, 2, new QTableWidgetItem(QString::number(amount)));
        currentRow++;
    }

    query.prepare("SELECT * FROM payments WHERE studId = :id");
    query.bindValue(":id", id);
    query.exec();                                                //https://www.techonthenet.com/sql/tables/create_table2.php

    rec = query.record();
    idName = rec.indexOf("date");
    currentRow = 0;

    while (query.next()) {
        QString date = query.value(idName).toString();

        ui->payTable->setItem(currentRow, 0, new QTableWidgetItem(date));
        currentRow++;
    }

    ui->tableWidget->clearSelection();
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
    query.exec("DROP TABLE students");
    query.exec("CREATE TABLE IF NOT EXISTS students(id integer primary key, name varchar(50), vorname varchar(50), balance float)");

    for (int i = 0; i < studAmount; i++) {
        QString name(stud[i].getName());
        QString vorname(stud[i].getVorname());
        double balance = stud[i].getBalance();

        query.prepare("INSERT INTO students (id, name, vorname, balance) VALUES(:id, :name, :vorname, :balance)");      //https://katecpp.wordpress.com/2015/08/28/sqlite-with-qt/
        query.bindValue(":id", i);
        query.bindValue(":name", name);
        query.bindValue(":vorname", vorname);
        query.bindValue(":balance", balance);

        if (!query.exec()) {
            message.critical(this, "Error", "yeet query");     //execute query & raise error if necessary
        }
    }

    saved = true;
}

void MainWindow::on_action_open_triggered()
{
    ui->tableWidget->model()->removeRows(0, ui->tableWidget->rowCount());       //clear student table
    studAmount = 0;

    query.exec("SELECT name FROM students ORDER BY name");            //https://doc.qt.io/qt-5/qsqlquery.html#next
    QSqlRecord rec = query.record();
    int idName = rec.indexOf("name");

    int i = 0;
    while (query.next()) {
        QString name = query.value(idName).toString();          //load names
        stud[i].setName(name);
        i++;
    }

    query.exec("SELECT vorname FROM students ORDER BY name");

    i = 0;
    while (query.next()) {
        QString vorname = query.value(idName).toString();       //load vornames
        stud[i].setVorname(vorname);
        i++;
    }

    query.exec("SELECT balance FROM students ORDER BY name");

    i = 0;
    while (query.next()) {
        double balance = query.value(idName).toDouble();        //load balances
        stud[i].setBalance(balance);
        i++;

        addCell();
    }

    for (int i = 0; i < studAmount; i++) {
        updateTable(i);
    }
}

void MainWindow::on_actionSort_triggered()
{
    /*ui->tableWidget->setSortingEnabled(true);                   //https://stackoverflow.com/questions/10079750/how-to-sort-values-in-columns-and-update-table
    ui->tableWidget->sortByColumn(0,Qt::AscendingOrder);
    ui->tableWidget->setSortingEnabled(false);*/
    /*QString order = "name";

    query.prepare("CREATE TABLE sub AS SELECT * FROM students ORDER BY :order");
    query.bindValue(":order", order);

    if (!query.exec()) {
        message.critical(this, "Error", "yeet query");     //execute query & raise error if necessary
    }

    query.exec("DROP TABLE students");*/
}


void MainWindow::on_actionBeenden_triggered()
{
    on_quitButton_clicked();
}


