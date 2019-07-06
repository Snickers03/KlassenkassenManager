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


    QSqlTableModel *model = new QSqlTableModel(nullptr, database);      //https://stackoverflow.com/questions/13099830/qt-qtableview-sqlite-how-to-connect
    model->setTable("students");
    model->select();

    model->removeColumn(0);         //delete id row
    model->setHeaderData(0, Qt::Horizontal, tr("Name"));
    model->setHeaderData(1, Qt::Horizontal, tr("Vorname"));
    model->setHeaderData(2, Qt::Horizontal, tr("Balance"));

    ui->transactionTableView->setModel(model);
    ui->transactionTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    stud[0].setVorname("Max");      //iniatalize example
    stud[0].setName("Mustermann");
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



    //query.exec("CREATE TABLE IF NOT EXISTS payments(id integer primary key, date text, reason text, amount float)");     //https://stackoverflow.com/questions/7145933/create-table-dynamic-name-of-table
        //work in progress


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
    ui->tableWidget->selectRow(row);
    ui->transactionLabel->setText("Transaktionen von " + stud[row].getName()+ " " + stud[row].getVorname());
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

        query.exec();
    }

    saved = true;
}

void MainWindow::on_action_open_triggered()
{
    ui->tableWidget->model()->removeRows(0, ui->tableWidget->rowCount());       //clear student table
    studAmount = 0;

    query.exec("SELECT name FROM students");            //https://doc.qt.io/qt-5/qsqlquery.html#next
    QSqlRecord rec = query.record();
    int idName = rec.indexOf("name");

    int i = 0;
    while (query.next()) {
        QString name = query.value(idName).toString();          //load names
        stud[i].setName(name);
        i++;
    }

    query.exec("SELECT vorname FROM students");

    i = 0;
    while (query.next()) {
        QString vorname = query.value(idName).toString();       //load vornames
        stud[i].setVorname(vorname);
        i++;
    }

    query.exec("SELECT balance FROM students");

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
    ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget->sortByColumn(0,Qt::AscendingOrder);
    ui->tableWidget->setSortingEnabled(false);
}


void MainWindow::on_actionBeenden_triggered()
{
    on_quitButton_clicked();
}


