#include "sure.h"
#include "ui_sure.h"

Sure::Sure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Sure)
{
    ui->setupUi(this);
}

Sure::~Sure()
{
    delete ui;
}

void Sure::on_quitYes_clicked()
{
    choice = 1;
    accept();
}

void Sure::on_quitNo_clicked()
{
    choice = 2;
    accept();
}

void Sure::on_cancelPushButton_clicked()
{
    choice = 3;
    accept();
}
