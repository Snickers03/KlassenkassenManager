#include "export.h"
#include "ui_export.h"
#include "mainwindow.h"
#include <QFileDialog>
#include <QPrinter>
#include <QPainter>
#include "xlsxdocument.h"

Export::Export(QWidget *parent, int mode) :
    QDialog(parent),
    ui(new Ui::Export)
{
    ui->setupUi(this);
    this->mode = mode;
}

Export::~Export()
{
    delete ui;
}

void Export::on_buttonBox_accepted()
{
    if (mode == 1) {                               //export as Excel
        filename = QFileDialog::getSaveFileName(this, "Speichern als", "C://", ".xlsx");

        if (filename == "") {
            choice = 0;
            return;
        }

        if (ui->radioOverView->isChecked()) {
            choice = 1;
        }
        else if (ui->radioAll->isChecked()) {
            choice = 2;
        }
        else if (ui->radioSelected->isChecked()) {
            choice = 3;
        }
    }
    else if (mode == 2) {                                    //export as PFD
        filename = QFileDialog::getSaveFileName(this, "Speichern als", "C://", ".pdf");

        if (filename == "") {
            choice = 0;
            return;
        }

        if (ui->radioOverView->isChecked()) {
            choice = 4;
        }
        else if (ui->radioAll->isChecked()) {
            choice = 5;
        }
        else if (ui->radioSelected->isChecked()) {
            choice = 6;
        }
    }
}

void Export::pdfOverView(QTableWidget *tableWidget)
{
    //QFile::remove("students.pdf");

    QPrinter printer(QPrinter::HighResolution);                                     //https://doc.qt.io/qt-5/qtprintsupport-index.html
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename + ".pdf");

    QPainter painter(&printer);

    double xscale = printer.pageRect().width() / double(tableWidget->width());              //https://stackoverflow.com/questions/45467942/how-can-i-print-a-qwidget-in-qt
    double yscale = printer.pageRect().height() / double(tableWidget->height());
    double scale = qMin(xscale, yscale);
    painter.translate(printer.paperRect().center());
    painter.scale(scale, scale);
    painter.translate(-tableWidget->width()/ 2, -tableWidget->height()/ 2);
    tableWidget->render(&painter);
}

void Export::pdfAll(QTableWidget *payTable, Student stud[], int studAmount)
{
    //QFile::remove("all.pdf");

    QPrinter printer(QPrinter::HighResolution);             //https://doc.qt.io/qt-5/qtprintsupport-index.html
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename + ".pdf");

    QPainter painter;
    painter.begin(&printer);

    //painter.setFont(QFont("Arial", 20));
    double xscale = printer.pageRect().width() / double(payTable->width());              //https://stackoverflow.com/questions/45467942/how-can-i-print-a-qwidget-in-qt
    double yscale = printer.pageRect().height() / double(payTable->height());
    double scale = qMin(xscale, yscale);
    painter.translate(printer.paperRect().center());
    painter.scale(scale, scale);
    painter.translate(-payTable->width()/ 2, -payTable->height()/ 2 + 50);

    for (int page = 0; page < studAmount; page++)
    {
        ///////////update pay table
        payTable->model()->removeRows(0, payTable->rowCount());        //clear table
        int currentRow = 0;
        double total = 0;           //current balance

        for(int i = 0; i < stud[page].payCount; i++) {

            QString date = stud[page].pay[i].getDate();
            QString reason = stud[page].pay[i].getReason();
            double amount = stud[page].pay[i].getAmount();

            payTable->insertRow(payTable->rowCount());
            currentRow = payTable->rowCount() - 1;

            payTable->setItem(currentRow, 0, new QTableWidgetItem(date));
            payTable->setItem(currentRow, 1, new QTableWidgetItem(reason));
            payTable->setItem(currentRow, 2, new QTableWidgetItem(QString::number(amount, 'f', 2)));
            payTable->item(currentRow, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

            total += amount;
        }

        ////////////////////////
        //painter.drawText(rect(), Qt::AlignLeft, "Transaktionen von " + stud[page].getName() + " " + stud[page].getVorname());
        payTable->render(&painter);

        if (page != studAmount - 1) {
            printer.newPage();
        }
    }

    painter.end();
}

