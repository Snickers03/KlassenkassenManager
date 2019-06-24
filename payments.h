#ifndef PAYMENTS_H
#define PAYMENTS_H

#include <QDialog>

namespace Ui {
class payments;
}

class payments : public QDialog
{
    Q_OBJECT

public:
    explicit payments(QWidget *parent = nullptr);
    ~payments();

private:
    Ui::payments *ui;
};

#endif // PAYMENTS_H
