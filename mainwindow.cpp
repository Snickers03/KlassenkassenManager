#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sure.h"
#include "addstudent.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //QStringList titles;
    //ui->tableWidget->setColumnCount(3);
    //titles << "Name" << "Vorname" << "Guthaben";
   // ui->tableWidget->setHorizontalHeaderLabels(titles);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_quitButton_clicked()
{
    Sure su(this);
    bool res;
    su.setWindowTitle("Schliessen");
    res = su.exec();

    if(res == true) {
        close();
    }
}

void MainWindow::on_addStudentButton_clicked()
{
    addStudent addStud(this);
    addStud.setWindowTitle("Schüler hinzufügen");
    double balance;
    QString name, vorname;

    bool res;
    res = addStud.exec();
    if (res == false)
            return;

    balance = addStud.getBalance();
    name = addStud.getName();
    vorname = addStud.getVorname();

    ui->tableWidget->insertRow(ui->tableWidget->rowCount());    //add new cell
    int currentRow = ui->tableWidget->rowCount() - 1;

    ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(name));
    ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(vorname));
    ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(QString::number(balance)));
}
