#include "payments.h"
#include "ui_payments.h"

Payments::Payments(QWidget *parent, int studNum, QString name, QString vorname) :
    QDialog(parent),
    ui(new Ui::Payments)
{
    ui->setupUi(this);
    this->studNum = studNum;
    this->name = name;
    this->vorname = vorname;

    ui->studName->setText("Transaktionen von " + name + " " + vorname);
}

Payments::~Payments()
{
    delete ui;
}

void Payments::setLabel(int student) {

}
