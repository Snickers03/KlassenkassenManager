#ifndef PAYMENTS_H
#define PAYMENTS_H

#include <QDialog>
#include "student.h"

namespace Ui {
class Payments;
}

class Payments : public QDialog
{
    Q_OBJECT

public:
    explicit Payments(QWidget *parent = nullptr, int studNum = 0, QString name = "", QString vorname = "");
    ~Payments();

    void setLabel(int student = 0);

private:
    Ui::Payments *ui;

    int studNum;
    QString name;
    QString vorname;

};

#endif // PAYMENTS_H
