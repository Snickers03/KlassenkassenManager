#ifndef COMMANDS_H
#define COMMANDS_H
#include <QtWidgets>
#include <QUndoCommand>
#include "student.h"
#include "mainwindow.h"

class AddCommand : public QUndoCommand
{
public:
    AddCommand(QVector<Student> &stud, QString vorname = "", QString name = "", double balance = 0,
               QTableWidget *tableWidget = nullptr, QLineEdit *totalLineEdit = nullptr, QUndoCommand *parent = nullptr);   /////parameter needed maybe
    //~AddCommand() override;

    void redo() override;
    void undo() override;

private:
    QVector<Student> &st;
    QTableWidget *table;
    QLineEdit *totLine;

    QString vorname;
    QString name;
    double balance;
    QString date;
};

//////////////////////////////////////////////////

class PayCommand : public QUndoCommand
{
public:
    PayCommand(QVector<Student> &stud, QString date = "", QString reason = "", double amount = 0, QTableWidget *tableWidget = nullptr, QModelIndexList selected = QList<QModelIndex>(),
               QLineEdit *totalLineEdit = nullptr, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;
private:
    QVector<Student> &st;
    QTableWidget *table;
    QModelIndexList sel;
    QLineEdit *totLine;

    int row;
    QString date;
    QString reason;
    double amount;
};

///////////////////////////////////////////////////

class DeleteCommand : public QUndoCommand
{
public:
    DeleteCommand(QVector<Student> &stud, QTableWidget *tableWidget = nullptr, QTableWidget *payTable = nullptr, QModelIndexList studSel = QList<QModelIndex>(), QModelIndexList paySel = QList<QModelIndex>(),
                  QLineEdit *totalLineEdit = nullptr, QLineEdit *balanceLineEdit = nullptr, int selectedStudent = 0, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QVector<Student> &st;
    QVector<Student> oldStud;
    QTableWidget *table;
    QTableWidget *payTable;
    QModelIndexList studSel;
    QModelIndexList paySel;
    QLineEdit *totLine;
    QLineEdit *balLine;

    int selectedSt;
    int row;
};

/////////////////////////////////////////////////

class EditCommand : public QUndoCommand
{
public:
    EditCommand(QVector<Student> &stud, QTableWidget *tableWidget = nullptr, QTableWidget *payTable = nullptr, int *selectedStudent = nullptr,
                QLineEdit *totalLineEdit = nullptr, QLineEdit *balanceLineEdit = nullptr, int row = 0, int column = 0, int mode = 0, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QVector<Student> &st;
    QTableWidget *studTable;
    QTableWidget *payTable;
    //QTableWidget *table;
    QLineEdit *totLine;
    QLineEdit *balLine;
    int *selectedSt;
    int oldSelectedSt;

    int row;
    int column;
    int mode;

    QString oldValue;   //stores old value of chanced cell
    QString newValue;   // ""    new ""
};

/////////////////////////////////////////////////

void updateTable(int row, QTableWidget *table, QVector<Student> stud, QLineEdit *totalLineEdit);
#endif // COMMANDS_H
