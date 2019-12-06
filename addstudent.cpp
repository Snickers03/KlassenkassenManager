#include "addstudent.h"
#include "ui_addstudent.h"
#include "mainwindow.h"
#include <QPushButton>


addStudent::addStudent(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addStudent)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Abbrechen");
}

addStudent::~addStudent()
{
    delete ui;
}

void addStudent::on_buttonBox_accepted()
{
    name = ui->lineEdit->text();
    vorname = ui->lineEdit_2->text();
    balance = ui->doubleSpinBox->value();

    if (name == "" || vorname == "") {
        message.critical(this, "Error", "Alle Felder müssen ausgefüllt werden!");
        return;
    }

    accept();
}

void addStudent::on_buttonBox_rejected()
{
    reject();
}

QString addStudent::getName()
{
    return name;
}

QString addStudent::getVorname()
{
    return vorname;
}

double addStudent::getBalance()
{   
    return balance;
}

