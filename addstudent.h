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

private:
    Ui::addStudent *ui;
};

#endif // ADDSTUDENT_H
