#ifndef SURE_H
#define SURE_H

#include <QDialog>

namespace Ui {
class Sure;
}

class Sure : public QDialog
{
    Q_OBJECT

public:
    explicit Sure(QWidget *parent = nullptr);
    ~Sure();

private slots:


    void on_quitYes_clicked();

private:
    Ui::Sure *ui;

};

#endif // SURE_H
