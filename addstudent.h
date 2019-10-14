#ifndef ADDSTUDENT_H
#define ADDSTUDENT_H

#include <QDialog>

namespace Ui {
class addStudent;
}

class addStudent : public QDialog
{
    Q_OBJECT

public:
    explicit addStudent(QWidget *parent = nullptr);
    ~addStudent();

    QString getName();
    void setName();

    QString getVorname();
    void setVorname();

    double getBalance();
    void setBalance();

private slots:

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::addStudent *ui;

    QString name;
    QString vorname;
    double balance;
};

#endif // ADDSTUDENT_H
