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

    int choice = 0;         //1: save and quit, 2: don't save and quit, 3: cancel

private slots:

    void on_quitYes_clicked();

    void on_quitNo_clicked();

    void on_cancelPushButton_clicked();

private:
    Ui::Sure *ui;

};

#endif // SURE_H
