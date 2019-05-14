#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sure.h"
#include "addstudent.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_quitButton_clicked()
{
    Sure su(this);
    su.setWindowTitle("Schliessen");
    su.exec();
}

void MainWindow::on_addStudentButton_clicked()
{
    addStudent addStud(this);
    addStud.setWindowTitle("Schüler hinzufügen");
    addStud.exec();
}
