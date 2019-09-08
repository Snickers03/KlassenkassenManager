#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sure.h"
#include "addstudent.h"
#include "student.h"
#include "export.h"
#include <QFile>
#include <QTextStream>
#include <QItemSelectionModel>
#include <QModelIndexList>
#include <QMessageBox>
#include <QtSql>
#include <QDate>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QPainter>
#include <QFileDialog>
#include "xlsxdocument.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->editLabel->setVisible(false);
    ui->editSaveButton->setVisible(false);
    ui->balanceButtonBox->button(QDialogButtonBox::Cancel)->setText("Abbrechen");

    databaseName = "studentDatabase.db";                                            // https://wiki.qt.io/How_to_Store_and_Retrieve_Image_on_SQLite/de
    database = QSqlDatabase::addDatabase("QSQLITE");                                //create database
    database.setDatabaseName(databaseName);
    database.open();
    query = QSqlQuery(database);

    on_action_open_triggered();                                                     //auto-open existing database

    ui->tableWidget->verticalHeader()->setVisible(true);
    ui->tableWidget->horizontalHeader()->setVisible(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    ui->payTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)             //https://stackoverflow.com/questions/17480984/qt-how-do-i-handle-the-event-of-the-user-pressing-the-x-close-button
{
    if (saved == true) {
        event->accept();
    }
    else {
        Sure su(this);
        bool res;
        su.setWindowTitle("Schliessen");
        res = su.exec();

        if(res == false) {
            event->ignore();
            return;
        }

        switch (su.choice) {
        case 0: return;
        case 1: on_actionSpeichern_triggered();
                event->accept();
            break;
        case 2: event->accept();
            break;
        case 3: event->ignore();
            return;
        }
    }
}

void MainWindow::on_quitButton_clicked()
{
    close();        //calls closeEvent
}

void MainWindow::on_actionBeenden_triggered()
{
    close();
}

void MainWindow::on_addStudent_triggered()
{
    addStudent addStud(this);
    addStud.setWindowTitle("Schüler hinzufügen");
    //double balance;
    QString name, vorname;

    bool res;
    res = addStud.exec();                                       //opens addStudent dialog
    if (res == false)
            return;

    stud[studAmount].setVorname(addStud.getVorname());          //set object values
    stud[studAmount].setName(addStud.getName());
    stud[studAmount].setBalance(addStud.getBalance());

    QDate currentDate = QDate::currentDate();                   //http://qt.shoutwiki.com/wiki/Get_current_Date_and_Time_in_Qt
    QString date = currentDate.toString("dd.MM.yy");

    stud[studAmount].pay[0].setDate(date);                      //save initial payment
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

    if (stud[row].getBalance() < 0) {
        ui->tableWidget->item(row, 2)->setTextColor(Qt::red);
    }

    total = 0;

    for (int i = 0; i < studAmount; i++) {
        total += stud[i].getBalance();
    }
    ui->totalLineEdit->setText(QString::number(total, 'f', 2));

    saved = false;
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
    total = 0;

    for (int i = 0; i < studAmount; i++) {
        total += stud[i].getBalance();
    }
    ui->totalLineEdit->setText(QString::number(total, 'f', 2));

    saved = false;
}

void MainWindow::on_balanceButtonBox_accepted()
{
    int row;
    double amount;
    QString reason = ui->balanceTextEdit->toPlainText();

    if (ui->plusToolButton->isChecked()) {
        amount = ui->balanceSpinBox->value();
    }
    else if (ui->minusToolButton->isEnabled()) {
        amount = -ui->balanceSpinBox->value();
    }
    else {
        message.critical(this, "Error", "how?");
        on_balanceButtonBox_rejected();
        return;
    }

    QDate currentDate = QDate::currentDate();                               //http://qt.shoutwiki.com/wiki/Get_current_Date_and_Time_in_Qt
    QString date = currentDate.toString("dd.MM.yy");

    QItemSelectionModel *selections = ui->tableWidget->selectionModel();
    QModelIndexList selected = selections->selectedIndexes();

    if (selected.size() == 0) {
        message.critical(this, "Error", "Kein Schüler ausgewählt!");        //error if no student selected
    }

    for (int i = 0; i < selected.size(); i++)
    {
        row = selected[i].row();

        if (!stud[row].changed) {                                           //ensure balance not changed multiple times
            stud[row].changeBalance(amount);

            int payCount = stud[row].payCount;
            stud[row].pay[payCount].setReason(reason);
            stud[row].pay[payCount].setDate(date);
            stud[row].pay[payCount].setAmount(amount);

            stud[row].payCount++;
            stud[row].changed = true;
            updateTable(row);
        }
    }

    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {                 //reset stud.changed
        stud[i].changed = false;
    }

    ui->balanceSpinBox->setValue(0);
    ui->balanceTextEdit->clear();

    if (selectedStudent != -1) {
        on_tableWidget_cellDoubleClicked(selectedStudent, 0);
    }
    saved = false;
}

