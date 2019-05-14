#include "addstudent.h"
#include "ui_addstudent.h"

addStudent::addStudent(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addStudent)
{
    ui->setupUi(this);
}

addStudent::~addStudent()
{
    delete ui;
}
