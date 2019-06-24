#include "payments.h"
#include "ui_payments.h"

payments::payments(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::payments)
{
    ui->setupUi(this);
}

payments::~payments()
{
    delete ui;
}
