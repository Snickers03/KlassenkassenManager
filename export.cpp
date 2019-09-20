#include "export.h"
#include "ui_export.h"
#include "mainwindow.h"
#include <QFileDialog>
#include <QPushButton>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QPrintDialog>
#include <QMessageBox>
#include "xlsxdocument.h"

Export::Export(QWidget *parent, int mode) :
    QDialog(parent),
    ui(new Ui::Export)
{
    ui->setupUi(this);
    this->mode = mode;

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Abbrechen");
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

void Export::pdfOverView(int studAmount, Student stud[], double total)     //somewhat fixed
{
    QString strStream;                                  //https://stackoverflow.com/questions/3147030/qtableview-printing/4079676#4079676
    QTextStream out(&strStream);

    QPrinter printer;                                    //https://doc.qt.io/qt-5/qtprintsupport-index.html
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename + ".pdf");

    out <<  "<html>\n"
        "<head>\n"
        "<meta Content=\"Text/html; charset=Windows-1251\">\n"
        <<  QString("<title>%1</title>\n").arg("Export")
        <<  "</head>\n"
        "<body bgcolor=#ffffff link=#5000A0>\n"
        <<  QString("<h3>%1</h3>\n").arg("Übersicht");
    out <<  "<table border=1 cellspacing=0 cellpadding=2\n>";     //https://stackoverflow.com/questions/19993869/cannot-move-to-next-page-to-print-html-content-with-qprinter

    // headers
    out << "<thead><tr bgcolor=#f0f0f0>";
    out << QString("<th>%1</th>").arg("Name");
    out << QString("<th>%1</th>").arg("Vorname");
    out << QString("<th>%1</th>").arg("Guthaben");
    out << "</tr></thead>\n";

    for (int i = 0; i < studAmount; i++)
    {
        out << "<tr>";
        out << QString("<td bkcolor=0>%1</td>").arg(stud[i].getName());
        out << QString("<td bkcolor=0>%1</td>").arg(stud[i].getVorname());
        out << QString("<td bkcolor=0 style=\"text-align:right\">%1</td>").arg(QString::number(stud[i].getBalance(), 'f', 2));
        out << "</tr>\n";
    }

    out << "<tr> </tr>\n <tr>";
    out << "<td bkcolor=0></td>";
    out << "<td bkcolor=0>Total: </td>";
    out << QString("<td bkcolor=0 style=\"text-align:right\">%1</td>").arg(QString::number(total, 'f', 2));
    out << "</tr>";

    out <<  "</table>\n";
    out <<  "</body>\n"
        "</html>\n";

    QTextDocument *document = new QTextDocument();
    document->setHtml(strStream);

    document->print(&printer);
    delete document;
}

void Export::pdfAll(Student stud[], int studAmount)
{
    QString strStream;                                  //https://stackoverflow.com/questions/3147030/qtableview-printing/4079676#4079676
    QTextStream out(&strStream);

    QPrinter printer;             //https://doc.qt.io/qt-5/qtprintsupport-index.html
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename + ".pdf");

    for (int i = 0; i < studAmount; i++)
    {
        out <<  "<html>\n"
            "<head>\n"
            "<meta Content=\"Text/html; charset=Windows-1251\">\n"
            <<  QString("<title>%1</title>\n").arg("Export")
            <<  "</head>\n"
            "<body bgcolor=#ffffff link=#5000A0>\n"
            <<  QString("<h3>%1</h3>\n").arg("Transaktionen von " + stud[i].getName() + " " + stud[i].getVorname());
        out <<  "<table border=1 cellspacing=0 cellpadding=2\n>";     //https://stackoverflow.com/questions/19993869/cannot-move-to-next-page-to-print-html-content-with-qprinter

        // headers
        out << "<thead><tr bgcolor=#f0f0f0>";

        out << QString("<th>%1</th>").arg("Datum");
        out << QString("<th>%1</th>").arg("Grund");
        out << QString("<th>%1</th>").arg("Betrag");

        out << "</tr></thead>\n";

        for(int j = 0; j < stud[i].payCount; j++) {
            out << "<tr>";

            out << QString("<td bkcolor=0>%1</td>").arg(stud[i].pay[j].getDate());
            out << QString("<td bkcolor=0>%1</td>").arg(stud[i].pay[j].getReason());
            out << QString("<td bkcolor=0 style=\"text-align:right\">%1</td>").arg(QString::number(stud[i].pay[j].getAmount(), 'f', 2));

            out << "</tr>\n";
        }

        out << "<tr> </tr>\n <tr>";
        out << "<td bkcolor=0></td>";
        out << "<td bkcolor=0 width=30%>Total: </td>";
        out << QString("<td bkcolor=0 style=\"text-align:right\">%1</td>").arg(QString::number(stud[i].getBalance(), 'f', 2));
        out << "</tr>";

        out <<  "</table>\n";

        if (i != studAmount - 1) {
            out << "<div style=\"page-break-after:always\"></div>";
            qDebug() << "yees";
        }

        out <<  "</body>\n"
            "</html>\n";

        QTextDocument *document = new QTextDocument();
        document->setHtml(strStream);

        document->print(&printer);
        delete document;
    }
}

