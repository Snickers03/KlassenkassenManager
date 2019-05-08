#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sure.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*connect(ui->quitButton, SIGNAL(clicked()),
            this, SLOT(close()));*/
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_quitButton_clicked()
{
    close();
}
