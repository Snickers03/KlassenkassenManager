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
    accept();
}

void Sure::on_quitNo_clicked()
{
    reject();
}

