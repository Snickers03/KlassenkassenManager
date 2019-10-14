#ifndef EXPORT_H
#define EXPORT_H

#include <QDialog>
#include <QWidget>
#include <QTableWidget>
#include "mainwindow.h"
namespace Ui {
class Export;
}

class Export : public QDialog
{
    Q_OBJECT

public:
    explicit Export(QWidget *parent = nullptr, int mode = 0);
    ~Export();

    void pdfOverView(int studAmount, QVector<Student> &stud, double total);
    void pdfAll(QVector<Student> &stud, int studAmount);
    void pdfSelected(QTableWidget *tableWidget, QVector<Student> &stud);

    void excelOverView(QVector<Student> &stud, int studAmount, double total);
    void excelAll(QVector<Student> &stud, int studAmount);
    void excelSelected(QVector<Student> &stud, QTableWidget* tableWidget);

    int mode;       //1: excel, 2: pdf
    int choice;
private slots:
    void on_buttonBox_accepted();

private:
    Ui::Export *ui;
    QString filename;
    QMessageBox message;
};

#endif // EXPORT_H
