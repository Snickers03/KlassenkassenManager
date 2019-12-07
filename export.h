#ifndef EXPORT_H
#define EXPORT_H

#include <QDialog>
#include <QWidget>
#include <QTableWidget>
#include "mainwindow.h"
#include "xlsxdocument.h"

namespace Ui {
class Export;
}

class Export : public QDialog
{
    Q_OBJECT

public:
    explicit Export(QWidget *parent = nullptr, int mode = 0);
    ~Export();

    void pdfOverView(QVector<Student> &stud, double total);
    void pdfAll(QVector<Student> &stud);
    void pdfSelected(QTableWidget *tableWidget, QVector<Student> &stud);

    void excelOverView(QVector<Student> &stud, double total);
    void excelAll(QVector<Student> &stud);
    void excelSelected(QVector<Student> &stud, QTableWidget* tableWidget);

    int mode;       //1: excel, 2: pdf
    int choice;
private slots:
    void on_buttonBox_accepted();

private:
    Ui::Export *ui;
    QString filename;
    QMessageBox message;

    QXlsx::Format fatFont;
    QXlsx::Format redFont;
    QXlsx::Format fatRedFont;

};

#endif // EXPORT_H
