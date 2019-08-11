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
#include <QDate>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QPainter>
#include "xlsxdocument.h"

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

    stud[studAmount].setVorname(addStud.getVorname());          //set object values
    stud[studAmount].setName(addStud.getName());
    stud[studAmount].setBalance(addStud.getBalance());

    QDate currentDate = QDate::currentDate();                   //http://qt.shoutwiki.com/wiki/Get_current_Date_and_Time_in_Qt
    QString date = currentDate.toString("dd.MM.yy");

    stud[studAmount].pay[0].setDate(date);                     //save initial payment
    stud[studAmount].pay[0].setReason("Anfangsbestand");
    stud[studAmount].pay[0].setAmount(addStud.getBalance());
    stud[studAmount].payCount++;

    addCell();
}

void MainWindow::updateTable(int row)
{
    ui->tableWidget->item(row, 0)->setText(stud[row].getName());
    ui->tableWidget->item(row, 1)->setText(stud[row].getVorname());
    ui->tableWidget->item(row, 2)->setText(QString::number(stud[row].getBalance(), 'f', 2));        //https://www.qtcentre.org/threads/40328-Formatting-for-two-decimal-places

    double total = 0;

    for (int i = 0; i < studAmount; i++) {
        total += stud[i].getBalance();
    }
    ui->totalLineEdit->setText(QString::number(total, 'f', 2));
}

void MainWindow::addCell()
{
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());    //add new cell
    int currentRow = ui->tableWidget->rowCount() - 1;

    ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(stud[studAmount].getName()));
    ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(stud[studAmount].getVorname()));
    ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(QString::number(stud[studAmount].getBalance(), 'f', 2)));
    ui->tableWidget->item(currentRow, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

    studAmount++;

    double total = 0;

    for (int i = 0; i < studAmount; i++) {
        total += stud[i].getBalance();
    }
    ui->totalLineEdit->setText(QString::number(total, 'f', 2));
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

    QDate currentDate = QDate::currentDate();           //http://qt.shoutwiki.com/wiki/Get_current_Date_and_Time_in_Qt
    QString date = currentDate.toString("dd.MM.yy");

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

            int payCount = stud[row].payCount;
            stud[row].pay[payCount].setReason(reason);
            stud[row].pay[payCount].setDate(date);
            stud[row].pay[payCount].setAmount(amount);

            stud[row].payCount++;
            stud[row].changed = true;
            updateTable(row);

            if (!query.exec()) {
                message.critical(this, "Error", "yeet query");
            }
        }
    }

    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {     //reset stud.changed
        stud[i].changed = false;
    }

    ui->balanceSpinBox->setValue(0);
    ui->balanceTextEdit->setText("");

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
    selectedStudent = row;
    ui->tableWidget->selectRow(row);
    ui->transactionLabel->setText("Transaktionen von " + stud[row].getName()+ " " + stud[row].getVorname());

    ui->payTable->model()->removeRows(0, ui->payTable->rowCount());        //clear table
    int currentRow = 0;
    double total = 0;           //current balance

    for(int i = 0; i < stud[row].payCount; i++) {

        QString date = stud[row].pay[i].getDate();
        QString reason = stud[row].pay[i].getReason();
        double amount = stud[row].pay[i].getAmount();

        ui->payTable->insertRow(ui->payTable->rowCount());
        currentRow = ui->payTable->rowCount() - 1;

        ui->payTable->setItem(currentRow, 0, new QTableWidgetItem(date));
        ui->payTable->setItem(currentRow, 1, new QTableWidgetItem(reason));
        ui->payTable->setItem(currentRow, 2, new QTableWidgetItem(QString::number(amount, 'f', 2)));
        ui->payTable->item(currentRow, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

        total += amount;
    }

    ui->balanceLineEdit->setText(QString::number(total, 'f', 2));       //https://www.qtcentre.org/threads/40328-Formatting-for-two-decimal-places
    ui->tableWidget->clearSelection();
}

void MainWindow::on_chooseAllButton_clicked()
{
    ui->tableWidget->setFocus();    //select all cells
    ui->tableWidget->selectAll();
}

void MainWindow::on_clearSelectionButton_clicked()
{
    ui->tableWidget->clearSelection();
}

void MainWindow::on_deleteStudent_triggered()
{
    int row = ui->tableWidget->currentRow();

    if (ui->tableWidget->hasFocus())
    {
        ui->tableWidget->removeRow(row);
        studAmount--;
        stud[row].payCount = 0;

        for (int i = row; i < studAmount; i++) {        //push back following objects
            stud[i] = stud[i + 1];
        }
    }
    else if (ui->payTable->hasFocus())
    {
        int payRow = ui->payTable->currentRow();
        ui->payTable->removeRow(payRow);
        stud[selectedStudent].payCount--;
        stud[selectedStudent].changeBalance(-stud[selectedStudent].pay[payRow].getAmount());        //change balance
        updateTable(selectedStudent);

        for (int i = payRow; i < stud[selectedStudent].payCount; i++) {
            stud[selectedStudent].pay[i] = stud[selectedStudent].pay[i + 1];
        }
    }
    else {
        message.critical(this, "Error", "Kein Schüler ausgewählt!");
    }
}


