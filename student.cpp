#include "student.h"

/*Student::Student(QString name, QString vorname, int balance)
{
    this->name = name;
    this->vorname = vorname;
    this->balance = balance;
} */

Student::Student()
{

};

void Student::setName(QString name)
{
    this->name = name;
}

void Student::setVorname(QString vorname)
{
    this->vorname = vorname;
}

void Student::setBalance(double balance)
{
    this->balance = balance;
}


QString Student::getName()
{
    return name;
}

QString Student::getVorname()
{
    return vorname;
}

double Student::getBalance()
{
    return balance;
}

void Student::changeBalance(double amount)
{
    this->balance += amount;
}


