#ifndef STUDENT_H
#define STUDENT_H

#include <QObject>
#include <QVector>
#include "payments.h"

class Student
{
public:
    Student(QString name, QString vorname, double balance);
    Student();
    QString getName();
    void setName(QString name);

    QString getVorname();
    void setVorname(QString vorname);

    double getBalance();
    void setBalance(double balance);

    void changeBalance(double amount);
    bool changed = false;

    QVector<Payments> pay;

private:
    QString vorname;
    QString name;
    double balance;
};

#endif // STUDENT_H
