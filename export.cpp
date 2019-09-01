#include "export.h"
#include "ui_export.h"
#include "mainwindow.h"
#include <QFileDialog>
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

    if (mode == 1) {
        ui->warnLabel->setVisible(false);
    }
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

void Export::pdfOverView(QTableWidget *tableWidget)     //somewhat fixed
{
    QPrinter printer(QPrinter::HighResolution);                                     //https://doc.qt.io/qt-5/qtprintsupport-index.html
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename + ".pdf");

    QPainter painter(&printer);

    QPoint p;
    p.setX(5);
    p.setY(-3);

    double xscale = printer.pageRect().width() / double(tableWidget->width() + 20);              //https://stackoverflow.com/questions/45467942/how-can-i-print-a-qwidget-in-qt
    double yscale = printer.pageRect().height() / double(tableWidget->height());
    double scale = qMin(xscale, yscale);
    painter.translate(printer.paperRect().center());
    painter.scale(scale, scale);
    painter.translate(-tableWidget->width()/ 2, -tableWidget->height()/ 2 + 20);
    tableWidget->render(&painter);

    //painter.restore();
    painter.setFont(QFont("Arial", 1));
    painter.drawText(p, "Übersicht");
}

void Export::pdfAll(QTableWidget *payTable, Student stud[], int studAmount)
{
    QPrinter printer(QPrinter::HighResolution);             //https://doc.qt.io/qt-5/qtprintsupport-index.html
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename + ".pdf");

    QPainter painter;
    painter.begin(&printer);

    QPoint p;
    p.setX(5);
    p.setY(-5);

    painter.setFont(QFont("Arial", 1));

    double xscale = printer.pageRect().width() / double(payTable->width() + 20);              //https://stackoverflow.com/questions/45467942/how-can-i-print-a-qwidget-in-qt
    double yscale = printer.pageRect().height() / double(payTable->height() + 20);
    double scale = qMin(xscale, yscale);
    painter.translate(printer.paperRect().center());
    painter.scale(scale, scale);
    painter.translate(-payTable->width()/ 2, -payTable->height()/ 2 + 20);

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

        payTable->resizeRowsToContents();
        ////////////////////////

        payTable->render(&painter);
        painter.drawText(p, "Transaktionen von " + stud[page].getName() + " " + stud[page].getVorname());

        if (page != studAmount - 1) {
            printer.newPage();
        }
    }
    painter.end();
}

void Export::pdfSelected(QTableWidget *tableWidget, QTableWidget *payTable, Student stud[])
{
    QPrinter printer(QPrinter::HighResolution);                                     //https://doc.qt.io/qt-5/qtprintsupport-index.html
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename + ".pdf");

    QPainter painter(&printer);                                                     //https://doc.qt.io/qt-5/qpainter.html
    painter.setFont(QFont("Arial", 1));

    QPoint p;
    p.setX(5);
    p.setY(-5);

    int page = 1;

    QItemSelectionModel *selections = tableWidget->selectionModel();
    QModelIndexList selected = selections->selectedIndexes();

    if (selected.size() == 0) {
        message.critical(this, "Error", "Kein Schüler ausgewählt!");    //error if no student selected
    }

    double xscale = printer.pageRect().width() / double(payTable->width() + 20);              //https://stackoverflow.com/questions/45467942/how-can-i-print-a-qwidget-in-qt
    double yscale = printer.pageRect().height() / double(payTable->height() + 20);
    double scale = qMin(xscale, yscale);
    painter.translate(printer.paperRect().center());
    painter.scale(scale, scale);
    painter.translate(-payTable->width()/ 2, -payTable->height()/ 2 + 20);

    for (int i = 0; i < selected.size(); i++)
    {
        payTable->model()->removeRows(0, payTable->rowCount());        //clear table
        int currentRow = 0;
        double total = 0;
        int sel = selected[i].row();

        if (!stud[sel].changed)
        {
            for(int j = 0; j < stud[sel].payCount; j++) {
                QString date = stud[sel].pay[j].getDate();
                QString reason = stud[sel].pay[j].getReason();
                double amount = stud[sel].pay[j].getAmount();

                payTable->insertRow(payTable->rowCount());
                currentRow = payTable->rowCount() - 1;

                payTable->setItem(currentRow, 0, new QTableWidgetItem(date));
                payTable->setItem(currentRow, 1, new QTableWidgetItem(reason));
                payTable->setItem(currentRow, 2, new QTableWidgetItem(QString::number(amount, 'f', 2)));
                payTable->item(currentRow, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

                total += amount;
                page++;

            }
            payTable->resizeRowsToContents();
            payTable->render(&painter);
            painter.drawText(p, "Transaktionen von " + stud[sel].getName() + " " + stud[sel].getVorname());

            if (i != selected.size() - 3) {        /////////////////////////
                printer.newPage();
            }

            stud[sel].changed = true;
        }
    }
    for (int i = 0; i < tableWidget->rowCount(); i++) {     //reset stud.changed
        stud[i].changed = false;
    }

    painter.end();

    ////////////////////// NOTLÖSUNG /////////////////

   /* QString strStream;                                  //https://stackoverflow.com/questions/3147030/qtableview-printing/4079676#4079676  complete copy and paste
    QTextStream out(&strStream);

    const int rowCount = payTable->rowCount();
    const int columnCount = payTable->columnCount();

    out <<  "<html>\n"
        "<head>\n"
        "<meta Content=\"Text/html; charset=Windows-1251\">\n"
        <<  QString("<title>%1</title>\n").arg("Export")
        <<  "</head>\n"
        "<body bgcolor=#ffffff link=#5000A0>\n"
        "<table border=1 cellspacing=0 cellpadding=2>\n";

    // headers
    out << "<thead><tr bgcolor=#f0f0f0>";
    for (int column = 0; column < columnCount; column++)
        if (!payTable->isColumnHidden(column))
            out << QString("<th>%1</th>").arg(payTable->model()->headerData(column, Qt::Horizontal).toString());
    out << "</tr></thead>\n";

    // data table
    for (int row = 0; row < rowCount; row++) {
        out << "<tr>";
        for (int column = 0; column < columnCount; column++) {
            if (!payTable->isColumnHidden(column)) {
                QString data = payTable->model()->data(payTable->model()->index(row, column)).toString().simplified();
                out << QString("<td bkcolor=0>%1</td>").arg((!data.isEmpty()) ? data : QString("&nbsp;"));
            }
        }
        out << "</tr>\n";
    }
    out <<  "</table>\n"
        "</body>\n"
        "</html>\n";

    QTextDocument *document = new QTextDocument();
    document->setHtml(strStream);

    QPrinter printer;

    QPrintDialog *dialog = new QPrintDialog(&printer, nullptr);
    if (dialog->exec() == QDialog::Accepted) {
        document->print(&printer);
    }

    delete document;*/

    //////////////////////NOTLÖSUNG/////////////////
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
    QModelIndexList selected = selections->selectedIndexes();

    if (selected.size() == 0) {
        message.critical(this, "Error", "Kein Schüler ausgewählt!");    //error if no student selected
    }

    for (int i = 0; i < selected.size(); i++)
    {
        int sel = selected[i].row();

        if (!stud[sel].changed)
        {
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
    }

    for (int i = 0; i < tableWidget->rowCount(); i++) {     //reset stud.changed
        stud[i].changed = false;
    }

    xlsx.saveAs(filename + ".xlsx");
}



