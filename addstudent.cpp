#include "addstudent.h"
#include "ui_addstudent.h"
#include "mainwindow.h"

addStudent::addStudent(QWidget *parent, QString name, QString vorname, int balance) :
    QDialog(parent),
    ui(new Ui::addStudent)
{
    ui->setupUi(this);
    this->name = name;
    this->vorname = vorname;
    this->balance = balance;
}

addStudent::~addStudent()
{
    delete ui;
}

void addStudent::on_pushButton_clicked()
{
    MainWindow mainw;
}

void addStudent::on_buttonBox_accepted()
{
    accept();
}

void addStudent::on_buttonBox_rejected()
{
    reject();
}

QString addStudent::getName()
{
    name = ui->lineEdit->text();
    return name;
}

QString addStudent::getVorname()
{
    vorname = ui->lineEdit_2->text();
    return vorname;
}

double addStudent::getBalance()
{
    balance = ui->doubleSpinBox->value();
    return balance;
}