void MainWindow::on_actionSpeichern_triggered()
{
    query.exec("DROP TABLE students");
    query.exec("CREATE TABLE IF NOT EXISTS students(id integer primary key, name varchar(50), vorname varchar(50), balance float)");

    query.exec("DROP TABLE payments");
    query.exec("CREATE TABLE IF NOT EXISTS payments(id integer primary key, studId integer, date text, reason text, amount float)");

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

        for (int j = 0; j < stud[i].payCount; j++) {
            QString reason(stud[i].pay[j].getReason());
            QString date(stud[i].pay[j].getDate());
            double amount = stud[i].pay[j].getAmount();

            query.prepare("INSERT INTO payments (id, studId, date, reason, amount) VALUES(NULL, :studId, :date, :reason, :amount)");
            query.bindValue(":studId", i);
            query.bindValue(":date", date);
            query.bindValue(":reason", reason);
            query.bindValue(":amount", amount);

            if (!query.exec()) {
                message.critical(this, "Error", "yeet query");     //execute query & raise error if necessary
            }
        }
    }

    saved = true;
}

void MainWindow::on_action_open_triggered()
{
    ui->tableWidget->model()->removeRows(0, ui->tableWidget->rowCount());       //clear student table
    studAmount = 0;

    query.prepare("SELECT name FROM students ORDER BY CASE "
                  "WHEN :sort = 1 THEN name "
                  "WHEN :sort = 2 THEN vorname "
                  "WHEN :sort = 3 THEN balance "
                  "END");                                                                               //https://doc.qt.io/qt-5/qsqlquery.html#next
    query.bindValue(":sort", sort);                                                                     //https://www.sqlteam.com/articles/dynamic-order-by
    query.exec();

    QSqlRecord rec = query.record();
    int idName = rec.indexOf("name");

    for (int i = 0; query.next(); i++) {
        QString name = query.value(idName).toString();          //load names
        stud[i].setName(name);
    }

    query.prepare("SELECT vorname FROM students ORDER BY CASE "
                  "WHEN :sort = 1 THEN name "
                  "WHEN :sort = 2 THEN vorname "
                  "WHEN :sort = 3 THEN balance "
                  "END");                                                                               //https://doc.qt.io/qt-5/qsqlquery.html#next
    query.bindValue(":sort", sort);
    query.exec();

    for (int i = 0; query.next(); i++) {
        QString vorname = query.value(idName).toString();       //load vornames
        stud[i].setVorname(vorname);   
    }

    query.prepare("SELECT balance FROM students ORDER BY CASE "
                  "WHEN :sort = 1 THEN name "
                  "WHEN :sort = 2 THEN vorname "
                  "WHEN :sort = 3 THEN balance "
                  "END");                                                                               //https://doc.qt.io/qt-5/qsqlquery.html#next
    query.bindValue(":sort", sort);
    query.exec();

    for (int i = 0; query.next(); i++) {
        double balance = query.value(idName).toDouble();        //load balances
        stud[i].setBalance(balance);
        addCell();
    }

    query.prepare("SELECT id FROM students ORDER BY CASE "
                  "WHEN :sort = 1 THEN name "
                  "WHEN :sort = 2 THEN vorname "
                  "WHEN :sort = 3 THEN balance "
                  "END");
    query.bindValue(":sort", sort);
    query.exec();

    int order[50];

    for (int i = 0; query.next(); i++) {
        order[i] = query.value(idName).toInt();             //get new student order -> match payments
    }

    for (int i = 0; i < studAmount; i++) {
        updateTable(i);

        ////////////////////////////////////////////////////////
        // load payments
        stud[i].payCount = 0;               //reset pay count

        query.prepare("SELECT * FROM payments WHERE studId = :id");
        query.bindValue(":id", order[i]);
        query.exec();

        rec = query.record();
        idName = rec.indexOf("reason");

        for (int j = 0; query.next(); j++) {
            QString reason = query.value(idName).toString();
            stud[i].pay[j].setReason(reason);
        }

        query.prepare("SELECT * FROM payments WHERE studId = :id");
        query.bindValue(":id", order[i]);
        query.exec();

        rec = query.record();
        idName = rec.indexOf("date");

        for (int j = 0; query.next(); j++) {
            QString date = query.value(idName).toString();
            stud[i].pay[j].setDate(date);
        }

        query.prepare("SELECT * FROM payments WHERE studId = :id");
        query.bindValue(":id", order[i]);
        query.exec();

        rec = query.record();
        idName = rec.indexOf("amount");

        for (int j = 0; query.next(); j++) {
            double amount = query.value(idName).toDouble();
            stud[i].pay[j].setAmount(amount);

            stud[i].payCount++;
        }
    }
}

void MainWindow::on_actionBeenden_triggered()
{
    on_quitButton_clicked();
}

void MainWindow::on_actionName_triggered()
{
    sort = 1;
    on_actionSpeichern_triggered();
    on_action_open_triggered();
}

