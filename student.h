#ifndef STUDENT_H
#define STUDENT_H

#include <QObject>

class Student
{
public:
    Student(QString name, QString vorname, int balance);
    Student();
    QString getName();
    void setName(QString name);

    QString getVorname();
    void setVorname(QString vorname);

    double getBalance();
    void setBalance(double balance);

    void changeBalance(double amount);

private:
    QString vorname;
    QString name;
    double balance;
};

#endif // STUDENT_H
