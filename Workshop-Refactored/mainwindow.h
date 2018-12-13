#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "confirmationdialog.h"
#include <QtSql>
#include <QMainWindow>
#include <QtCharts>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    bool createConnection();
    explicit MainWindow(QWidget *parent = 0);
    void openConfirmationDialog();
    ~MainWindow();


private slots:
    void on_pushButton_clicked();

    void on_actionLogout_triggered();

    void on_actionExit_triggered();

    void on_userText_returnPressed();

    void on_passText_returnPressed();

    void on_registerStaffSave_clicked();

    void updateCount();

    void on_editStaffLoad_clicked();

    void on_editStaffList_doubleClicked();

    void on_editStaffButtonList_clicked();

    void on_cashierCallNext_clicked();

    void on_adminEditPatientButtonList_clicked();

    void on_adminEditPatientLoadList_clicked();

    void on_adminEditPatientList_doubleClicked();

    void on_adminEditPatientSave_clicked();

    void on_saveStaffEdit_clicked();

    void on_receptionistAddNewPatient_clicked();

    void on_receptionistSearchButton_clicked();

    void on_receptionistEnqueue_clicked();

    void on_receptionistSearchByButton_clicked();

    void on_receptionistEditPatient_clicked();

    void on_receptionistBack_clicked();

    void on_receptionistEditSave_clicked();

    void on_receptionistSearchByList_doubleClicked();

    void on_doctorCallNext_clicked();

    void on_doctorChangeStack_clicked();

    void on_commandLinkButton_clicked();

    void on_doctorAddRecord_clicked();

    void on_doctorSendForPaymentCheckBox_clicked();

    void on_doctorRemoveRowCheckBox_clicked();

    void on_doctorRemoveCurrentRecordRow_clicked();

    void clearDoctor();

    void on_cashierToPayList_itemChanged();

    void on_cashierFinaliseTransactionCheckBox_clicked();

    void clearCashier();

    void clearAdmin();

    void clearReceptionist();

    void on_cashierFinaliseTransaction_clicked();

    //void createReceipt();

    void on_financeGenerateChart_clicked();

    void on_financeChartGenerateType_currentIndexChanged(const QString &arg1);

    void on_financeChartGenerateChoice1_currentIndexChanged(const QString &arg1);

    void on_financeChartGenerateChoice2_currentIndexChanged(const int &arg1);

    QLineSeries* generateDataSetDaily(QStringList*,int);
    QLineSeries* generateDataSetMonthly();
    QLineSeries *generateDataSetYearly(QStringList*);

    void on_editChangeStatus_clicked();

    void on_cashierSearchByButton_clicked();

    void on_cashierEditPatient_clicked();

    void on_cashierInvoiceList_clicked();

    void on_cashierCommitChangeCheckBox_clicked();

    void on_cashierDeleteInvoiceCheckBox_clicked();

    void on_cashierChangeCashierStack_clicked();

    void on_cashierCommitChange_clicked();

    void on_cashierDeleteInvoice_clicked();

    void cashierUncheck();

    void on_adminSearchRecordButton_clicked();

    void on_adminRecordsList_doubleClicked();

    void on_adminEditRecord_clicked();

    void on_adminInvoiceEditPatient_clicked();

    void on_adminInvoiceSearchByButton_clicked();

    void on_adminInvoiceCommitChangeCheckBox_clicked();

    void on_adminInvoiceDeleteInvoiceCheckBox_clicked();

    void on_adminInvoiceCommitChange_clicked();

    void on_adminInvoiceDeleteInvoice_clicked();

    void on_adminInvoiceChangeStack_clicked();

    void adminUncheck();

    void on_adminInvoiceList_clicked();

    void adminClearRecord();

    void on_doctorSearchByButton_clicked();

    void on_doctorEditPatient_clicked();

    void doctorClearRecord();
    void on_doctorEditRecord_clicked();

    void on_doctorChangeDoctorStack_clicked();

    void on_doctorRecordsList_doubleClicked();

    void on_doctorSearchByList_doubleClicked();

    void on_doctorSearchByList_clicked();


    void on_receptionistQueueOption_currentIndexChanged(int index);

    void on_doctorCurrentRecordList_doubleClicked(const QModelIndex &index);

private:
    void changeStack(int);
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    Ui::MainWindow *ui;
    QSqlQuery q;
    QString adminEditPatientID;
    QString receptionistPatientID;
    QString doctorPatientID;
    QString cashierPatientID;
    int cashierLoaded=0;
    QString username;
    QString password;
    QString position;
    QString staffID;
};

#endif // MAINWINDOW_H