void MainWindow::on_balanceButtonBox_rejected()
{
    ui->balanceSpinBox->setValue(0);
    ui->balanceTextEdit->clear();
}

void MainWindow::on_tableWidget_cellClicked(int row, int col)               //1 field selected == whole row selected
{
    bool rowSelected = !ui->tableWidget->item(row, col)->isSelected();

    if (rowSelected)
    {
        ui->tableWidget->item(row, 0)->setSelected(false);
        ui->tableWidget->item(row, 1)->setSelected(false);
        ui->tableWidget->item(row, 2)->setSelected(false);
    }
    else if (!rowSelected)
    {
        ui->tableWidget->selectRow(row);
    }
}

void MainWindow::on_tableWidget_cellDoubleClicked(int row, int)
{
    selectedStudent = row;
    ui->tableWidget->selectRow(row);
    ui->transactionLabel->setText("Transaktionen von " + stud[row].getName()+ " " + stud[row].getVorname());

    ui->payTable->model()->removeRows(0, ui->payTable->rowCount());         //clear table
    int currentRow = 0;
    studTotal = 0;                                                          //current balance

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

        studTotal += amount;
    }

    ui->payTable->resizeRowsToContents();      //makes cell bigger id doesnt fit      https://stackoverflow.com/questions/9544122/how-to-word-wrap-text-in-the-rows-and-columns-of-a-qtablewidget

    ui->balanceLineEdit->setText(QString::number(studTotal, 'f', 2));       //https://www.qtcentre.org/threads/40328-Formatting-for-two-decimal-places
    //ui->tableWidget->clearSelection();
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

        for (int i = row; i < studAmount; i++) {                    //push back following objects
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
    saved = false;
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
            message.critical(this, "Error", "yeet query");          //execute query & raise error if necessary
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
        QString name = query.value(idName).toString();                  //load names
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

void MainWindow::on_actionExcel_triggered()         //import               //https://wiki.qt.io/Handling_Microsoft_Excel_file_format           http://qtxlsx.debao.me/
{
    QString filename = QFileDialog::getOpenFileName(this, "Importiere Excel-Datei", "C://", "Excel Dateien (*.xlsx) ;; Alle Dateien (*.*)");     //https://www.youtube.com/watch?v=Fgt4WWdn3Ko
    //qDebug() << filename;                         //crashes sometimes, changing font fixes it for some f reason

    if (filename == "") {                                                       //cancel if no file selected
        return;
    }

    int nameHeader[2] = {0, 0};
    int vornameHeader[2] = {0, 0};
    int balanceHeader[2] = {0, 0};

    for (int i = 0; i < studAmount; i++)
    {
        stud[i].payCount = 0;
    }

    ui->tableWidget->model()->removeRows(0, ui->tableWidget->rowCount());       //clear student table
    studAmount = 0;

    QXlsx::Document xlsx(filename);

    for (int i = 1; i < 7; i++)
    {
        for (int j = 1; j < 7; j++)
        {
           if (xlsx.cellAt(i, j)->value().toString() == "Name" || xlsx.cellAt(i, j)->value().toString() == "Nachname") {    //get starting cell
               nameHeader[0] = i;
               nameHeader[1] = j;
           }
           if (xlsx.cellAt(i, j)->value().toString() == "Vorname") {
               vornameHeader[0] = i;
               vornameHeader[1] = j;
           }
           if (xlsx.cellAt(i, j) ->value().toString() == "Guthaben") {
               balanceHeader[0] = i;
               balanceHeader[1] = j;
           }
        }
    }

    if (nameHeader[0] == 0 || vornameHeader[0] == 0) {
        message.critical(this, "Error", "Namen und Vornamen Spalten nicht gefunden; "
                                        "Die Spalten müssen mit 'Name' und 'Vorname' beschriftet sein und sich in der oberen linken Ecke der Excel-Datei befinden. "
                                        "Optional: Die Spalte mit dem Kontostand muss mit 'Guthaben' beschriftet sein.");
        return;
    }

    for (int i = 0; xlsx.cellAt(i + nameHeader[0] + 1, nameHeader[1])->value().toString() != ""; i++)           //run until empty cell
    {
        stud[i].setName(xlsx.cellAt(i + nameHeader[0] + 1, nameHeader[1])->value().toString());
        stud[i].setVorname(xlsx.cellAt(i + vornameHeader[0] + 1, vornameHeader[1])->value().toString());

        if (balanceHeader[0] != 0)                                      //nested to avoid mistaking total value for balance of student
        {
            QDate currentDate = QDate::currentDate();                   //http://qt.shoutwiki.com/wiki/Get_current_Date_and_Time_in_Qt
            QString date = currentDate.toString("dd.MM.yy");

            stud[i].setBalance(xlsx.cellAt(i + balanceHeader[0] + 1, balanceHeader[1])->value().toDouble());
            stud[i].payCount = 1;
            stud[i].pay[0].setDate(date);
            stud[i].pay[0].setReason("Anfangsbestand");
            stud[i].pay[0].setAmount(stud[i].getBalance());
        }
        else {
            stud[i].setBalance(0);
        }

        addCell();
    }
}

void MainWindow::on_actionExcelExport_triggered()
{
    Export exp(this, 1);
    bool res;
    exp.setWindowTitle("Als Excel Datei exportieren");
    res = exp.exec();

    if (res == false) {
        return;
    }

    switch (exp.choice) {
    case 0: return;
    case 1: exp.excelOverView(stud, studAmount, total);
        break;
    case 2: exp.excelAll(stud, studAmount);
        break;
    case 3: exp.excelSelected(stud, ui->tableWidget);
    }
}

void MainWindow::on_actionPDF_triggered()
{
    Export exp(this, 2);
    bool res;
    exp.setWindowTitle("Als PDF exportieren");
    res = exp.exec();

    if (res == false) {
        return;
    }

    switch (exp.choice) {
    case 0: return;
    case 4: exp.pdfOverView(studAmount, stud, total);
        break;
    case 5: exp.pdfAll(stud, studAmount);
        break;
    case 6: exp.pdfSelected(ui->tableWidget, stud);
    }
}

void MainWindow::on_actionEditMode_triggered()
{
    ui->tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->payTable->setEditTriggers(QAbstractItemView::DoubleClicked);

    ui->tableWidget->blockSignals(true);                                                        //block signals -> no stud change on double click

    for (int i = 0; i < studAmount; i++) {                                                      //disable balance editing -> edit payments
        ui->tableWidget->item(i, 2)->setFlags(ui->tableWidget->item(i, 2)->flags() & ~Qt::ItemIsEditable);             //https://www.qtcentre.org/threads/26689-QTableWidget-one-column-editable
    }

    ui->editLabel->setVisible(true);
    ui->editSaveButton->setVisible(true);
}

void MainWindow::on_editSaveButton_clicked()
{
    if (selectedStudent != -1)
    {
        studTotal = 0;
        for (int i = 0; i < ui->payTable->rowCount(); i++)
        {
            stud[selectedStudent].pay[i].setDate(ui->payTable->item(i, 0)->text());
            stud[selectedStudent].pay[i].setReason(ui->payTable->item(i, 1)->text());

            double dif = ui->payTable->item(i, 2)->text().toDouble() - stud[selectedStudent].pay[i].getAmount();
            stud[selectedStudent].changeBalance(dif);

            stud[selectedStudent].pay[i].setAmount(ui->payTable->item(i, 2)->text().toDouble());
            studTotal += stud[selectedStudent].pay[i].getAmount();
        }

        for (int i = 0; i < ui->tableWidget->rowCount(); i++)
        {
            stud[i].setName(ui->tableWidget->item(i, 0)->text());            //watch out for sort bugs       -propably none
            stud[i].setVorname(ui->tableWidget->item(i, 1)->text());         //read table content and save
            updateTable(i);
        }

        ui->balanceLineEdit->setText(QString::number(studTotal, 'f', 2));

        Q_ASSERT(stud[selectedStudent].getBalance() == studTotal);          //bug detection
    }

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->payTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->editLabel->setVisible(false);
    ui->editSaveButton->setVisible(false);

    ui->tableWidget->blockSignals(false);           //re-enable signals
}
