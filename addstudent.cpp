#include "addstudent.h"
#include "ui_addstudent.h"
#include "mainwindow.h"
#include <QPushButton>

addStudent::addStudent(QWidget *parent, QString name, QString vorname, double balance) :
    QDialog(parent),
    ui(new Ui::addStudent)
{
    ui->setupUi(this);
    this->name = name;
    this->vorname = vorname;
    this->balance = balance;

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Abbrechen");
}

addStudent::~addStudent()
{
    delete ui;
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

