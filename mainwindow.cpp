#include <QItemSelectionModel>
#include <QModelIndexList>
#include <QMessageBox>
#include <QtSql>
#include <QDate>
#include <QFileDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sure.h"
#include "addstudent.h"
#include "student.h"
#include "export.h"
#include "xlsxdocument.h"
#include "commands.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadSettings();

    ui->editLabel->setVisible(false);
    ui->editSaveButton->setVisible(false);
    ui->balanceButtonBox->button(QDialogButtonBox::Cancel)->setText("Abbrechen");

    //databaseName = "studentDatabase.db";                                            // https://wiki.qt.io/How_to_Store_and_Retrieve_Image_on_SQLite/de
    database = QSqlDatabase::addDatabase("QSQLITE");                                //create database
    database.setDatabaseName(databaseName);
    database.open();
    query = QSqlQuery(database);

    openDatabase();                                                    //auto-open existing database
    this->setWindowTitle("KlassenkassenManager      |       " + databaseName);          //display file path

    ui->tableWidget->verticalHeader()->setVisible(true);
    ui->tableWidget->horizontalHeader()->setVisible(true);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);           //select multiple rows at once
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);           //always select whole row

    ui->payTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->payTable->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->payTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    if (stud.size() != 0) {
        on_tableWidget_cellDoubleClicked(0, 0);         //show payments of first person by default
    }

    //
    undoStack = new QUndoStack(this);
    createActions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createActions()
{
    addAction = new QAction(tr("&Schüler hinzufügen"), this);
    deleteAction = new QAction(tr("&Löschen"), this);
    payAction = new QAction(tr("&Zahlung erfassen"), this);
    editAction = new QAction(tr("&Bearbeiten"), this);

    undoAction = undoStack->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcuts(QKeySequence::Undo);

    redoAction = undoStack->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcuts(QKeySequence::Redo);
}

void MainWindow::loadSettings()                                         //https://doc.qt.io/qt-5/qsettings.html#
{
    QSettings settings("Nic AG", "KlassenkassenManager");
    settings.beginGroup("MainWindow");
    databaseName = settings.value("dbFile", "studentDatabase.db").toString();       //open file last opened
    settings.endGroup();
}

void MainWindow::saveSettings()
{
    QSettings settings("Nic AG", "KlassenkassenManager");
    settings.beginGroup("MainWindow");
    settings.setValue("dbFile", databaseName);                                      //save file last opened
    settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event)             //https://stackoverflow.com/questions/17480984/qt-how-do-i-handle-the-event-of-the-user-pressing-the-x-close-button
{
    if (edit == true) {
        on_editSaveButton_clicked();   //saves edits when closing
    }

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
        case 0: return;                             //nothing selected
        case 1: on_actionSpeichern_triggered();     //save and close
                event->accept();
            break;
        case 2: event->accept();                    //close without saving
            break;
        case 3: event->ignore();                    //don't close
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

    bool res;
    res = addStud.exec();                                       //opens addStudent dialog
    if (res == false)
            return;

    if (edit == true) {
        on_editSaveButton_clicked();                            //ends edit mode
    }

    QUndoCommand *addCommand = new AddCommand(stud, addStud.getVorname(), addStud.getName(), addStud.getBalance(), ui->tableWidget, ui->totalLineEdit);
    undoStack->push(addCommand);

    saved = false;
}

void MainWindow::updateTable(int row)
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->item(row, 0)->setText(stud[row].getName());
    ui->tableWidget->item(row, 1)->setText(stud[row].getVorname());
    ui->tableWidget->item(row, 2)->setText(QString::number(stud[row].getBalance(), 'f', 2));        //https://www.qtcentre.org/threads/40328-Formatting-for-two-decimal-places
    if (stud[row].getBalance() < 0) {
        ui->tableWidget->item(row, 2)->setTextColor(Qt::red);
    }

    total = 0;

    for (int i = 0; i < stud.size(); i++) {
        total += stud[i].getBalance();
    }
    ui->totalLineEdit->setText(QString::number(total, 'f', 2));             //update total

    saved = false;
    ui->tableWidget->blockSignals(false);
}