void Export::pdfSelected(QTableWidget *tableWidget, Student stud[])
{
    QString strStream;                                  //https://stackoverflow.com/questions/3147030/qtableview-printing/4079676#4079676
    QTextStream out(&strStream);

    QItemSelectionModel *selections = tableWidget->selectionModel();
    QModelIndexList selected = selections->selectedRows();

    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename + ".pdf");

    if (selected.size() == 0) {
        message.critical(this, "Error", "Kein Schüler ausgewählt!");    //error if no student selected
    }

    for (int i = 0; i < selected.size(); i++)
    {
        int sel = selected[i].row();

        out <<  "<html>\n"
                "<head>\n"
                "<meta Content=\"Text/html; charset=Windows-1251\">\n"
             <<  QString("<title>%1</title>\n").arg("Export")
              <<  "</head>\n"
                  "<body bgcolor=#ffffff link=#5000A0>\n"
               <<  QString("<h3>%1</h3>\n").arg("Transaktionen von " + stud[sel].getName() + " " + stud[sel].getVorname());
        out <<  "<table border=1 cellspacing=0 cellpadding=2\n>";     //https://stackoverflow.com/questions/19993869/cannot-move-to-next-page-to-print-html-content-with-qprinter

        // headers
        out << "<thead><tr bgcolor=#f0f0f0>";

        out << QString("<th>%1</th>").arg("Datum");
        out << QString("<th>%1</th>").arg("Grund");
        out << QString("<th>%1</th>").arg("Betrag");

        out << "</tr></thead>\n";

        for (int j = 0; j < stud[sel].payCount; j++)
        {
            out << "<tr>";
            out << QString("<td bkcolor=0>%1</td>").arg(stud[sel].pay[j].getDate());
            out << QString("<td bkcolor=0>%1</td>").arg(stud[sel].pay[j].getReason());
            out << QString("<td bkcolor=0 style=\"text-align:right\">%1</td>").arg(QString::number(stud[sel].pay[j].getAmount(), 'f', 2));
            out << "</tr>\n";
        }
        out << "<tr> </tr>\n <tr>";
        out << "<td bkcolor=0></td>";
        out << "<td bkcolor=0 width=30%>Total: </td>";
        out << QString("<td bkcolor=0 style=\"text-align:right\">%1</td>").arg(QString::number(stud[sel].getBalance(), 'f', 2));
        out << "</tr>";
        out <<  "</table>\n";

        if (i < selected.size() - 1) {        /////////////////////////
            out << "<div style=\"page-break-after:always\"></div>";
            qDebug() << "yees";
        }

        stud[sel].changed = true;

        out <<  "</body>\n"
            "</html>\n";

        QTextDocument *document = new QTextDocument();
        document->setHtml(strStream);

        document->print(&printer);
        delete document;
    }
}

void Export::excelOverView(Student stud[], int studAmount, double total)
{
    QXlsx::Document xlsx;       //overview
    QXlsx::Format format;
    format.setFontBold(true);

    xlsx.write("A1", "Name", format);       //headers
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

void Export::excelSelected(Student stud[], QTableWidget* tableWidget)
{
    QXlsx::Document xlsx;
    QXlsx::Format format;
    format.setFontBold(true);

    QItemSelectionModel *selections = tableWidget->selectionModel();
    QModelIndexList selected = selections->selectedRows();

    if (selected.size() == 0) {
        message.critical(this, "Error", "Kein Schüler ausgewählt!");    //error if no student selected
    }

    for (int i = 0; i < selected.size(); i++)
    {
        int sel = selected[i].row();
        xlsx.addSheet(stud[sel].getName());

        xlsx.write("A1", "Zahlungen von", format);
        xlsx.write("B1", stud[sel].getName(), format);
        xlsx.write("C1", stud[sel].getVorname(), format);

        xlsx.write("A3", "Datum", format);
        xlsx.write("B3", "Grund", format);
        xlsx.write("C3", "Menge", format);

        for (int j = 0; j < stud[sel].payCount; j++)
        {
            xlsx.write(j + 4, 1, stud[sel].pay[j].getDate());
            xlsx.write(j + 4, 2, stud[sel].pay[j].getReason());
            xlsx.write(j + 4, 3, stud[sel].pay[j].getAmount());
        }

        xlsx.write(stud[sel].payCount + 5, 2, "Total:", format);
        xlsx.write(stud[sel].payCount + 5, 3, stud[sel].getBalance(), format);

        stud[sel].changed = true;
    }
    xlsx.saveAs(filename + ".xlsx");
}