void Export::pdfSelected(QTableWidget *payTable, Student stud[], int selectedStudent)
{
    //QFile::remove("selected.pdf");        potential problem

    QPrinter printer(QPrinter::HighResolution);                                     //https://doc.qt.io/qt-5/qtprintsupport-index.html
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename + ".pdf");

    QPainter painter(&printer);                                                     //https://doc.qt.io/qt-5/qpainter.html

    painter.setFont(QFont("Arial", 20));
    painter.drawText(rect(), Qt::AlignLeft, "Transaktionen von " + stud[selectedStudent].getName() + " " + stud[selectedStudent].getVorname());

    double xscale = printer.pageRect().width() / double(payTable->width());              //https://stackoverflow.com/questions/45467942/how-can-i-print-a-qwidget-in-qt
    double yscale = printer.pageRect().height() / double(payTable->height());
    double scale = qMin(xscale, yscale);
    painter.translate(printer.paperRect().center());
    painter.scale(scale, scale);
    painter.translate(-payTable->width()/ 2, -payTable->height()/ 2 + 50);


    payTable->render(&painter);
}

void Export::excelOverView(Student stud[], int studAmount, double total)
{
    QXlsx::Document xlsx;       //overview
    QXlsx::Format format;
    format.setFontBold(true);

    xlsx.write("A1", "Name", format);
    xlsx.write("B1", "Vorname", format);
    xlsx.write("C1", "Guthaben", format);

    for (int i = 0; i < studAmount; i++)
    {
        xlsx.write(i + 2, 1, stud[i].getName());
        xlsx.write(i + 2, 2, stud[i].getVorname());
        xlsx.write(i + 2, 3, stud[i].getBalance());
    }

    xlsx.write(studAmount + 3, 2, "Total:", format);
    xlsx.write(studAmount + 3, 3, total, format);

    xlsx.saveAs(filename + ".xlsx");
}

void Export::excelAll(Student stud[], int studAmount)
{
    QXlsx::Document xlsx;
    QXlsx::Format format;
    format.setFontBold(true);

    for (int i = 0; i < studAmount; i++)
    {
        xlsx.addSheet(stud[i].getName());

        xlsx.write("A1", "Zahlungen von", format);
        xlsx.write("B1", stud[i].getName(), format);
        xlsx.write("C1", stud[i].getVorname(), format);
        xlsx.write("A3", "Datum", format);
        xlsx.write("B3", "Grund", format);
        xlsx.write("C3", "Menge", format);

        for (int j = 0; j < stud[i].payCount; j++)
        {
            xlsx.write(j + 4, 1, stud[i].pay[j].getDate());
            xlsx.write(j + 4, 2, stud[i].pay[j].getReason());
            xlsx.write(j + 4, 3, stud[i].pay[j].getAmount());
        }

        xlsx.write(stud[i].payCount + 5, 2, "Total:", format);
        xlsx.write(stud[i].payCount + 5, 3, stud[i].getBalance(), format);
    }

    xlsx.saveAs(filename + ".xlsx");
}

void Export::excelSelected(Student stud[], int sel)
{
    QXlsx::Document xlsx;
    QXlsx::Format format;
    format.setFontBold(true);
    xlsx.addSheet(stud[sel].getName());

    xlsx.write("A1", "Zahlungen von", format);
    xlsx.write("B1", stud[sel].getName(), format);
    xlsx.write("C1", stud[sel].getVorname(), format);

    xlsx.write("A3", "Datum", format);
    xlsx.write("B3", "Grund", format);
    xlsx.write("C3", "Menge", format);

    for (int i = 0; i < stud[sel].payCount; i++)
    {
        xlsx.write(i + 4, 1, stud[sel].pay[i].getDate());
        xlsx.write(i + 4, 2, stud[sel].pay[i].getReason());
        xlsx.write(i + 4, 3, stud[sel].pay[i].getAmount());
    }

    xlsx.write(stud[sel].payCount + 5, 2, "Total:", format);
    xlsx.write(stud[sel].payCount + 5, 3, stud[sel].getBalance(), format);

    xlsx.saveAs(filename + ".xlsx");
}



