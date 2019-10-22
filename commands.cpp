#include <QtWidgets>
#include "commands.h"
#include "addstudent.h"
#include "mainwindow.h"
#include <QUndoCommand>

AddCommand::AddCommand(QVector<Student> &stud, QString vorname, QString name, double balance, QTableWidget *tableWidget, QLineEdit *totalLineEdit,
                       QUndoCommand *parent)
    : QUndoCommand(parent), st(stud)
{
    qDebug() << "constructor";
    st = stud;
    table = tableWidget;
    totLine = totalLineEdit;

    this->vorname = vorname;
    this->name = name;
    this->balance = balance;


    QDate currentDate = QDate::currentDate();                   //http://qt.shoutwiki.com/wiki/Get_current_Date_and_Time_in_Qt
    date = currentDate.toString("dd.MM.yy");
}

void AddCommand::undo()
{
    qDebug() << "undo";
    st.erase(st.begin() + st.size() - 1);
    table->removeRow(table->rowCount() - 1);
}

void AddCommand::redo()
{
    table->blockSignals(true);
    st.append(Student(name, vorname, balance));
    st[st.size() - 1].pay.append(Payments(date, "Anfangsbestand", balance));

    table->insertRow(table->rowCount());    //add new cell
    int currentRow = table->rowCount() - 1;

    table->setItem(currentRow, 0, new QTableWidgetItem(name));
    table->setItem(currentRow, 1, new QTableWidgetItem(vorname));
    table->setItem(currentRow, 2, new QTableWidgetItem(QString::number(balance, 'f', 2)));
    table->item(currentRow, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

    updateTable(currentRow, table, st, totLine);
    table->blockSignals(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PayCommand::PayCommand(QVector<Student> &stud, QString date, QString reason, double amount, QTableWidget *tableWidget,
                       QModelIndexList selected, QLineEdit *totalLineEdit, QUndoCommand *parent)
    : QUndoCommand (parent), st(stud)           //https://www.geeksforgeeks.org/passing-vector-constructor-c/

{
    qDebug() << "constructor";
    //st = stud;
    table = tableWidget;
    sel = selected;
    totLine = totalLineEdit;

    this->date = date;
    this->reason = reason;
    this->amount = amount;

    /*ui->balanceSpinBox->setValue(0);
    ui->balanceTextEdit->clear();*/

    /*if (selectedStudent != -1 && ui->tableWidget->item(selectedStudent, 0)->isSelected()) {
        on_tableWidget_cellDoubleClicked(selectedStudent, 0);
    }*/
}

void PayCommand::undo()
{
    qDebug() << "undo";

    for (int i = 0; i < sel.size(); i++)
    {
        row = sel[i].row();
        st[row].changeBalance(-amount);

        st[row].pay.erase(st[row].pay.begin() + st[row].pay.size() - 1);
        updateTable(row, table, st, totLine);
    }
}

void PayCommand::redo()
{
    qDebug() << "redo";
    for (int i = 0; i < sel.size(); i++)
    {
        row = sel[i].row();
        st[row].changeBalance(amount);
        st[row].pay.append(Payments(date, reason, amount));

        updateTable(row, table, st, totLine);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////


DeleteCommand::DeleteCommand(QVector<Student> &stud, QTableWidget *tableWidget, QTableWidget *payTable, QModelIndexList studSel, QModelIndexList paySel, QLineEdit *totalLineEdit,
                             QLineEdit *balanceLineEdit, int selectedStudent, QUndoCommand *parent)
    : QUndoCommand (parent), st(stud)
{
    oldStud = stud;
    table = tableWidget;
    this->payTable = payTable;
    this->studSel = studSel;
    this->paySel = paySel;
    totLine = totalLineEdit;
    balLine = balanceLineEdit;

    selectedSt = selectedStudent;
}

void DeleteCommand::undo()
{
    st = oldStud;       //replaces stud vector with old copy

    table->model()->removeRows(0, table->rowCount());

    for (int i = 0; i < st.size(); i++) {
    table->insertRow(table->rowCount());    //add new cell
    int currentRow = table->rowCount() - 1;

    table->setItem(currentRow, 0, new QTableWidgetItem(st[currentRow].getName()));
    table->setItem(currentRow, 1, new QTableWidgetItem(st[currentRow].getVorname()));
    table->setItem(currentRow, 2, new QTableWidgetItem(QString::number(st[currentRow].getBalance(), 'f', 2)));
    table->item(currentRow, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
    }
    double total = 0;

    for (int i = 0; i < st.size(); i++) {
        total += st[i].getBalance();
    }
    totLine->setText(QString::number(total, 'f', 2));
}

void DeleteCommand::redo()
{
    //int row;
    //int oldStudAmount = st.size();
    //int oldPayCount = st[selectedSt].pay.size();

    for (int i = 0; i < paySel.size(); i++)
    {
        row = paySel[i].row();
        st[selectedSt].changeBalance(-st[selectedSt].pay[row].getAmount());        //change balance
        st[selectedSt].pay[row].changed = true;
    }

    for (int i = st[selectedSt].pay.size() - 1; i >= 0; i--)
    {
        if (st[selectedSt].pay[i].changed) {
            st[selectedSt].pay.erase(st[selectedSt].pay.begin() + i);
            payTable->removeRow(i);
        }
    }
/////////////
    balLine->setText(QString::number(st[selectedSt].getBalance(), 'f', 2));

///////////////////////

    for (int i = 0; i < studSel.size(); i++)
    {
        row = studSel[i].row();
        st[row].changed = true;             //mark selected student, no direct deletion due to studSel dependent on order of selection -> no guarantee of deletion from back to front
    }

    for (int i = st.size() - 1; i >= 0; i--)
    {
        if (st[i].changed) {
            st.erase(st.begin() + i);       //erase selected student
        }
    }

    for (int i = 0; i < st.size(); i++)
    {
        updateTable(i, table, st, totLine);
    }
    table->setRowCount(st.size());                   //remove empty rows
    table->clearSelection();
}


///////////////////////////////////////////////////////////////////////////////////////////


EditCommand::EditCommand(QVector<Student> &stud, QTableWidget *tableWidget, QTableWidget *payTable, int *selectedStudent, QLineEdit *totalLineEdit,
                         QLineEdit *balanceLineEdit, int row, int column, int mode, QUndoCommand *parent)
    : QUndoCommand (parent), st(stud)
{
    studTable = tableWidget;
    this->payTable = payTable;
    selectedSt = selectedStudent;
    oldSelectedSt = *selectedStudent;
    totLine = totalLineEdit;
    balLine = balanceLineEdit;

    this->row = row;
    this->column = column;
    this->mode = mode;

    if (mode == 1)              //student edited
    {
        if (column == 0) {
            oldValue = st[row].getName();
            newValue = studTable->item(row, 0)->text();
        }
        else if (column == 1) {
            oldValue = st[row].getVorname();
            newValue = studTable->item(row, 1)->text();
        }
        else if (column == 2) {
            oldValue = QString::number(st[row].getBalance(), 'f', 2);
            newValue = studTable->item(row, 2)->text();
        }
        else {
            qDebug() << "hello bug my old friend";
        }
    }
    else if (mode == 2)         //payment edited
    {
        if (column == 0) {
            oldValue = st[*selectedSt].pay[row].getDate();
            newValue = payTable->item(row, 0)->text();
        }
        else if (column == 1) {
            oldValue = st[*selectedSt].pay[row].getReason();
            newValue = payTable->item(row, 1)->text();
        }
        else if (column == 2) {
            oldValue = QString::number(st[*selectedSt].pay[row].getAmount(), 'f', 2);
            newValue = payTable->item(row, 2)->text();
        }
        else {
            qDebug() << "hello bug my old friend";
        }
    }
    else {
    qDebug() << "hello bug my old friend";
    }
}

void EditCommand::undo()
{
    qDebug() << "edit undo";
    if (mode == 1)
    {
        if (column == 0) {
            st[row].setName(oldValue);
        }
        else if (column == 1) {
            st[row].setVorname(oldValue);
        }
        else {
            qDebug() << "hello bug my old friend";
        }

        updateTable(row, studTable, st, totLine);
    }
    else if (mode == 2)
    {
        if (column == 0) {
            st[oldSelectedSt].pay[row].setDate(oldValue);
        }
        else if (column == 1) {
            st[oldSelectedSt].pay[row].setReason(oldValue);
        }
        else if (column == 2) {
            double dif = oldValue.toDouble() - newValue.toDouble();
            st[oldSelectedSt].changeBalance(dif);

            st[oldSelectedSt].pay[row].setAmount(oldValue.toDouble());
            updateTable(oldSelectedSt, studTable, st, totLine);         //update total
        }
        else {
            qDebug() << "hello bug my old friend";
        }
        *selectedSt = oldSelectedSt;        //show payments of affected student
    }
    else {
    qDebug() << "hello bug my old friend";
    }
}

void EditCommand::redo()
{
    if (mode == 1)              //student edited
    {
        if (column == 0) {
            st[row].setName(newValue);
        }
        else if (column == 1) {
            st[row].setVorname(newValue);
        }
        else {                                          //no "if (column == 2) because cant change balance column
            qDebug() << "hello bug my old friend";
        }
        updateTable(row, studTable, st, totLine);
    }
    else if (mode == 2)         //payment edited
    {
        if (column == 0) {
            st[oldSelectedSt].pay[row].setDate(newValue);
        }
        else if (column == 1) {
            st[oldSelectedSt].pay[row].setReason(newValue);
        }
        else if (column == 2) {
            double dif = newValue.toDouble() - oldValue.toDouble();
            st[oldSelectedSt].changeBalance(dif);

            st[oldSelectedSt].pay[row].setAmount(newValue.toDouble());
            updateTable(oldSelectedSt, studTable, st, totLine);

            payTable->blockSignals(true);
            payTable->item(row, 2)->setText(QString::number(newValue.toDouble(), 'f', 2));      //ensures correct formatting
            payTable->blockSignals(false);
        }
        else {
            qDebug() << "hello bug my old friend";
        }

        *selectedSt = oldSelectedSt;            //show affected student
    }
    else {
    qDebug() << "hello bug my old friend";
    }
}


///////////////////////////////////////////////////////////////////////////////////////////


void updateTable(int row, QTableWidget *table, QVector<Student> stud, QLineEdit *totalLineEdit)
{
    table->blockSignals(true);

    table->item(row, 0)->setText(stud[row].getName());
    table->item(row, 1)->setText(stud[row].getVorname());
    table->item(row, 2)->setText(QString::number(stud[row].getBalance(), 'f', 2));        //https://www.qtcentre.org/threads/40328-Formatting-for-two-decimal-places
    if (stud[row].getBalance() < 0) {
        table->item(row, 2)->setTextColor(Qt::red);
    }

    double total = 0;

    for (int i = 0; i < stud.size(); i++) {
        total += stud[i].getBalance();
    }
    totalLineEdit->setText(QString::number(total, 'f', 2));
    table->blockSignals(false);
}