void MainWindow::addCell()
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());        //add new cell
    int currentRow = ui->tableWidget->rowCount() - 1;

    ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(stud[currentRow].getName()));
    ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(stud[currentRow].getVorname()));
    ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(QString::number(stud[currentRow].getBalance(), 'f', 2)));
    ui->tableWidget->item(currentRow, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

    total = 0;

    for (int i = 0; i < stud.size(); i++) {
        total += stud[i].getBalance();
    }
    ui->totalLineEdit->setText(QString::number(total, 'f', 2));

    saved = false;
    ui->tableWidget->blockSignals(false);
}

void MainWindow::on_balanceButtonBox_accepted()             //payment accepted
{
    double amount;
    QString reason = ui->balanceTextEdit->toPlainText();

    if (ui->plusToolButton->isChecked()) {                  //positive
        amount = ui->balanceSpinBox->value();
    }
    else if (ui->minusToolButton->isEnabled()) {            //negative
        amount = -ui->balanceSpinBox->value();
    }
    else {
        message.critical(this, "Error", "how?");            //just in case
        on_balanceButtonBox_rejected();
        return;
    }

    QDate currentDate = QDate::currentDate();                               //http://qt.shoutwiki.com/wiki/Get_current_Date_and_Time_in_Qt
    QString date = currentDate.toString("dd.MM.yy");

    QItemSelectionModel *selections = ui->tableWidget->selectionModel();    //get selected rows
    QModelIndexList selected = selections->selectedRows();

    if (selected.size() == 0) {
        message.critical(this, "Error", "Kein Schüler ausgewählt!");        //error if no student selected
        return;
    }

    QUndoCommand *payCommand = new PayCommand(stud, date, reason, amount, ui->tableWidget, selected, ui->totalLineEdit);
    undoStack->push(payCommand);

    ui->balanceSpinBox->setValue(0);
    ui->balanceTextEdit->clear();

    if (selectedStudent != -1 && ui->tableWidget->item(selectedStudent, 0)->isSelected()) {
        on_tableWidget_cellDoubleClicked(selectedStudent, 0);               //update payTable
    }
    saved = false;
}

void MainWindow::on_balanceButtonBox_rejected()
{
    ui->balanceSpinBox->setValue(0);
    ui->balanceTextEdit->clear();
}

