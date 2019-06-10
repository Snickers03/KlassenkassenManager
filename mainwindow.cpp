#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sure.h"
#include "addstudent.h"
#include "student.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->balanceSpinBox->setVisible(false);
    ui->balanceButtonBox->setVisible(false);
    ui->studentSpinBox->setVisible(false);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    stud[0].setVorname("Nic");  //iniatalize example
    stud[0].setName("Schellenbaum");
    stud[0].setBalance(500);

    updateTable(0);
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

void MainWindow::on_addStudent_triggered()
{
    addStudent addStud(this);
    addStud.setWindowTitle("Schüler hinzufügen");
    //double balance;
    QString name, vorname;

    bool res;
    res = addStud.exec();
    if (res == false)
            return;

    stud[i].setVorname(addStud.getVorname()); //set object values
    stud[i].setName(addStud.getName());
    stud[i].setBalance(addStud.getBalance());

    ui->tableWidget->insertRow(ui->tableWidget->rowCount());    //add new cell
    int currentRow = ui->tableWidget->rowCount() - 1;

    ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(stud[i].getName()));
    ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(stud[i].getVorname()));
    ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(QString::number(stud[i].getBalance())));

    i++;
}

void MainWindow::updateTable(int row)
{
    ui->tableWidget->item(row, 0)->setText(stud[row].getName());
    ui->tableWidget->item(row, 1)->setText(stud[row].getVorname());
    ui->tableWidget->item(row, 2)->setText(QString::number(stud[row].getBalance()));
}

void MainWindow::on_actionZahlung_triggered()
{
    ui->balanceSpinBox->setVisible(true);
    ui->balanceButtonBox->setVisible(true);
    ui->studentSpinBox->setVisible(true);
}

void MainWindow::on_balanceButtonBox_accepted()
{
    double amount = ui->balanceSpinBox->value();
    int studentNum = ui->studentSpinBox->value() - 1;

    stud[studentNum].changeBalance(amount);
    updateTable(studentNum);

    ui->balanceSpinBox->setVisible(false);   //hide payment elements
    ui->balanceButtonBox->setVisible(false);
    ui->studentSpinBox->setVisible(false);
}

void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    ui->tableWidget->selectRow(row);
}

void MainWindow::on_chooseAllButton_clicked()
{
    ui->tableWidget->setFocus();
    ui->tableWidget->selectAll();
}

void MainWindow::on_deleteStudent_triggered()
{
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
}

