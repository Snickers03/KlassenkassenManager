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

    void pdfOverView(QTableWidget *tableWidget);
    void pdfAll(QTableWidget *payTable, Student stud[], int studAmount);
    void pdfSelected(QTableWidget *payTable, Student stud[], int selectedStudent);

    void excelOverView(Student stud[], int studAmount);
    void excelAll(Student stud[], int studAmount);
    void excelSelected(Student stud[], int selectedStudent);

    int mode;
    int choice;
private slots:
    void on_buttonBox_accepted();

private:
    Ui::Export *ui;
    QString filename;
};

#endif // EXPORT_H