void MainWindow::on_tableWidget_cellDoubleClicked(int row, int col)
{
    ui->tableWidget->blockSignals(true);
    ui->payTable->blockSignals(true);

    if (edit == false || (edit == true && col == 2))                            //if edit enabled, only execute if balance row double clicked
    {
        selectedStudent = row;
        ui->transactionLabel->setText("Transaktionen von " + stud[row].getName()+ " " + stud[row].getVorname());

        ui->payTable->model()->removeRows(0, ui->payTable->rowCount());         //clear table
        int currentRow = 0;
        studTotal = 0;                                                          //current balance

        for(int i = 0; i < stud[row].pay.size(); i++) {

            QString date = stud[row].pay[i].getDate();
            QString reason = stud[row].pay[i].getReason();
            double amount = stud[row].pay[i].getAmount();

            ui->payTable->insertRow(ui->payTable->rowCount());                  //add row to payTable
            currentRow = ui->payTable->rowCount() - 1;

            ui->payTable->setItem(currentRow, 0, new QTableWidgetItem(date));
            ui->payTable->setItem(currentRow, 1, new QTableWidgetItem(reason));
            ui->payTable->setItem(currentRow, 2, new QTableWidgetItem(QString::number(amount, 'f', 2)));
            ui->payTable->item(currentRow, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

            studTotal += amount;
        }

        ui->payTable->resizeRowsToContents();  //makes cell bigger if doesnt fit      https://stackoverflow.com/questions/9544122/how-to-word-wrap-text-in-the-rows-and-columns-of-a-qtablewidget
        ui->balanceLineEdit->setText(QString::number(studTotal, 'f', 2));       //https://www.qtcentre.org/threads/40328-Formatting-for-two-decimal-places
        Q_ASSERT(stud[row].getBalance() == studTotal);                          //bug detection
    }
    ui->tableWidget->blockSignals(false);
    ui->payTable->blockSignals(false);
}

void MainWindow::on_chooseAllButton_clicked()
{
    ui->tableWidget->setFocus();            //select all cells
    ui->tableWidget->selectAll();
}

void MainWindow::on_clearSelectionButton_clicked()
{
    ui->tableWidget->clearSelection();
}

void MainWindow::on_deleteStudent_triggered()
{
    if (edit == true) {
        on_editSaveButton_clicked();        //saves edits when closing
    }

    QItemSelectionModel *studSelections = ui->tableWidget->selectionModel();
    QModelIndexList studSel = studSelections->selectedRows();

    QItemSelectionModel *paySelections = ui->payTable->selectionModel();
    QModelIndexList paySel = paySelections->selectedRows();

    if (studSel.size() == 0 && paySel.size() == 0) {
        message.critical(this, "Error", "Kein Schüler oder Zahlung ausgewählt");        //error if no student or payment selected
        return;
    }

    QUndoCommand *deleteCommand = new DeleteCommand(stud, ui->tableWidget, ui->payTable, studSel, paySel, ui->totalLineEdit, ui->balanceLineEdit, &selectedStudent);
    undoStack->push(deleteCommand);

    saved = false;
}


void MainWindow::on_actionSpeichern_triggered()
{
    if (edit == true) {
        on_editSaveButton_clicked();                //ends edit mode
    }

    query.exec("DROP TABLE students");              //delete old table
    query.exec("CREATE TABLE IF NOT EXISTS students(id integer primary key, name varchar(50), vorname varchar(50), balance float)");

    query.exec("DROP TABLE payments");
    query.exec("CREATE TABLE IF NOT EXISTS payments(id integer primary key, studId integer, date text, reason text, amount float)");

    for (int i = 0; i < stud.size(); i++) {
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

        for (int j = 0; j < stud[i].pay.size(); j++) {
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
    QString filename = QFileDialog::getOpenFileName(this, "Öffne Datenbank", "C://", "Datenbanken (*.db)");     //https://www.youtube.com/watch?v=Fgt4WWdn3Ko

    if (filename == "") {                                                       //cancel if no file selected
        return;
    }

    if (saved == false)             //asks if sure if another file opened
    {
        Sure su(this);
        bool res;
        su.setWindowTitle("Schliessen");
        res = su.exec();

        if(res == false) {
            return;
        }

        switch (su.choice) {
        case 0: return;                             //cancel
        case 1: on_actionSpeichern_triggered();     //save and continue
            break;
        case 2:                                     //dont save and continue
            break;
        case 3: return;                             //cancel
        }
    }
    database.close();
    databaseName = filename;
    database.setDatabaseName(databaseName);
    database.open();
    query = QSqlQuery(database);

    this->setWindowTitle("KlassenkassenManager      |       " + databaseName);
    selectedStudent = -1;                   //no student selected
    saveSettings();                         //save opened file
    openDatabase();

    undoStack->clear();         //clear undo stack
}

void MainWindow::openDatabase()
{
    if (edit == true) {
        on_editSaveButton_clicked();   //saves edits
    }

    ui->tableWidget->model()->removeRows(0, ui->tableWidget->rowCount());       //clear student table
    stud.clear();

    query.prepare("SELECT id, name, vorname, balance FROM students ORDER BY CASE "
                  "WHEN :sort = 1 THEN name "
                  "WHEN :sort = 2 THEN vorname "
                  "WHEN :sort = 3 THEN balance "
                  "END");                                                                               //https://doc.qt.io/qt-5/qsqlquery.html#next
    query.bindValue(":sort", sort);                                                                     //https://www.sqlteam.com/articles/dynamic-order-by
    query.exec();

    QSqlRecord rec = query.record();
    int idId = rec.indexOf("id");
    int idName = rec.indexOf("name");
    int idVorname = rec.indexOf("vorname");
    int idBalance = rec.indexOf("balance");

    int order[100];                                                     //turn into vector maybe

    for (int i = 0; query.next(); i++)
    {
        QString name = query.value(idName).toString();                  //load names
        QString vorname = query.value(idVorname).toString();
        double balance = query.value(idBalance).toDouble();
        order[i] = query.value(idId).toInt();                           //get new student order -> match payments

        stud.append(Student(name, vorname, balance));                   //insert new Student object into stud vector
        addCell();
    }

    for (int i = 0; i < stud.size(); i++) {
        updateTable(i);

        //////////////// load payments

        query.prepare("SELECT * FROM payments WHERE studId = :id");
        query.bindValue(":id", order[i]);
        query.exec();

        rec = query.record();
        int idDate = rec.indexOf("date");
        int idReason = rec.indexOf("reason");
        int idAmount = rec.indexOf("amount");

        for (int j = 0; query.next(); j++) {
            QString date = query.value(idDate).toString();
            QString reason = query.value(idReason).toString();
            double amount = query.value(idAmount).toDouble();

            stud[i].pay.append(Payments(date, reason, amount));                     //insert new payment object into vector
        }
        ////////////// load payments
    }
    if (selectedStudent != -1 && stud.size() != 0) {
        on_tableWidget_cellDoubleClicked(selectedStudent, 0);
    }
    else if (stud.size() != 0) {
        on_tableWidget_cellDoubleClicked(0, 0);
    }
    else {
        ui->payTable->model()->removeRows(0, ui->payTable->rowCount());             //clear table
    }
}

void MainWindow::on_actionExcel_triggered()         //import               //https://wiki.qt.io/Handling_Microsoft_Excel_file_format           http://qtxlsx.debao.me/
{
    QString filename = QFileDialog::getOpenFileName(this, "Importiere Excel-Datei", "C://", "Excel Dateien (*.xlsx) ;; Alle Dateien (*.*)");     //https://www.youtube.com/watch?v=Fgt4WWdn3Ko
    //qDebug() << filename;                         //crashes sometimes, changing font fixes it for some f reason

    if (filename == "") {                           //cancel if no file selected
        return;
    }

    if (edit == true) {
        on_editSaveButton_clicked();                //saves edits when closing
    }

/////////////
    undoStack->clear();                             //clear undo stack
    stud.clear();                                   //delete all old students & payments
    int nameHeader[2] = {0, 0};
    int vornameHeader[2] = {0, 0};
    int balanceHeader[2] = {0, 0};

    ui->tableWidget->model()->removeRows(0, ui->tableWidget->rowCount());       //clear student table

    QXlsx::Document xlsx(filename);

    for (int i = 1; i < 7; i++)
    {
        for (int j = 1; j < 7; j++)
        {
           if (xlsx.cellAt(i, j)->value().toString() == "Name" || xlsx.cellAt(i, j)->value().toString() == "Nachname") {    //search for header cell of name column
               nameHeader[0] = i;
               nameHeader[1] = j;
           }
           if (xlsx.cellAt(i, j)->value().toString() == "Vorname") {                                                        // "" vorname column
               vornameHeader[0] = i;
               vornameHeader[1] = j;
           }
           if (xlsx.cellAt(i, j) ->value().toString() == "Guthaben") {                                                      // "" balance column
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
        stud.append(Student());
        stud[i].setName(xlsx.cellAt(i + nameHeader[0] + 1, nameHeader[1])->value().toString());
        stud[i].setVorname(xlsx.cellAt(i + vornameHeader[0] + 1, vornameHeader[1])->value().toString());

        if (balanceHeader[0] != 0)                                      //nested to avoid mistaking total value for balance of student
        {
            QDate currentDate = QDate::currentDate();                   //http://qt.shoutwiki.com/wiki/Get_current_Date_and_Time_in_Qt
            QString date = currentDate.toString("dd.MM.yy");

            stud[i].setBalance(xlsx.cellAt(i + balanceHeader[0] + 1, balanceHeader[1])->value().toDouble());
            stud[i].pay.append(Payments());

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
    case 1: exp.excelOverView(stud, total);
        break;
    case 2: exp.excelAll(stud);
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
    case 4: exp.pdfOverView(stud, total);
        break;
    case 5: exp.pdfAll(stud);
        break;
    case 6: exp.pdfSelected(ui->tableWidget, stud);
    }
}

void MainWindow::on_actionEditMode_triggered()
{
    ui->tableWidget->blockSignals(true);
    edit = true;
    //setStyleSheet("centralWidget{background-color: #b3ffbb}");
    //setStyleSheet("QMainWindow {background-color: #b3ffbb}");
    //ui->centralWidget->setStyleSheet("background-color: #b3ffbb");

    ui->tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->payTable->setEditTriggers(QAbstractItemView::DoubleClicked);

    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableWidget->clearSelection();

    ui->payTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->payTable->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->payTable->clearSelection();

    for (int i = 0; i < stud.size(); i++) {                                                      //disable balance editing -> edit payments
        ui->tableWidget->item(i, 2)->setFlags(ui->tableWidget->item(i, 2)->flags() & ~Qt::ItemIsEditable);             //https://www.qtcentre.org/threads/26689-QTableWidget-one-column-editable
    }

    ui->editLabel->setVisible(true);
    ui->editSaveButton->setVisible(true);

    ui->payLabel->setVisible(false);
    ui->payLabel_2->setVisible(false);
    ui->balanceSpinBox->setVisible(false);
    ui->balanceTextEdit->setVisible(false);
    ui->balanceButtonBox->setVisible(false);
    ui->minusToolButton->setVisible(false);
    ui->plusToolButton->setVisible(false);
    ui->tableWidget->blockSignals(false);
}

void MainWindow::on_editSaveButton_clicked()
{
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->payTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);           //select multiple rows at once
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->clearSelection();

    ui->payTable->setSelectionMode(QAbstractItemView::MultiSelection);           //select multiple rows at once
    ui->payTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->payTable->clearSelection();

    ui->editLabel->setVisible(false);
    ui->editSaveButton->setVisible(false);

    ui->payLabel->setVisible(true);
    ui->payLabel_2->setVisible(true);
    ui->balanceSpinBox->setVisible(true);
    ui->balanceTextEdit->setVisible(true);
    ui->balanceButtonBox->setVisible(true);
    ui->minusToolButton->setVisible(true);
    ui->plusToolButton->setVisible(true);

    saved = false;
    edit = false;
}

void MainWindow::on_minusToolButton_clicked()
{
    ui->balanceSpinBox->setPrefix("-");
}

void MainWindow::on_plusToolButton_clicked()
{
    ui->balanceSpinBox->setPrefix("+");
}

void MainWindow::on_actionUndo_triggered()
{
    undoStack->undo();
    saved = false;

    if (selectedStudent >= stud.size())
    {
        selectedStudent = stud.size() - 1;
    }

    if (selectedStudent != -1) {                            //refresh payTable
        on_tableWidget_cellDoubleClicked(selectedStudent, 2);
    }
}

void MainWindow::on_actionRedo_triggered()
{
    undoStack->redo();
    saved = false;

    if (selectedStudent >= stud.size())
    {
        selectedStudent = stud.size() - 1;
    }

    if (selectedStudent != -1) {                //refresh payTable
        on_tableWidget_cellDoubleClicked(selectedStudent, 2);
    }
}

void MainWindow::on_tableWidget_cellChanged(int row, int column)
{
    qDebug() << "table cell changed";
    if (edit == true)
    {
        ui->tableWidget->blockSignals(true);            //avoid calling function again
        ui->payTable->blockSignals(true);
        int mode = 1;

        QUndoCommand *editCommand = new EditCommand(stud, ui->tableWidget, ui->payTable, &selectedStudent, ui->totalLineEdit, ui->balanceLineEdit, row, column, mode);
        undoStack->push(editCommand);

        ui->tableWidget->blockSignals(false);
        ui->payTable->blockSignals(false);
    }  
}

void MainWindow::on_payTable_cellChanged(int row, int column)
{
    qDebug() << "pay table cell changed";
    if (edit == true)
    {
        ui->tableWidget->blockSignals(true);            //avoid calling function again
        ui->payTable->blockSignals(true);
        int mode = 2;

        QUndoCommand *editCommand = new EditCommand(stud, ui->tableWidget, ui->payTable, &selectedStudent, ui->totalLineEdit, ui->balanceLineEdit, row, column, mode);
        undoStack->push(editCommand);

        on_tableWidget_cellDoubleClicked(selectedStudent, 2);

        ui->tableWidget->blockSignals(false);
        ui->payTable->blockSignals(false);
    }
}
