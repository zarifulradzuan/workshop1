#include "confirmationdialog.h"
#include "ui_confirmationdialog.h"

confirmationDialog::confirmationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::confirmationDialog)
{
    ui->setupUi(this);
}

confirmationDialog::~confirmationDialog()
{
    delete ui;
}

bool confirmationDialog::on_confirmationBox_accepted()
{
    delete ui;
    return 1;
}

bool confirmationDialog::on_confirmationBox_rejected()
{
    delete ui;
    return 0;
}