void MainWindow::on_actionVorname_triggered()
{
    sort = 2;
    on_actionSpeichern_triggered();
    on_action_open_triggered();
}

void MainWindow::on_actionGuthaben_triggered()
{
    sort = 3;
    on_actionSpeichern_triggered();
    on_action_open_triggered();
}

void MainWindow::on_actionExcel_triggered()       //import               //https://wiki.qt.io/Handling_Microsoft_Excel_file_format           http://qtxlsx.debao.me/
{
    studAmount = 0;

    QXlsx::Document xlsx("students.xlsx");

    for (int i = 0; xlsx.cellAt(i + 2, 2)->value().toString() != ""; i++)           //run until empty cell
    {
        stud[i].setName(xlsx.cellAt(i + 2, 2)->value().toString());
        stud[i].setVorname(xlsx.cellAt(i + 2, 3)->value().toString());
        studAmount++;
    }
}

void MainWindow::on_actionExcelExport_triggered()
{
    QXlsx::Document xlsx;       //overview
    QXlsx::Format format;

    format.setFontBold(true);

    xlsx.write("A1", "Name", format);
    xlsx.write("B1", "Vorname", format);
    xlsx.write("C1", "Guthaben", format);

    for (int i = 0; i < studAmount; i++)
    {
        xlsx.write(1, i + 2, stud[i].getName());
        xlsx.write(2, i + 2, stud[i].getVorname());
        xlsx.write(3, i + 2, stud[i].getBalance());
    }

    xlsx.saveAs("export.xlsx");
}

void MainWindow::on_actionPrintOverview_triggered()
{
    QFile::remove("students.pdf");

    QPrinter printer(QPrinter::HighResolution);                                     //https://doc.qt.io/qt-5/qtprintsupport-index.html
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName("students.pdf");

    QPainter painter(&printer);

    double xscale = printer.pageRect().width() / double(ui->tableWidget->width());              //https://stackoverflow.com/questions/45467942/how-can-i-print-a-qwidget-in-qt
    double yscale = printer.pageRect().height() / double(ui->tableWidget->height());
    double scale = qMin(xscale, yscale);
    painter.translate(printer.paperRect().center());
    painter.scale(scale, scale);
    painter.translate(-ui->tableWidget->width()/ 2, -ui->tableWidget->height()/ 2);
    ui->tableWidget->render(&painter);
}

void MainWindow::on_actionPrintAll_triggered()      //in process
{
    QFile::remove("all.pdf");

    QPrinter printer(QPrinter::HighResolution);             //https://doc.qt.io/qt-5/qtprintsupport-index.html
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName("all.pdf");

    QPainter painter;
    painter.begin(&printer);

    double xscale = printer.pageRect().width() / double(ui->payTable->width());              //https://stackoverflow.com/questions/45467942/how-can-i-print-a-qwidget-in-qt
    double yscale = printer.pageRect().height() / double(ui->payTable->height());
    double scale = qMin(xscale, yscale);
    painter.translate(printer.paperRect().center());
    painter.scale(scale, scale);
    painter.translate(-ui->payTable->width()/ 2, -ui->payTable->height()/ 2);

    for (int page = 0; page < studAmount; page++)
    {
        ///////////update pay table
        ui->payTable->model()->removeRows(0, ui->payTable->rowCount());        //clear table
        int currentRow = 0;
        double total = 0;           //current balance

        for(int i = 0; i < stud[page].payCount; i++) {

            QString date = stud[page].pay[i].getDate();
            QString reason = stud[page].pay[i].getReason();
            double amount = stud[page].pay[i].getAmount();

            ui->payTable->insertRow(ui->payTable->rowCount());
            currentRow = ui->payTable->rowCount() - 1;

            ui->payTable->setItem(currentRow, 0, new QTableWidgetItem(date));
            ui->payTable->setItem(currentRow, 1, new QTableWidgetItem(reason));
            ui->payTable->setItem(currentRow, 2, new QTableWidgetItem(QString::number(amount, 'f', 2)));
            ui->payTable->item(currentRow, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

            total += amount;
        }

        ////////////////////////
        ui->payTable->render(&painter);

        if (page != studAmount - 1) {
            printer.newPage();
        }
    }

    painter.end();
}

void MainWindow::on_actionPrintSelected_triggered()
{
    QFile::remove("selected.pdf");

    QPrinter printer(QPrinter::HighResolution);                                     //https://doc.qt.io/qt-5/qtprintsupport-index.html
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName("selected.pdf");

    QPainter painter(&printer);

    double xscale = printer.pageRect().width() / double(ui->payTable->width());              //https://stackoverflow.com/questions/45467942/how-can-i-print-a-qwidget-in-qt
    double yscale = printer.pageRect().height() / double(ui->payTable->height());
    double scale = qMin(xscale, yscale);
    painter.translate(printer.paperRect().center());
    painter.scale(scale, scale);
    painter.translate(-ui->payTable->width()/ 2, -ui->payTable->height()/ 2);
    ui->payTable->render(&painter);
}


