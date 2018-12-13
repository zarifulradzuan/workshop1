#ifndef CONFIRMATIONDIALOG_H
#define CONFIRMATIONDIALOG_H

#include <QDialog>

namespace Ui {
class confirmationDialog;
}

class confirmationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit confirmationDialog(QWidget *parent = 0);
    ~confirmationDialog();

private slots:
    bool on_confirmationBox_accepted();

    bool on_confirmationBox_rejected();
private:
    Ui::confirmationDialog *ui;
};

#endif // CONFIRMATIONDIALOG_H
