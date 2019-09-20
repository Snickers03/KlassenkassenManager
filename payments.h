#ifndef PAYMENTS_H
#define PAYMENTS_H

#include <QObject>

class Payments
{
public:
    Payments();

    QString getReason();
    void setReason(QString reason);

    QString getDate();
    void setDate(QString date);

    double getAmount();
    void setAmount(double amount);

    bool changed = false;

private:
    QString reason;
    QString date;
    double amount;
};

#endif // PAYMENTS_H
