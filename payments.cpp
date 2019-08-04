#include "payments.h"

Payments::Payments()
{

}

void Payments::setReason(QString reason)
{
    this->reason = reason;
}

void Payments::setDate(QString date)
{
    this->date = date;
}

void Payments::setAmount(double amount)
{
    this->amount = amount;
}


QString Payments::getReason()
{
    return reason;
}

QString Payments::getDate()
{
    return date;
}

double Payments::getAmount()
{
    return amount;
}
