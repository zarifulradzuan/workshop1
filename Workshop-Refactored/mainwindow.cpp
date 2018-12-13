#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql>
#include <QMessageBox>
#include <string>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QTextBrowser>
#include <QtCharts>
#include <QTextDocument>
#include <QWebEngineView>

using namespace std;

bool MainWindow:: createConnection(){
    db.setHostName("aestia");
    db.setDatabaseName("ics");
    db.setUserName("zarifulradzuan");
    db.setPassword("30497");
    if(!db.open())
        return false;
    else
        return true;
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow){
    ui->setupUi(this);
    if(createConnection())
        statusBar()->showMessage(tr("Ready"));
    else
        statusBar()->showMessage(tr("Database not connected"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateCount(){
    if(position=="DOCTOR"){
        q.exec("select count(idqueue) from queuetoserve where iddoctor='"+staffID+"' OR iddoctor=0");
        q.next();
        ui->doctorWaitingAmount->display(q.value(0).toInt());
        if(q.value(0).toInt()==0 || doctorPatientID!=NULL)
            ui->doctorCallNext->setDisabled(1);
        else
            ui->doctorCallNext->setDisabled(0);
    }
    if(position=="CASHIER"){
        q.exec("select count(idqueue) from queuetopay");
        q.next();
        ui->cashierWaitingAmount->display(q.value(0).toInt());
        if(q.value(0).toInt()==0 || cashierPatientID!=NULL)
            ui->cashierCallNext->setDisabled(1);
        else
            ui->cashierCallNext->setDisabled(0);
    }
    if(position=="RECEPTIONIST"){
        q.exec("select count(idqueue) from queuetoserve");
        q.next();
        ui->receptionistWaitingAmountServe->display(q.value(0).toInt());
        q.exec("select count(idqueue) from queuetopay");
        q.next();
        ui->receptionistWaitingAmountPay->display(q.value(0).toInt());
    }
}

void MainWindow::changeStack(int pageIndex){
    ui->actionLogout->setEnabled(1);
    ui->interfaceStack->setCurrentIndex(pageIndex);
    ui->userText->clear();
    ui->passText->clear();
    QString message="Signed in as ";
    message+=username;
    statusBar()->showMessage(message,5000);
}

void MainWindow::on_pushButton_clicked(){
    username=ui->userText->text().toUpper();
    password=ui->passText->text();
    q.exec("select position,staffid,status from staff where username=upper('"+username+"') and password='"+password+"'");
    if(q.next()){
        if(q.value(2).toString()=="ACTIVE"){
            staffID=q.value(1).toString();
            position=q.value(0).toString();
            if(position=="FINANCE"){
                changeStack(5);
            }
            else{
                if(position=="CASHIER"){
                    changeStack(1);
                }
                else if(position=="ADMIN"){
                    changeStack(2);
                }
                else if(position=="DOCTOR"){
                    changeStack(3);
                }
                else if(position=="RECEPTIONIST"){
                    changeStack(4);
                }
                QTimer *timer = new QTimer(this);
                connect(timer, SIGNAL(timeout()), this, SLOT(updateCount()));
                timer->start(1000);
            }
        }
        else {
            QMessageBox::warning(this,"Login", "Account was deactivated, contact administrator");
        }
    }
    else
        QMessageBox::warning(this,"Login", "Username or password is not correct");
}

void MainWindow::on_actionLogout_triggered(){
    ui->actionLogout->setEnabled(0);
    ui->interfaceStack->setCurrentIndex(0);
    staffID.clear();
    clearDoctor();
    clearReceptionist();
    clearCashier();
    clearAdmin();
    position.clear();
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_userText_returnPressed()
{
    on_pushButton_clicked();
}

void MainWindow::on_passText_returnPressed()
{
    on_pushButton_clicked();
}

void MainWindow::on_registerStaffSave_clicked()
{
    QString cNumber=ui->registerStaffCNumber->text(),
            fName=ui->registerStaffFirstName->text().toUpper(),
            lName=ui->registerStaffLastName->text().toUpper(),
            email=ui->registerStaffEmail->text().toUpper(),
            username=ui->registerUsername->text(),
            password=ui->registerPassword->text(),
            position=ui->registerPosition->currentText();
    if(fName=="" || lName=="" || position=="" || username=="" || password=="" || cNumber==""|| email=="")
        statusBar()->showMessage("Please fill all the required fields",3000);
    else{
        q.exec("SELECT username from staff where username='"+username+"'");
        if(q.next())
            statusBar()->showMessage("Username already taken",3000);
        else{
            q.exec("INSERT INTO staff (`fName`, `lName`, `username`, `password`, `position`, `email`, `cNumber`, `dateHired`) VALUES ('"+fName+"', '"+lName+"', '"+username+"', '"+password+"', '"+position+"', '"+email+"', '"+cNumber+"', curdate())");
            ui->registerStaffFirstName->clear();
            ui->registerStaffLastName->clear();
            ui->registerStaffCNumber->clear();
            ui->registerStaffEmail->clear();
            ui->registerUsername->clear();
            ui->registerPassword->clear();
            ui->registerPosition->setCurrentIndex(-1);
        }
    }
}



void MainWindow::on_editStaffLoad_clicked()
{
    ui->editEmail->clear();
    ui->editUsername->clear();
    ui->editPassword->clear();
    ui->editPosition->setCurrentIndex(-1);
    ui->editStaffCNumber->clear();
    ui->editStaffFirstName->clear();
    ui->editStaffLastName->clear();
    ui->editStatusLabel->clear();
    QSqlQueryModel *list= new QSqlQueryModel;
    q.exec("select * from staff");
    list->setQuery(q);
    ui->editStaffList->setModel(list);

}

void MainWindow::on_editStaffList_doubleClicked()
{
    int currentRow=ui->editStaffList->selectionModel()->currentIndex().row();
    QSqlQueryModel *list= new QSqlQueryModel;
    q.exec("select * from staff");
    list->setQuery(q);
    ui->editStaffFirstName->setText(list->record(currentRow).value(1).toString());
    ui->editStaffLastName->setText(list->record(currentRow).value(2).toString());
    ui->editUsername->setText(list->record(currentRow).value(3).toString());
    ui->editPassword->setText(list->record(currentRow).value(4).toString());
    ui->editPosition->setCurrentIndex(ui->editPosition->findText(list->record(currentRow).value(5).toString()));
    ui->editEmail->setText(list->record(currentRow).value(6).toString());
    ui->editStaffCNumber->setText(list->record(currentRow).value(7).toString());
    ui->editStatusLabel->setText(list->record(currentRow).value(9).toString());
}

void MainWindow::on_editChangeStatus_clicked()
{
    if(ui->editStatusLabel->text()=="ACTIVE"){
        q.exec("UPDATE `ics`.`STAFF` SET `Status`='DEACTIVATED' WHERE username='"+ui->editUsername->text()+"'");
        statusBar()->showMessage("Staff deactivated",3000);
    }
    else{
        q.exec("UPDATE `ics`.`STAFF` SET `Status`='ACTIVE' WHERE username='"+ui->editUsername->text()+"'");
        statusBar()->showMessage("Staff activated",3000);
    }
    on_editStaffLoad_clicked();
}


void MainWindow::on_editStaffButtonList_clicked()
{
    on_editStaffList_doubleClicked();
}


void MainWindow::on_cashierCallNext_clicked()
{
    double sum=0;
    q.exec("select * from queuetopay");
    q.next();
    QString idqueue=q.value(0).toString();
    cashierPatientID=q.value(1).toString();
    QStringList labels;
    labels << tr("Doctor")<<tr("Record Number") << tr("Service")<<tr("Detail")<<tr("Charge");
    ui->cashierToPayList->setRowCount(0);
    ui->cashierToPayList->horizontalHeader()->show();
    ui->cashierToPayList->setHorizontalHeaderLabels(labels);
    q.exec("SELECT p.idPatient,p.fName,p.lName,r.idDoctor 'Doctor',r.treatmentID 'Treatment',r.treatmentDetails 'Treatment Details',r.charge 'Charge',sum(i.amountPaid) 'Paid ',r.idrecord from record r left join patient p on r.idPatient=p.idPatient left join invoice i on r.idPatient =p.idPatient AND r.idrecord = i.idRecord where r.idPatient='"+cashierPatientID+"'  group by r.idrecord");
    while(q.next()){
        if(q.value(6).toInt()==q.value(7))
            continue;
        double remaining=q.value(6).toDouble()-q.value(7).toDouble();
        sum+=remaining;
        QString remainingString=QString::number(remaining);
        ui->cashierFirstName->setText(q.value(1).toString());
        ui->cashierLastName->setText(q.value(2).toString());
        ui->cashierToPayList->insertRow(0);
        ui->cashierToPayList->setItem (0,0,new QTableWidgetItem(q.value(3).toString())); //doctor
        ui->cashierToPayList->item(0,0)->setFlags(ui->cashierToPayList->item(0,0)->flags() & ~Qt::ItemIsEditable);
        ui->cashierToPayList->setItem (0,1,new QTableWidgetItem(q.value(8).toString())); //recordid
        ui->cashierToPayList->item(0,1)->setFlags(ui->cashierToPayList->item(0,1)->flags() & ~Qt::ItemIsEditable);
        ui->cashierToPayList->setItem (0,2,new QTableWidgetItem(q.value(4).toString())); //service
        ui->cashierToPayList->item(0,2)->setFlags(ui->cashierToPayList->item(0,2)->flags() & ~Qt::ItemIsEditable);
        ui->cashierToPayList->setItem (0,3,new QTableWidgetItem(q.value(5).toString())); //detail
        ui->cashierToPayList->item(0,3)->setFlags(ui->cashierToPayList->item(0,3)->flags() & ~Qt::ItemIsEditable);
        ui->cashierToPayList->setItem (0,4,new QTableWidgetItem(remainingString)); //charge
        ui->cashierToPayList->item(0,4)->setFlags(ui->cashierToPayList->item(0,4)->flags() & ~Qt::ItemIsEditable);
        ui->cashierToPayList->setItem (0,5,new QTableWidgetItem("0")); //initialise payment to 0
    }
    ui->cashierTotal->setText(QString::number(sum));
    QFont font;
    font.setBold(1);
    font.setPointSize(14);
    ui->cashierTotal->setFont(font);
    q.exec("delete from queuetopay where idqueue='"+idqueue+"'");
    ui->cashierCallNext->setDisabled(1);
    ui->cashierFinaliseTransactionCheckBox->setEnabled(1);
    cashierLoaded=1;
}


void MainWindow::on_adminEditPatientButtonList_clicked()
{
    int currentRow=ui->adminEditPatientList->selectionModel()->currentIndex().row();
    QSqlQueryModel *list= new QSqlQueryModel;
    q.exec("select * from patient");
    list->setQuery(q);
    ui->adminEditPatientFirstName->setText(list->record(currentRow).value(1).toString());
    ui->adminEditPatientLastName->setText(list->record(currentRow).value(2).toString());
    ui->adminEditPatientGender->setCurrentIndex(ui->adminEditPatientGender->findText(list->record(currentRow).value(3).toString()));
    QDate date=date.fromString(q.value(4).toString(),"yyyy-MM-dd");
    ui->adminEditPatientDOB->setDate(date);
    ui->adminEditPatientID->setText(q.value(5).toString());
    ui->adminEditPatientCNumber->setText(list->record(currentRow).value(6).toString());
    ui->adminEditPatientAddress->setText(q.value(7).toString());
    ui->adminEditPatientState->setCurrentIndex(ui->adminEditPatientState->findText(q.value(8).toString()));
    ui->adminEditPatientPoscode->setText(q.value(9).toString());
    ui->adminEditPatientMedicalNotes->document()->setPlainText(q.value(10).toString());
    adminEditPatientID=q.value(0).toString();
}

void MainWindow::on_adminEditPatientLoadList_clicked()
{
    QSqlQueryModel *list= new QSqlQueryModel;
    q.exec("select * from patient");
    list->setQuery(q);
    ui->adminEditPatientList->setModel(list);
}

void MainWindow::on_adminEditPatientList_doubleClicked()
{
    on_adminEditPatientButtonList_clicked();
}

void MainWindow::on_adminEditPatientSave_clicked()
{
    QString fName=ui->adminEditPatientFirstName->text().toUpper(),
            lName=ui->adminEditPatientLastName->text().toUpper(),
            gender=ui->adminEditPatientGender->currentText(),
            dob=ui->adminEditPatientDOB->date().toString("yyyy-MM-dd"),
            id=ui->adminEditPatientID->text().toUpper(),
            cNumber=ui->adminEditPatientCNumber->text(),
            address=ui->adminEditPatientAddress->text().toUpper(),
            states=ui->adminEditPatientState->currentText(),
            poscode=ui->adminEditPatientPoscode->text(),
            medicalNotes=ui->adminEditPatientMedicalNotes->toPlainText();
    q.exec("UPDATE patient SET `fName`='"+fName+"', `lName`='"+lName+"', `Gender`='"+gender+"', `dob`='"+dob+"', `ICPassport`='"+id+"', `contactNumber`='"+cNumber+"', `address`='"+address+"', `pState`='"+states+"', `Poscode`='"+poscode+"', `medicalNotes`='"+medicalNotes+"' WHERE `idPatient`='"+adminEditPatientID+"'");
    if(q.lastError().isValid())
        statusBar()->showMessage("Error occurred while saving",3000);
    else
        statusBar()->showMessage("Saved successfully",3000);
}

void MainWindow::on_saveStaffEdit_clicked()
{

}

void MainWindow::on_receptionistAddNewPatient_clicked()
{
    QString fName=ui->receptionistfName->text().toUpper(),
            lName=ui->receptionistlName->text().toUpper(),
            gender=ui->receptionistgender->currentText(),
            dob=ui->receptionistdob->date().toString("yyyy-MM-dd"),
            id=ui->receptionistid->text().toUpper(),
            cNumber=ui->receptionistcNumber->text(),
            address=ui->receptionistAddress->text().toUpper(),
            states=ui->receptioniststate->currentText(),
            poscode=ui->receptionistposcode->text(),
            medicalNotes=ui->receptionistmedicalNotes->toPlainText();
    if(fName=="" || lName=="" || gender=="" || id=="" || cNumber=="")
        statusBar()->showMessage("Please fill all the required fields",3000);
    else{
        q.exec("select icpassport from patient where icpassport='"+id+"'");
        if(q.next())
            statusBar()->showMessage(tr("Patient already exist, try searching"));
        else{
            q.exec("INSERT INTO `ics`.`patient` (`fName`, `lName`, `Gender`, `dob`, `ICPassport`, `contactNumber`, `address`, `pState`, `Poscode`, `medicalNotes`, `registerDate`) VALUES ('"+fName+"', '"+lName+"', '"+gender+"', '"+dob+"', '"+id+"', '"+cNumber+"', '"+address+"', '"+states+"', '"+poscode+"', '"+medicalNotes+"', curdate())");
            if(q.lastError().isValid())
                statusBar()->showMessage("Error occurred while saving",3000);
            else{
                ui->receptionistfName->clear();
                ui->receptionistlName->clear();
                ui->receptionistgender->setCurrentIndex(-1);
                ui->receptionistid->clear();
                ui->receptionistcNumber->clear();
                ui->receptionistAddress->clear();
                ui->receptioniststate->setCurrentIndex(-1);
                ui->receptionistposcode->clear();
                ui->receptionistmedicalNotes->clear();
                statusBar()->showMessage("Saved successfully",3000);
            }
        }
    }
}

void MainWindow::on_receptionistSearchButton_clicked()
{
    q.exec("select idpatient,fname,lname from patient where icpassport='"+ui->receptionistSearchInput->text().toUpper()+"'");
    if(q.next()){
        ui->receptionistFirstName->setText(q.value(1).toString().toUpper());
        ui->receptionistLastName->setText(q.value(2).toString().toUpper());
        receptionistPatientID=q.value(0).toString().toUpper();
    }
    else
        statusBar()->showMessage("No patient found with that id.",3000);
}

void MainWindow::on_receptionistEnqueue_clicked()
{
    if(ui->receptionistQueueOption->currentText()=="Doctor"){
        q.exec("select idpatient from queuetoserve where idpatient='"+receptionistPatientID+"'");
        if(q.next())
            statusBar()->showMessage("Patient already queued.",2000);
        else if (ui->receptionistDoctorOption->currentText()=="Any doctor"){
            q.exec("INSERT INTO queuetoserve (`idPatient`,`idDoctor`) VALUES ('"+receptionistPatientID+"','0')");
            statusBar()->showMessage("Patient sent to queue.",2000);
        }
        else{
            q.exec("SELECT staffID FROM staff WHERE CONCAT(CONCAT(fName, ' ', lName),' ID:',staffid) = '"+ui->receptionistDoctorOption->currentText()+"'");
            q.next();
            //QString doctorID=
            q.exec("INSERT INTO queuetoserve (`idPatient`,`idDoctor`) VALUES ('"+receptionistPatientID+"',"+q.value(0).toString()+")");
            statusBar()->showMessage("Patient sent to queue.",2000);
        }
    }
    else if(ui->receptionistQueueOption->currentText()=="Cashier"){
        q.exec("SELECT idpatient from queuetopay where idpatient='"+receptionistPatientID+"'");
        if(q.next())
            statusBar()->showMessage("Patient already queued.",2000);
        else{
            q.exec("INSERT INTO queuetopay (`idPatient`) VALUES ('"+receptionistPatientID+"')");
            statusBar()->showMessage("Patient sent to queue.",2000);
        }
    }
}

void MainWindow::on_receptionistSearchByButton_clicked()
{
    QString searchByKey=ui->receptionistSearchKey->text().toUpper();
    int searchByVariable=ui->receptionistSearchVariable->currentIndex();
    switch (searchByVariable){
    case 0:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where fname='"+searchByKey+"'");
        break;
    case 1:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where lname='"+searchByKey+"'");
        break;
    case 2:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where gender='"+searchByKey+"'");
        break;
    case 3:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where pstate='"+searchByKey+"'");
        break;
    case 4:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT',dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where contactnumber='"+searchByKey+"'");
        break;
    case 5:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT',dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where poscode='"+searchByKey+"'");
        break;
    case 6:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT',dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient");
        break;
    }
    QSqlQueryModel *list= new QSqlQueryModel;
    list->setQuery(q);
    ui->receptionistSearchByList->setModel(list);
}

void MainWindow::on_receptionistEditPatient_clicked()
{
    if(ui->receptionistSearchByList->selectionModel()->currentIndex().row()>=0){
        ui->receptionistStack->setCurrentIndex(1);
        int currentRow=ui->receptionistSearchByList->selectionModel()->currentIndex().row();
        receptionistPatientID=ui->receptionistSearchByList->model()->data(ui->receptionistSearchByList->model()->index(currentRow,1)).toString();
        ui->receptionistEditFirstName->setText(ui->receptionistSearchByList->model()->data(ui->receptionistSearchByList->model()->index(currentRow,1)).toString());
        ui->receptionistEditLastName->setText(ui->receptionistSearchByList->model()->data(ui->receptionistSearchByList->model()->index(currentRow,2)).toString());
        ui->receptionistEditGender->setCurrentIndex(ui->receptionistEditGender->findText(ui->receptionistSearchByList->model()->data(ui->receptionistSearchByList->model()->index(currentRow,3)).toString()));
        QDate date=date.fromString(ui->receptionistSearchByList->model()->data(ui->receptionistSearchByList->model()->index(currentRow,5)).toString(),"yyyy-MM-dd");
        ui->receptionistEditDOB->setDate(date);
        ui->receptionistEditID->setText(ui->receptionistSearchByList->model()->data(ui->receptionistSearchByList->model()->index(currentRow,4)).toString());
        ui->receptionistEditContactNumber->setText(ui->receptionistSearchByList->model()->data(ui->receptionistSearchByList->model()->index(currentRow,6)).toString());
        ui->receptionistEditAddress->setText(ui->receptionistSearchByList->model()->data(ui->receptionistSearchByList->model()->index(currentRow,7)).toString());
        ui->receptionistEditState->setCurrentIndex(ui->receptionistEditState->findText(ui->receptionistSearchByList->model()->data(ui->receptionistSearchByList->model()->index(currentRow,8)).toString()));
        ui->receptionistEditPoscode->setText(ui->receptionistSearchByList->model()->data(ui->receptionistSearchByList->model()->index(currentRow,9)).toString());
        ui->receptionistEditMedicalNotes->document()->setPlainText(ui->receptionistSearchByList->model()->data(ui->receptionistSearchByList->model()->index(currentRow,10)).toString());
    }
}

void MainWindow::on_receptionistBack_clicked()
{
    on_receptionistSearchByButton_clicked();
    ui->receptionistStack->setCurrentIndex(0);
}

void MainWindow::on_receptionistEditSave_clicked()
{
    QString fName=ui->receptionistEditFirstName->text().toUpper(),
            lName=ui->receptionistEditLastName->text().toUpper(),
            gender=ui->receptionistEditGender->currentText(),
            dob=ui->receptionistEditDOB->date().toString("yyyy-MM-dd"),
            id=ui->receptionistEditID->text().toUpper(),
            cNumber=ui->receptionistEditContactNumber->text(),
            address=ui->receptionistEditAddress->text().toUpper(),
            states=ui->receptionistEditState->currentText(),
            poscode=ui->receptionistEditPoscode->text(),
            medicalNotes=ui->receptionistEditMedicalNotes->toPlainText();
    q.exec("UPDATE patient SET `fName`='"+fName+"', `lName`='"+lName+"', `Gender`='"+gender+"', `dob`='"+dob+"', `ICPassport`='"+id+"', `contactNumber`='"+cNumber+"', `address`='"+address+"', `pState`='"+states+"', `Poscode`='"+poscode+"', `medicalNotes`='"+medicalNotes+"' WHERE `idPatient`='"+receptionistPatientID+"'");
    if(q.lastError().isValid())
        statusBar()->showMessage(tr("Error occurred while saving"));
    else{
        clearReceptionist();
        statusBar()->showMessage(tr("Saved successfully"));
    }
}

void MainWindow::on_receptionistSearchByList_doubleClicked()
{
    int currentRow=ui->receptionistSearchByList->selectionModel()->currentIndex().row();
    ui->receptionistSearchInput->setText(ui->receptionistSearchByList->model()->data(ui->receptionistSearchByList->model()->index(currentRow,4)).toString());
    on_receptionistSearchButton_clicked();
}


void MainWindow::on_doctorCallNext_clicked()
{
    q.exec("SELECT p.idPatient,p.fName,p.lName,q.idqueue,p.medicalnotes FROM patient p INNER JOIN queuetoserve q ON q.idPatient= p.idPatient WHERE q.idDoctor="+staffID+" order by idqueue");
    q.next();
    ui->doctorSendForPaymentCheckBox->setEnabled(1);
    ui->doctorRemoveRowCheckBox->setEnabled(1);
    QString doctorIDQueue=q.value(3).toString();
    ui->doctorFirstName->setText(q.value(1).toString());
    ui->doctorLastName->setText(q.value(2).toString());
    ui->doctorMedicalNotes->document()->setPlainText(q.value(4).toString());
    doctorPatientID=q.value(0).toString();
    q.exec("SELECT r.workArea 'Work Area',r.idDoctor 'Doctor',r.treatmentID 'Treatment',r.treatmentDetails 'Treatment Details',r.charge 'Charge',sum(i.amountPaid) 'Paid ', r.dateDone 'Date' from record r left join invoice i on r.idPatient ='"+doctorPatientID+"' AND r.idrecord = i.idRecord where r.idPatient='"+doctorPatientID+"' group by r.idrecord");
    QSqlQueryModel *list= new QSqlQueryModel;
    list->setQuery(q);
    ui->doctorRecordList->setModel(list);
    q.exec("delete from queuetoserve where idqueue='"+doctorIDQueue+"'");
}


void MainWindow::on_doctorChangeStack_clicked()
{
    if(ui->doctorRecordStackWidget->currentIndex()==0){
        ui->doctorChangeStack->setText("View past record");
        ui->doctorRecordLabel->setText("Current record:");
        ui->doctorRecordStackWidget->setCurrentIndex(1);
    }
    else{
        ui->doctorRecordLabel->setText("Record:");
        ui->doctorChangeStack->setText("View current record");
        ui->doctorRecordStackWidget->setCurrentIndex(0);
    }
}

void MainWindow::on_commandLinkButton_clicked()
{
    int rowCount=ui->doctorCurrentRecordList->rowCount()-1;
    while(rowCount!=-1){
        q.exec("INSERT INTO record (`idPatient`, `idDoctor`, `treatmentID`, `workArea`, `treatmentDetails`, `charge`, `dateDone`) VALUES ('"+doctorPatientID+"', '"+staffID+"', '"+ui->doctorCurrentRecordList->item(rowCount,1)->text()+"', '"+ui->doctorCurrentRecordList->item(rowCount,0)->text()+"', '"+ui->doctorCurrentRecordList->item(rowCount,2)->text()+"', '"+ui->doctorCurrentRecordList->item(rowCount,3)->text()+"', curdate())");
        rowCount--;
    }
    if(q.lastError().isValid())
        statusBar()->showMessage(tr("Error occurred while saving"));
    else{
        statusBar()->showMessage(tr("Saved successfully"));
        q.exec("INSERT INTO queuetopay (`idpatient`) VALUES ('"+doctorPatientID+"')");
        clearDoctor();
    }
}

void MainWindow::on_doctorAddRecord_clicked()
{
    QString charge;
    if(ui->doctorRecordStackWidget->currentIndex()==0)
        on_doctorChangeStack_clicked();
    if(ui->doctorCharge->text()==NULL)
        charge="0";
    else
        charge=ui->doctorCharge->text();
    QStringList labels;
    labels << tr("Working Area") <<tr("Service") << tr("Description")<<tr("Charge");
    ui->doctorCurrentRecordList->horizontalHeader()->show();
    ui->doctorCurrentRecordList->setHorizontalHeaderLabels(labels);
    ui->doctorCurrentRecordList->insertRow(0);
    ui->doctorCurrentRecordList->setItem (0,0,new QTableWidgetItem(ui->doctorWorkingArea->text()));
    ui->doctorCurrentRecordList->setItem (0,1,new QTableWidgetItem(ui->doctorService->currentText()));
    ui->doctorCurrentRecordList->setItem (0,2,new QTableWidgetItem(ui->doctorTreatmentDetails->toPlainText()));
    ui->doctorCurrentRecordList->setItem (0,3,new QTableWidgetItem(charge));
    ui->doctorCharge->clear();
    ui->doctorTreatmentDetails->clear();
    ui->doctorWorkingArea->clear();
}

void MainWindow::on_doctorSendForPaymentCheckBox_clicked()
{
    if(ui->commandLinkButton->isEnabled())
        ui->commandLinkButton->setEnabled(0);
    else
        ui->commandLinkButton->setEnabled(1);
}

void MainWindow::on_doctorRemoveRowCheckBox_clicked()
{
    if(ui->doctorRemoveCurrentRecordRow->isEnabled())
        ui->doctorRemoveCurrentRecordRow->setEnabled(0);
    else
        ui->doctorRemoveCurrentRecordRow->setEnabled(1);
}

void MainWindow::on_doctorRemoveCurrentRecordRow_clicked()
{
    ui->doctorRemoveRowCheckBox->setChecked(0);
    ui->doctorRemoveCurrentRecordRow->setEnabled(0);
    int currentRow=ui->doctorCurrentRecordList->selectionModel()->currentIndex().row();
    ui->doctorCurrentRecordList->removeRow(currentRow);
}

void MainWindow::on_cashierToPayList_itemChanged()
{
    if(cashierLoaded==1){
        double sum=0;
        for(int i=0;i<ui->cashierToPayList->rowCount();i++){
            if(ui->cashierToPayList->item(i,5)->text().toDouble()>ui->cashierToPayList->item(i,4)->text().toDouble()){
                statusBar()->showMessage("Overpaying, please enter the correct amount.",3500);
                ui->cashierToPayList->setItem(i,5,new QTableWidgetItem("0"));
            }
            int decimal=0;
            bool valid=1;
            QString cellValue=ui->cashierToPayList->item(i,5)->text();
            for (int i=0;i<cellValue.size();i++){
                if(cellValue[i].isDigit())
                    continue;
                else
                    if(cellValue[i]=='.'){
                        decimal++;
                        if(decimal==2){
                            valid=0;
                            break;
                        }
                    }
                    else{
                        valid=0;
                        break;
                    }
            }
            if(!valid){
                ui->cashierToPayList->setItem(i,5,new QTableWidgetItem("0"));
                statusBar()->showMessage("Invalid input, please enter the correct amount.",3500);
            }
            else
                sum+=ui->cashierToPayList->item(i,5)->text().toDouble();
        }
        ui->cashierAmountPaying->setText(QString::number(sum));
        double toPayLeft=ui->cashierTotal->text().toDouble()-ui->cashierAmountPaying->text().toDouble();
        ui->cashierAmountLeftToPay->setText(QString::number(toPayLeft));
        QFont font;
        font.setBold(1);
        font.setPointSize(14);
        ui->cashierAmountPaying->setFont(font);
        ui->cashierAmountLeftToPay->setFont(font);
    }
}

void MainWindow::on_cashierFinaliseTransactionCheckBox_clicked()
{
    if(ui->cashierFinaliseTransaction->isEnabled())
        ui->cashierFinaliseTransaction->setEnabled(0);
    else
        ui->cashierFinaliseTransaction->setEnabled(1);
}

void MainWindow::on_cashierFinaliseTransaction_clicked()
{
    //createReceipt();
    bool sqlError=0;
    for(int i=0;i<ui->cashierToPayList->rowCount();i++){
        if(ui->cashierToPayList->item(i,5)->text()!="0"){
            q.exec("INSERT INTO invoice (`amountPaid`, `patientID`, `idDoctor`, `idRecord`, `dateDone`,`cashierID`) VALUES ('"+ui->cashierToPayList->item(i,5)->text()+"', '"+cashierPatientID+"', '"+ui->cashierToPayList->item(i,0)->text()+"', '"+ui->cashierToPayList->item(i,1)->text()+"', curdate(),'"+staffID+"')");
            if(q.lastError().isValid())
                sqlError=1;
        }
    }
    clearCashier();
    if(sqlError)
        statusBar()->showMessage("Error occured while saving",3000);
    else
        statusBar()->showMessage("Saved",3000);
    ui->cashierFinaliseTransaction->setEnabled(0);
    ui->cashierFinaliseTransactionCheckBox->setEnabled(0);
    ui->cashierFinaliseTransactionCheckBox->setChecked(0);
}

void MainWindow::clearDoctor(){
    ui->doctorRecordList->setModel(NULL);
    ui->doctorRecordStackWidget->setCurrentIndex(0);
    ui->doctorFirstName->clear();
    ui->doctorLastName->clear();
    ui->doctorMedicalNotes->clear();
    ui->doctorWorkingArea->clear();
    ui->doctorService->setCurrentIndex(-1);
    ui->doctorTreatmentDetails->clear();
    ui->doctorCharge->clear();
    ui->doctorCurrentRecordList->setRowCount(0);
    ui->commandLinkButton->setDisabled(1);
    ui->doctorRemoveRowCheckBox->setDisabled(1);
    ui->doctorSendForPaymentCheckBox->setDisabled(1);
    ui->doctorRemoveRowCheckBox->setChecked(0);
    ui->doctorSendForPaymentCheckBox->setChecked(0);
    doctorPatientID.clear();
}

void MainWindow::clearReceptionist(){
    receptionistPatientID.clear();
    ui->receptionistSearchInput->clear();
    ui->receptionistSearchKey->clear();
    ui->receptionistFirstName->clear();
    ui->receptionistLastName->clear();
    ui->receptionistSearchByList->setModel(NULL);
    ui->receptionistAddress->clear();
    ui->receptionistcNumber->clear();
    ui->receptionistdob->clear();
    ui->receptionistEditAddress->clear();
    ui->receptionistEditContactNumber->clear();
    ui->receptionistEditDOB->clear();
    ui->receptionistEditFirstName->clear();
    ui->receptionistEditLastName->clear();
    ui->receptionistEditID->clear();
    ui->receptionistEditMedicalNotes->clear();
    ui->receptionistEditPoscode->clear();
    ui->receptionistEditState->setCurrentIndex(-1);
    ui->receptionistTab->setCurrentIndex(0);
}

void MainWindow::clearAdmin(){
    adminEditPatientID.clear();
    ui->adminEditPatientAddress->clear();
    ui->adminEditPatientCNumber->clear();
    ui->adminEditPatientDOB->clear();
    ui->adminEditPatientFirstName->clear();
    ui->adminEditPatientGender->setCurrentIndex(-1);
    ui->adminEditPatientID->clear();
    ui->adminEditPatientLastName->clear();
    ui->adminEditPatientList->setModel(NULL);
    ui->adminEditPatientMedicalNotes->clear();
    ui->adminEditPatientPoscode->clear();
    ui->adminEditPatientState->setCurrentIndex(-1);
    ui->adminRecordsList->setModel(NULL);
    ui->registerStaffFirstName->clear();
    ui->registerStaffLastName->clear();
    ui->registerStaffCNumber->clear();
    ui->registerStaffEmail->clear();
    ui->registerUsername->clear();
    ui->registerPassword->clear();
    ui->registerPosition->setCurrentIndex(-1);
    ui->editEmail->clear();
    ui->editUsername->clear();
    ui->editPassword->clear();
    ui->editPosition->setCurrentIndex(-1);
    ui->editStaffCNumber->clear();
    ui->editStaffFirstName->clear();
    ui->editStaffLastName->clear();
    ui->editStatusLabel->clear();
    ui->editStaffList->setModel(NULL);
    ui->adminTab->setCurrentIndex(0);
    adminUncheck();
    adminClearRecord();
}

void MainWindow::clearCashier(){
    cashierLoaded=0;
    cashierPatientID.clear();
    ui->cashierAmountLeftToPay->setText("0000.00");
    ui->cashierAmountPaying->setText("0000.00");
    ui->cashierTotal->setText("0000.00");
    ui->cashierToPayList->setRowCount(0);
    ui->cashierFirstName->clear();
    ui->cashierLastName->clear();
    QFont font;
    font.setBold(1);
    font.setPointSize(14);
    ui->cashierAmountPaying->setFont(font);
    ui->cashierAmountLeftToPay->setFont(font);
    ui->cashierTotal->setFont(font);
    ui->cashierFinaliseTransaction->setEnabled(0);
    ui->cashierFinaliseTransactionCheckBox->setEnabled(0);
    ui->cashierTab->setCurrentIndex(0);
}

//void MainWindow::createReceipt()
//{
//        QFile data("lastReceipt.html");
//        q.exec("select curdate(),curtime()");
//        q.next();
//        if (data.open(QFile::WriteOnly | QFile::Truncate)) {
//            QTextStream out(&data);
//            out << "<html>"
//                   "<head>"
//                   "<style>"
//                   "<!--"
//                   "/* Font Definitions */"
//                   "@font-face"
//                   "{font-family:\"MS Mincho\";"
//                   "panose-1:2 2 6 9 4 2 5 8 3 4;}"
//                   "@font-face"
//                   "{font-family:\"MS Gothic\";"
//                   "panose-1:2 11 6 9 7 2 5 8 2 4;}"
//                   "@font-face"
//                   "{font-family:\"Cambria Math\";"
//                   "panose-1:2 4 5 3 5 4 6 3 2 4;}"
//                   "@font-face"
//                   "{font-family:\"Calibri Light\";"
//                   "panose-1:2 15 3 2 2 2 4 3 2 4;}"
//                   "@font-face"
//                   "{font-family:Calibri;"
//                   "panose-1:2 15 5 2 2 2 4 3 2 4;}"
//                   "@font-face"
//                   "{font-family:\"Arial\";"
//                   "panose-1:2 11 6 9 7 2 5 8 2 4;}"
//                   "@font-face"
//                   "{font-family:\"Arial\";"
//                   "panose-1:2 2 6 9 4 2 5 8 3 4;}"
//                   "/* Style Definitions */"
//                   "p.MsoNormal, li.MsoNormal, div.MsoNormal"
//                   "{margin-top:0in;"
//                   "margin-right:0in;"
//                   "margin-bottom:8.0pt;"
//                   "margin-left:0in;"
//                   "line-height:107%;"
//                   "font-size:11.0pt;"
//                   "font-family:\"Calibri\",sans-serif;}"
//                   "h1"
//                   "{mso-style-link:\"Heading 1 Char\";"
//                   "margin-top:12.0pt;"
//                   "margin-right:0in;"
//                   "margin-bottom:0in;"
//                   "margin-left:0in;"
//                   "margin-bottom:.0001pt;"
//                   "line-height:107%;"
//                   "page-break-after:avoid;"
//                   "font-size:16.0pt;"
//                   "font-family:\"Calibri Light\",sans-serif;"
//                   "color:#2E74B5;"
//                   "font-weight:normal;}"
//                   "p.MsoTitle, li.MsoTitle, div.MsoTitle"
//                   "{mso-style-link:\"Title Char\";"
//                   "margin:0in;"
//                   "margin-bottom:.0001pt;"
//                   "font-size:28.0pt;"
//                   "font-family:\"Calibri Light\",sans-serif;"
//                   "letter-spacing:-.5pt;}"
//                   "p.MsoTitleCxSpFirst, li.MsoTitleCxSpFirst, div.MsoTitleCxSpFirst"
//                   "{mso-style-link:\"Title Char\";"
//                   "margin:0in;"
//                   "margin-bottom:.0001pt;"
//                   "font-size:28.0pt;"
//                   "font-family:\"Calibri Light\",sans-serif;"
//                   "letter-spacing:-.5pt;}"
//                   "p.MsoTitleCxSpMiddle, li.MsoTitleCxSpMiddle, div.MsoTitleCxSpMiddle"
//                   "{mso-style-link:\"Title Char\";"
//                   "margin:0in;"
//                   "margin-bottom:.0001pt;"
//                   "font-size:28.0pt;"
//                   "font-family:\"Calibri Light\",sans-serif;"
//                   "letter-spacing:-.5pt;}"
//                   "p.MsoTitleCxSpLast, li.MsoTitleCxSpLast, div.MsoTitleCxSpLast"
//                   "{mso-style-link:\"Title Char\";"
//                   "margin:0in;"
//                   "margin-bottom:.0001pt;"
//                   "font-size:28.0pt;"
//                   "font-family:\"Calibri Light\",sans-serif;"
//                   "letter-spacing:-.5pt;}"
//                   "p.MsoNoSpacing, li.MsoNoSpacing, div.MsoNoSpacing"
//                   "{margin:0in;"
//                   "margin-bottom:.0001pt;"
//                   "font-size:11.0pt;"
//                   "font-family:\"Calibri\",sans-serif;}"
//                   "span.Heading1Char"
//                   "{mso-style-name:\"Heading 1 Char\";"
//                   "mso-style-link:\"Heading 1\";"
//                   "font-family:\"Calibri Light\",sans-serif;"
//                   "color:#2E74B5;}"
//                   "span.TitleChar"
//                   "{mso-style-name:\"Title Char\";"
//                   "mso-style-link:Title;"
//                   "font-family:\"Calibri Light\",sans-serif;"
//                   "letter-spacing:-.5pt;}"
//                   ".MsoPapDefault"
//                   "{margin-bottom:8.0pt;"
//                   "line-height:107%;}"
//                   "@page WordSection1"
//                   "{size:297.7pt 419.6pt;"
//                   "margin:.5in .5in .5in .5in;}"
//                   "div.WordSection1"
//                   "{page:WordSection1;}"
//                   "-->"
//                   "</style>"
//                   "</head>"
//                   "<body lang=EN-GB>"
//                   "<div class=WordSection1>"
//                   "<p class=MsoTitle><span lang=EN-US>NAZARICK CLINIC</span></p>"
//                   "<p class=MsoNoSpacing><span lang=EN-US>"<<ui->cashierFirstName->text()<<" "<<ui->cashierLastName->text()<<"</span></p>"
//                                                                                                                              "<p class=MsoNoSpacing><span lang=EN-US>"<<q.value(0).toString()
//                <<" "
//               <<q.value(1).toString()
//              <<"</span></p>"
//                "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt'><span"
//                "lang=EN-US>&nbsp;</span></p>"
//                "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt'><b><span"
//                "lang=EN-US>&nbsp;</span></b></p>"
//                "<table class=MsoTable15Plain3 border=0 cellspacing=0 cellpadding=0"
//                "style='border-collapse:collapse'>"
//                "<tr>"
//                "<td width=72 valign=top style='width:53.75pt;border:none;border-bottom:solid #7F7F7F 1.0pt;"
//                "padding:0in 5.4pt 0in 5.4pt'>"
//                "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                "normal'><b><sup><span lang=EN-US style='font-size:18.0pt;text-transform:uppercase'>DOCTOR</span></sup></b></p>"
//                "</td>"
//                "<td width=121 valign=top style='width:91.1pt;border:none;border-bottom:solid #7F7F7F 1.0pt;"
//                "padding:0in 5.4pt 0in 5.4pt'>"
//                "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                "normal'><b><sup><span lang=EN-US style='font-size:18.0pt;text-transform:uppercase'>SERVICE</span></sup></b></p>"
//                "</td>"
//                "<td width=107 valign=top style='width:91.1pt;border:none;border-bottom:solid #7F7F7F 1.0pt;"
//                "padding:0in 5.4pt 0in 5.4pt'>"
//                "<p class=MsoNormal align=right style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                "normal'><b><sup><span lang=EN-US style='font-size:18.0pt;text-transform:uppercase'>CHARGE</span></sup></b></p>"
//                "</td>"
//                "</tr>";
//            for(int i=0;i<ui->cashierToPayList->rowCount();i++){
//                double balance=ui->cashierToPayList->item(i,4)->text().toDouble()-ui->cashierToPayList->item(i,5)->text().toDouble();
//                q.exec("select fname,lname,staffid from staff where staffid='"+ui->cashierToPayList->item(i,0)->text()+"'");
//                q.next();
//                out<<"<tr>"
//                     "<td width=72 valign=top style='width:53.75pt;border:none;border-right:solid #7F7F7F 1.0pt;"
//                     "background:#F2F2F2;padding:0in 5.4pt 0in 5.4pt'>"
//                     "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                     "normal'><b><span lang=EN-US style='font-size:9.0pt;text-transform:uppercase'>"
//                  <<q.value(0).toString()
//                 <<" "
//                <<q.value(1).toString()
//                <<" ID:"
//                <<q.value(2).toString()
//                <<"</span></b></p>"
//                  "</td>"
//                  "<td width=121 valign=top style='width:91.1pt;background:#F2F2F2;padding:0in 5.4pt 0in 5.4pt'>"
//                  "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                  "normal'><span lang=EN-US style='font-size:9.0pt'>"
//                <<ui->cashierToPayList->item(i,2)->text()
//                <<"</span></p>"
//                  "</td>"
//                  "<td width=107 valign=top style='width:80.35pt;background:#F2F2F2;padding:"
//                  "0in 5.4pt 0in 5.4pt'>"
//                  "<p class=MsoNormal align=right style='margin-bottom:0in;margin-bottom:.0001pt;"
//                  "text-align:right;line-height:normal'><span lang=EN-US style='font-size:9.0pt'>RM "
//                <<ui->cashierToPayList->item(i,4)->text()
//                <<"</span></p>"
//                  "</td>"
//                  "</tr>"
//                  "<tr>"
//                  "<td width=72 valign=top style='width:53.75pt;border:none;border-right:solid #7F7F7F 1.0pt;"
//                  "padding:0in 5.4pt 0in 5.4pt'>"
//                  "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                  "normal'><b><span lang=EN-US style='font-size:9.0pt;text-transform:uppercase'>&nbsp;</span></b></p>"
//                  "</td>"
//                  "<td width=121 valign=top style='width:91.1pt;padding:0in 5.4pt 0in 5.4pt'>"
//                  "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                  "normal'><span lang=EN-US style='font-size:9.0pt'>-PAID</span></p>"
//                  "</td>"
//                  "<td width=107 valign=top style='width:80.35pt;padding:0in 5.4pt 0in 5.4pt'>"
//                  "<p class=MsoNormal align=right style='margin-bottom:0in;margin-bottom:.0001pt;"
//                  "text-align:right;line-height:normal'><span lang=EN-US style='font-size:9.0pt'>RM "
//                <<ui->cashierToPayList->item(i,5)->text()
//                <<"</span></p>"
//                  "</td>"
//                  "</tr>"
//                  "<tr>"
//                  "<td width=72 valign=top style='width:53.75pt;border:none;border-right:solid #7F7F7F 1.0pt;"
//                  "background:#F2F2F2;padding:0in 5.4pt 0in 5.4pt'>"
//                  "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                  "normal'><b><span lang=EN-US style='font-size:9.0pt;text-transform:uppercase'>&nbsp;</span></b></p>"
//                  "</td>"
//                  "<td width=121 valign=top style='width:91.1pt;background:#F2F2F2;padding:0in 5.4pt 0in 5.4pt'>"
//                  "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                  "normal'><span lang=EN-US style='font-size:9.0pt'>-BALANCE</span></p>"
//                  "</td>"
//                  "<td width=107 valign=top style='width:80.35pt;background:#F2F2F2;padding:"
//                  "0in 5.4pt 0in 5.4pt'>"
//                  "<p class=MsoNormal align=right style='margin-bottom:0in;margin-bottom:.0001pt;"
//                  "text-align:right;line-height:normal'><span lang=EN-US style='font-size:9.0pt'>RM "
//                <<balance
//                <<"</span></p>"
//                  "</td>"
//                  "</tr>"
//                  "<tr>"
//                  "<td width=72 valign=top style='width:53.75pt;border:none;border-right:solid #7F7F7F 1.0pt;"
//                  "padding:0in 5.4pt 0in 5.4pt'>"
//                  "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                  "normal'><b><span lang=EN-US style='font-size:9.0pt;text-transform:uppercase'>&nbsp;</span></b></p>"
//                  "</td>"
//                  "<td width=121 valign=top style='width:91.1pt;padding:0in 5.4pt 0in 5.4pt'>"
//                  "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                  "normal'><span lang=EN-US style='font-size:9.0pt'>&nbsp;</span></p>"
//                  "</td>"
//                  "<td width=107 valign=top style='width:80.35pt;padding:0in 5.4pt 0in 5.4pt'>"
//                  "<p class=MsoNormal align=right style='margin-bottom:0in;margin-bottom:.0001pt;"
//                  "text-align:right;line-height:normal'><span lang=EN-US style='font-size:9.0pt'>&nbsp;</span></p>"
//                  "</td>""</tr>";
//            }
//            q.exec("select fname,lname from staff where staffid='"+staffID+"'");
//            q.next();
//            out<<"<tr>"
//                 "<td width=72 valign=top style='width:53.75pt;border:none;border-right:solid #7F7F7F 1.0pt;"
//                 "background:#F2F2F2;padding:0in 5.4pt 0in 5.4pt'>"
//                 "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                 "normal'><span lang=EN-US style='font-size:9.0pt;text-transform:uppercase'>&nbsp;</span></p>"
//                 "</td>"
//                 "<td width=121 valign=top style='width:91.1pt;background:#F2F2F2;padding:0in 5.4pt 0in 5.4pt'>"
//                 "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//                 "normal'><b><span lang=EN-US style='font-size:9.0pt'>TOTAL CHARGE</span></b></p>"
//                 "</td>"
//                 "<td width=107 valign=top style='width:80.35pt;background:#F2F2F2;padding:"
//                 "0in 5.4pt 0in 5.4pt'>"
//                 "<p class=MsoNormal align=right style='margin-bottom:0in;margin-bottom:.0001pt;"
//                 "text-align:right;line-height:normal'><span lang=EN-US style='font-size:9.0pt'>RM "
//              <<ui->cashierTotal->text()
//             <<"</span></p>"
//               "</td>"
//               "</tr>"
//               "<tr>"
//               "<td width=72 valign=top style='width:53.75pt;border:none;border-right:solid #7F7F7F 1.0pt;"
//               "padding:0in 5.4pt 0in 5.4pt'>"
//               "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//               "normal'><span lang=EN-US style='font-size:9.0pt;text-transform:uppercase'>&nbsp;</span></p>"
//               "</td>"
//               "<td width=121 valign=top style='width:91.1pt;padding:0in 5.4pt 0in 5.4pt'>"
//               "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//               "normal'><b><span lang=EN-US style='font-size:9.0pt'>TOTAL PAID</span></b></p>"
//               "</td>"
//               "<td width=107 valign=top style='width:80.35pt;padding:0in 5.4pt 0in 5.4pt'>"
//               "<p class=MsoNormal align=right style='margin-bottom:0in;margin-bottom:.0001pt;"
//               "text-align:right;line-height:normal'><span lang=EN-US style='font-size:9.0pt'>RM "
//            <<ui->cashierAmountPaying->text()
//            <<"</span></p>"
//              "</td>"
//              "</tr>"
//              "<tr>"
//              "<td width=72 valign=top style='width:53.75pt;border:none;border-right:solid #7F7F7F 1.0pt;"
//              "background:#F2F2F2;padding:0in 5.4pt 0in 5.4pt'>"
//              "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//              "normal'><span lang=EN-US style='font-size:9.0pt;text-transform:uppercase'>&nbsp;</span></p>"
//              "</td>"
//              "<td width=121 valign=top style='width:91.1pt;background:#F2F2F2;padding:0in 5.4pt 0in 5.4pt'>"
//              "<p class=MsoNormal style='margin-bottom:0in;margin-bottom:.0001pt;line-height:"
//              "normal'><b><span lang=EN-US style='font-size:9.0pt'>BALANCE</span></b></p>"
//              "</td>"
//              "<td width=107 valign=top style='width:80.35pt;background:#F2F2F2;padding:"
//              "0in 5.4pt 0in 5.4pt'>"                                                                                                                              "<p class=MsoNormal align=right style='margin-bottom:0in;margin-bottom:.0001pt;"                                                                                                                    "text-align:right;line-height:normal'><span lang=EN-US style='font-size:9.0pt'>RM "<<ui->cashierAmountLeftToPay->text()<<"</span></p>""</td>""</tr>""</table>""<p class=MsoNormal><span lang=EN-US style='font-size:9.0pt;line-height:107%'>&nbsp;</span></p>""<p class=MsoNoSpacing><span lang=EN-US>SERVED BY</span></p>""<p class=MsoNormal><span lang=EN-US style='font-size:9.0pt;line-height:107%'>"
//            <<q.value(0).toString()
//            <<" "
//            <<q.value(1).toString()
//            <<" ID:"
//            <<staffID
//            <<"</span></p>""</div>""</body>""</html>";
//        }
//        data.close();
//    QWebEngineView *view = new QWebEngineView;
//    view->load(QUrl("file:///C:/Users/zarf/Documents/build-Workshop-Refactored-Desktop_Qt_5_9_1_MSVC2015_64bit-Release/lastReceipt.html"));
//    view->setFixedHeight(600);
//    view->setFixedWidth(400);
//    view->show();
//}

void MainWindow::on_financeChartGenerateType_currentIndexChanged(const QString &arg1)
{
    ui->financeChartGenerateChoice2->clear();
    ui->financeChartGenerateChoice2->setEnabled(0);
    QDate start(QDate::currentDate());
    QDate end(QDate::currentDate());
    ui->financeStartDate->setDate(start);
    ui->financeEndDate->setDate(end);
    if(arg1=="Individual Doctor Earning Report"){
        q.exec("SELECT CONCAT(fName, ' ', lName) FROM   staff where position='DOCTOR'");
        ui->financeChartGenerateChoice2->setEnabled(1);
        while(q.next()){
            ui->financeChartGenerateChoice2->addItem(q.value(0).toString());
        }
    }
    ui->financeChartGenerateChoice1->setCurrentIndex(-1);
    ui->financeStartDate->setEnabled(0);
    ui->financeEndDate->setEnabled(0);
}

void MainWindow::on_financeChartGenerateChoice1_currentIndexChanged(const QString &arg1)
{
    ui->financeStartDate->setEnabled(1);
    ui->financeEndDate->setEnabled(1);
    if(arg1=="Daily"){
        ui->financeStartDate->setDisplayFormat("MMMM/yyyy");
        ui->financeEndDate->setDisplayFormat("MMMM/yyyy");
        ui->financeEndDate->setEnabled(0);
    }
    if(arg1=="Monthly"){
        ui->financeStartDate->setDisplayFormat("yyyy");
        ui->financeEndDate->setEnabled(0);
    }
    if(arg1=="Yearly"){
        ui->financeStartDate->setDisplayFormat("yyyy");
        ui->financeEndDate->setDisplayFormat("yyyy");
    }
}
QLineSeries* MainWindow::generateDataSetDaily(QStringList *xAxis,int numberOfDays){
    QLineSeries *series = new QLineSeries();
    QSqlQueryModel *list= new QSqlQueryModel;
    list->setQuery(q);
    ui->financeReportTable->setModel(list);
    q.next();
    for(int i=1;i<=numberOfDays;i   ++){
        QString ith=QString::number(i);
        if(i==1)
            ith+="st";
        else
            if(i==2)
                ith+="nd";
            else
                if(i==3)
                    ith+="rd";
                else
                    if(i==31)
                        ith+="st";
                    else
                        ith+="th";
        xAxis->append(ith);
        if(q.value(0).toString().toInt()==i){
            series->append(i-1,q.value(1).toDouble());
            q.next();
        }
        else{
            series->append(i-1,0);
        }
    }
    return series;
}

QLineSeries *MainWindow::generateDataSetMonthly()
{
    QLineSeries *series = new QLineSeries();
    QSqlQueryModel *list= new QSqlQueryModel;
    list->setQuery(q);
    ui->financeReportTable->setModel(list);
    q.next();
    for (int i=1;i<=12;i++){
        if(i==q.value(0).toInt()){
            series->append(i-1,q.value(1).toDouble());
            q.next();
        }
        else{
            series->append(i-1,0);
        }
    }
    return series;
}

QLineSeries *MainWindow::generateDataSetYearly(QStringList *xAxis)
{
    QLineSeries *series = new QLineSeries();
    QSqlQueryModel *list= new QSqlQueryModel;
    list->setQuery(q);
    ui->financeReportTable->setModel(list);
    q.next();
    for(int i=ui->financeStartDate->text().toInt(),j=0;i<=ui->financeEndDate->text().toInt();i++,j++){
        xAxis->append(QString::number(i));
        if(q.value(0).toInt()==i){
            series->append(j,q.value(1).toDouble());
            q.next();
        }
        else{
            series->append(j,0);
        }
    }
    return series;
}

void MainWindow::on_financeGenerateChart_clicked()
{
    QLineSeries *series;
    QStringList xAxis;
    QBarCategoryAxis *axis = new QBarCategoryAxis();
    if(ui->financeChartGenerateType->currentText()=="Total Earning Report"){
        if(ui->financeChartGenerateChoice1->currentText()=="Daily"){
            axis->setTitleText("Days of the month");
            q.exec("select day(dateDone) 'Date',sum(amountPaid) 'RM',day(last_day(datedone)) from invoice where year(dateDone)='"+ui->financeStartDate->date().toString("yyyy")+"' AND month(dateDone)='"+ui->financeStartDate->date().toString("MM")+"' group by dateDone order by dateDone");
            q.next();
            int lastDay=q.value(2).toInt();
            q.exec("select day(dateDone) 'Date',sum(amountPaid) 'RM' from invoice where year(dateDone)='"+ui->financeStartDate->date().toString("yyyy")+"' AND month(dateDone)='"+ui->financeStartDate->date().toString("MM")+"' group by dateDone order by dateDone");
            series=generateDataSetDaily(&xAxis,lastDay);
        }
        if(ui->financeChartGenerateChoice1->currentText()=="Monthly"){
            axis->setTitleText("Month");
            xAxis<<"January"<<"February"<<"March"<<"April"<<"May"<<"June"<<"July"<<"August"<<"September"<<"October"<<"November"<<"December";
            //q.exec("select date_format(datedone,'%Y-%m'),sum(amountPaid) from invoice where dateDone>='"+ui->financeStartDate->date().toString("yyyy")+"' AND dateDone<='"+ui->financeEndDate->date().toString("yyyy")+"' group by year(dateDone),month(dateDone) order by year(datedone),month(dateDone)");
            q.exec("select month(datedone) 'Month',sum(amountPaid) 'RM' from invoice where year(dateDone)='"+ui->financeStartDate->date().toString("yyyy")+"'  group by month(dateDone) order by month(dateDone)");
            series=generateDataSetMonthly();

        }
        if(ui->financeChartGenerateChoice1->currentText()=="Yearly"){
            axis->setTitleText("Year");
            q.exec("select year(dateDone) 'Year',sum(amountPaid) 'RM' from invoice where year(dateDone)>='"+ui->financeStartDate->date().toString("yyyy")+"' AND year(dateDone)<='"+ui->financeEndDate->date().toString("yyyy")+"' group by year(dateDone) order by year(dateDone)");
            series=generateDataSetYearly(&xAxis);
        }
    }
    else if(ui->financeChartGenerateType->currentText()=="Patient Registration Count Report"){
        if(ui->financeChartGenerateChoice1->currentText()=="Daily"){
            axis->setTitleText("Days of the month");
            q.exec("select day(registerDate),count(idPatient),day(last_day(registerDate)) from patient where year(registerDate)='"+ui->financeStartDate->date().toString("yyyy")+"' AND month(registerDate)='"+ui->financeStartDate->date().toString("MM")+"' group by registerDate order by RegisterDate");
            q.next();
            int lastDay=q.value(2).toInt();
            q.exec("select day(registerDate) 'Date',count(idPatient) 'Number Of New Patient Registered' from patient where year(registerDate)='"+ui->financeStartDate->date().toString("yyyy")+"' AND month(registerDate)='"+ui->financeStartDate->date().toString("MM")+"' group by registerDate order by RegisterDate");
            series=generateDataSetDaily(&xAxis,lastDay);
        }

        if(ui->financeChartGenerateChoice1->currentText()=="Monthly"){
            axis->setTitleText("Month");
            xAxis<<"January"<<"February"<<"March"<<"April"<<"May"<<"June"<<"July"<<"August"<<"September"<<"October"<<"November"<<"December";
            q.exec("select month(registerDate) 'Month',count(idpatient) 'Number Of New Patient Registered' from patient where year(registerDate)='"+ui->financeStartDate->date().toString("yyyy")+"'  group by month(registerDate) order by month(registerDate)");
            series=generateDataSetMonthly();

        }
        if(ui->financeChartGenerateChoice1->currentText()=="Yearly"){
            axis->setTitleText("Year");
            q.exec("select year(registerDate) 'Year',count(idpatient) 'Number Of New Patient Registered' from patient where year(registerDate)>='"+ui->financeStartDate->date().toString("yyyy")+"' AND year(registerDate)<='"+ui->financeEndDate->date().toString("yyyy")+"' group by year(registerDate) order by year(registerDate)");
            series=generateDataSetYearly(&xAxis);
        }
    }
    else if(ui->financeChartGenerateType->currentText()=="Individual Doctor Earning Report"){
        if(ui->financeChartGenerateChoice1->currentText()=="Daily"){
            axis->setTitleText("Days of the month");
            q.exec("select day(dateDone) 'Date',sum(amountPaid) 'RM',day(last_day(datedone)) from invoice where year(dateDone)='"+ui->financeStartDate->date().toString("yyyy")+"' AND month(dateDone)='"+ui->financeStartDate->date().toString("MM")+"' AND idDoctor=(select staffID from staff where CONCAT(fName, ' ', lName)='"+ui->financeChartGenerateChoice2->currentText()+"') group by dateDone order by dateDone");
            q.next();
            int lastDay=q.value(2).toInt();
            q.exec("select day(dateDone) 'Date',sum(amountPaid) 'RM' from invoice where year(dateDone)='"+ui->financeStartDate->date().toString("yyyy")+"' AND month(dateDone)='"+ui->financeStartDate->date().toString("MM")+"' AND idDoctor=(select staffID from staff where CONCAT(fName, ' ', lName)='"+ui->financeChartGenerateChoice2->currentText()+"') group by dateDone order by dateDone");
            series=generateDataSetDaily(&xAxis,lastDay);
        }

        if(ui->financeChartGenerateChoice1->currentText()=="Monthly"){
            axis->setTitleText("Month");
            xAxis<<"January"<<"February"<<"March"<<"April"<<"May"<<"June"<<"July"<<"August"<<"September"<<"October"<<"November"<<"December";
            q.exec("select month(dateDone) 'Month',sum(amountPaid) 'RM' from invoice where year(dateDone)='"+ui->financeStartDate->date().toString("yyyy")+"' AND idDoctor=(select staffID from staff where CONCAT(fName, ' ', lName)='"+ui->financeChartGenerateChoice2->currentText()+"') group by month(dateDone) order by month(dateDone)");
            series=generateDataSetMonthly();
        }
        if(ui->financeChartGenerateChoice1->currentText()=="Yearly"){
            axis->setTitleText("Year");
            q.exec("select year(dateDone) 'Year',sum(amountPaid) 'RM' from invoice where year(dateDone)>='"+ui->financeStartDate->date().toString("yyyy")+"' AND year(dateDone)<='"+ui->financeEndDate->date().toString("yyyy")+"' AND idDoctor=(select staffID from staff where CONCAT(fName, ' ', lName)='"+ui->financeChartGenerateChoice2->currentText()+"') group by year(dateDone) order by year(dateDone)");
            series=generateDataSetYearly(&xAxis);
        }
    }
    else if(ui->financeChartGenerateType->currentText()=="Patient Count Report"){
        if(ui->financeChartGenerateChoice1->currentText()=="Daily"){
            axis->setTitleText("Days of the month");
            q.exec("select day(dateDone) 'Date',count(idPatient),day(last_day(dateDone)) 'Number Of Patient Served' from record where year(dateDone)='"+ui->financeStartDate->date().toString("yyyy")+"' AND month(dateDone)='"+ui->financeStartDate->date().toString("MM")+"' group by dateDone order by dateDone");
            q.next();
            int lastDay=q.value(2).toInt();
            q.exec("select day(dateDone) 'Date',count(idPatient) 'Number Of Patient Served' from record where year(dateDone)='"+ui->financeStartDate->date().toString("yyyy")+"' AND month(dateDone)='"+ui->financeStartDate->date().toString("MM")+"' group by dateDone order by dateDone");
            series=generateDataSetDaily(&xAxis,lastDay);
        }

        if(ui->financeChartGenerateChoice1->currentText()=="Monthly"){
            axis->setTitleText("Month");
            xAxis<<"January"<<"February"<<"March"<<"April"<<"May"<<"June"<<"July"<<"August"<<"September"<<"October"<<"November"<<"December";
            q.exec("select month(dateDone) 'Month',count(idpatient) 'Number Of Patient Served' from record where year(dateDone)='"+ui->financeStartDate->date().toString("yyyy")+"'  group by month(dateDone) order by month(dateDone)");
            series=generateDataSetMonthly();

        }
        if(ui->financeChartGenerateChoice1->currentText()=="Yearly"){
            axis->setTitleText("Year");
            q.exec("select year(dateDone) 'Year',count(dateDone) 'Number Of Patient Served' from record where year(dateDone)>='"+ui->financeStartDate->date().toString("yyyy")+"' AND year(dateDone)<='"+ui->financeEndDate->date().toString("yyyy")+"' group by year(dateDone) order by year(dateDone)");
            series=generateDataSetYearly(&xAxis);
        }
    }

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();

    QValueAxis *y=new QValueAxis;
    y->setTickCount(20);
    y->setLabelFormat("%.2f");
    if(ui->financeChartGenerateType->currentText()=="Total Earning Report" || ui->financeChartGenerateType->currentText()=="Individual Doctor Earning Report")
        y->setTitleText("RM");
    else
        y->setTitleText("Patient Count");
    y->setMin(0);

    axis->append(xAxis);

    chart->setTitle(ui->financeChartGenerateType->currentText());



    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->chart()->setAxisY(y,series);
    chartView->chart()->setAxisX(axis,series);
    chartView->resize(1350,500);
    chartView->show();
}

void MainWindow::on_financeChartGenerateChoice2_currentIndexChanged(const int &arg1)
{
    if(arg1==-1)
        ui->financeChartGenerateChoice2->setEnabled(0);
}

void MainWindow::on_cashierSearchByButton_clicked()
{
    QString searchByKey=ui->cashierSearchKey->text().toUpper();
    int searchByVariable=ui->cashierSearchVariable->currentIndex();
    switch (searchByVariable){
    case 0:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where fname='"+searchByKey+"'");
        break;
    case 1:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where lname='"+searchByKey+"'");
        break;
    case 2:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where gender='"+searchByKey+"'");
        break;
    case 3:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where pstate='"+searchByKey+"'");
        break;
    case 4:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT',dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where contactnumber='"+searchByKey+"'");
        break;
    case 5:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT',dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where poscode='"+searchByKey+"'");
        break;
    case 6:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT',dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient");
        break;
    }
    QSqlQueryModel *list= new QSqlQueryModel;
    list->setQuery(q);
    ui->cashierSearchByList->setModel(list);
}

void MainWindow::on_cashierEditPatient_clicked()
{
    int currentRow=ui->cashierSearchByList->selectionModel()->currentIndex().row();
    int patientID=ui->cashierSearchByList->model()->data(ui->cashierSearchByList->model()->index(currentRow,0)).toInt();
    QStringList labels;
    labels << tr("Invoice Number")<<tr("Doctor ID") << tr("Date Paid")<<tr("Paid");
    ui->cashierInvoiceList->setRowCount(0);
    ui->cashierInvoiceList->horizontalHeader()->show();
    ui->cashierInvoiceList->setHorizontalHeaderLabels(labels);
    q.exec("SELECT idinvoice 'Invoice Number', iddoctor 'Doctor ID',datedone 'Date Paid',amountpaid 'Paid' FROM INVOICE where patientid='"+QString::number(patientID)+"'");
    ui->cashierPatientInvoiceStack->setCurrentIndex(1);
    while(q.next()){
        ui->cashierInvoiceList->insertRow(0);
        ui->cashierInvoiceList->setItem (0,0,new QTableWidgetItem(q.value(0).toString())); //Invoice number
        ui->cashierInvoiceList->item(0,0)->setFlags(ui->cashierInvoiceList->item(0,0)->flags() & ~Qt::ItemIsEditable);
        ui->cashierInvoiceList->setItem (0,1,new QTableWidgetItem(q.value(1).toString())); //Doctor ID
        ui->cashierInvoiceList->item(0,1)->setFlags(ui->cashierInvoiceList->item(0,1)->flags() & ~Qt::ItemIsEditable);
        ui->cashierInvoiceList->setItem (0,2,new QTableWidgetItem(q.value(2).toString())); //Date
        ui->cashierInvoiceList->item(0,2)->setFlags(ui->cashierInvoiceList->item(0,2)->flags() & ~Qt::ItemIsEditable);
        ui->cashierInvoiceList->setItem (0,3,new QTableWidgetItem(q.value(3).toString())); //Paid
    }
}

void MainWindow::on_cashierInvoiceList_clicked()
{
    ui->cashierCommitChangeCheckBox->setEnabled(1);
    ui->cashierDeleteInvoiceCheckBox->setEnabled(1);
}

void MainWindow::on_cashierCommitChangeCheckBox_clicked()
{
    if(ui->cashierCommitChange->isEnabled())
        ui->cashierCommitChange->setEnabled(0);
    else
        ui->cashierCommitChange->setEnabled(1);
}

void MainWindow::on_cashierDeleteInvoiceCheckBox_clicked()
{
    if(ui->cashierDeleteInvoice->isEnabled())
        ui->cashierDeleteInvoice->setEnabled(0);
    else
        ui->cashierDeleteInvoice->setEnabled(1);
}

void MainWindow::on_cashierChangeCashierStack_clicked()
{
    ui->cashierPatientInvoiceStack->setCurrentIndex(0);
    cashierUncheck();

}

void MainWindow::on_cashierCommitChange_clicked()
{
    for(int i=0;i<ui->cashierInvoiceList->rowCount();i++)
        q.exec("UPDATE invoice SET amountPaid='"+ui->cashierInvoiceList->item(i,3)->text()+"' WHERE idinvoice='"+ui->cashierInvoiceList->item(i,0)->text()+"'");
    cashierUncheck();
    on_cashierEditPatient_clicked();
}

void MainWindow::on_cashierDeleteInvoice_clicked()
{
    int currentRow=ui->cashierInvoiceList->selectionModel()->currentIndex().row();
    QString toDelete=ui->cashierInvoiceList->model()->data(ui->cashierInvoiceList->model()->index(currentRow,0)).toString();
    q.exec("DELETE FROM invoice WHERE idinvoice='"+toDelete+"'");
    cashierUncheck();
    on_cashierEditPatient_clicked();

}

void MainWindow::cashierUncheck()
{
    ui->cashierCommitChangeCheckBox->setEnabled(0);
    ui->cashierCommitChange->setEnabled(0);
    ui->cashierCommitChangeCheckBox->setChecked(0);
    ui->cashierDeleteInvoiceCheckBox->setEnabled(0);
    ui->cashierDeleteInvoiceCheckBox->setChecked(0);
    ui->cashierDeleteInvoice->setEnabled(0);
}

void MainWindow::adminUncheck()
{
    ui->adminInvoiceCommitChangeCheckBox->setEnabled(0);
    ui->adminInvoiceCommitChange->setEnabled(0);
    ui->adminInvoiceCommitChangeCheckBox->setChecked(0);
    ui->adminInvoiceDeleteInvoiceCheckBox->setEnabled(0);
    ui->adminInvoiceDeleteInvoiceCheckBox->setChecked(0);
    ui->adminInvoiceDeleteInvoice->setEnabled(0);
}

void MainWindow::on_adminSearchRecordButton_clicked()
{
    q.exec("select r.idrecord 'Record ID',CONCAT(p.fname, ' ',p.lname) 'Patient Name', CONCAT(CONCAT(s.fName, ' ', s.lName),' ID:',s.staffid) 'Doctor Name',r.workarea 'Working Area',r.treatmentid 'Service', r.treatmentdetails 'Treatment Details', r.charge 'Charge' from record r inner join staff s on s.staffID=r.idDoctor inner join patient p on p.idPatient=r.idPatient where r.idpatient=(select p.idpatient from patient p where p.icpassport='"+ui->adminSearchRecord->text().toUpper()+"')");
    QSqlQueryModel *list= new QSqlQueryModel;
    list->setQuery(q);
    ui->adminRecordsList->setModel(list);
    adminClearRecord();
}

void MainWindow::on_adminRecordsList_doubleClicked()
{
    int currentRow=ui->adminRecordsList->selectionModel()->currentIndex().row();
    ui->adminDoctorNameEdit->clear();
    q.exec("SELECT CONCAT(CONCAT(fName, ' ', lName),' ID:',staffid) FROM   staff where position='DOCTOR'");
    while(q.next()){
        ui->adminDoctorNameEdit->addItem(q.value(0).toString());
    }
    ui->adminRecordIDEdit->setText(ui->adminRecordsList->model()->data(ui->adminRecordsList->model()->index(currentRow,0)).toString());
    ui->adminPatientNameEdit->setText(ui->adminRecordsList->model()->data(ui->adminRecordsList->model()->index(currentRow,1)).toString());
    ui->adminTreatmentDetailsEdit->document()->setPlainText(ui->adminRecordsList->model()->data(ui->adminRecordsList->model()->index(currentRow,5)).toString());
    ui->adminWorkingAreaEdit->setText(ui->adminRecordsList->model()->data(ui->adminRecordsList->model()->index(currentRow,3)).toString());
    ui->adminServiceEdit->setCurrentIndex(ui->adminServiceEdit->findText(ui->adminRecordsList->model()->data(ui->adminRecordsList->model()->index(currentRow,4)).toString()));
    ui->adminChargeEdit->setText(ui->adminRecordsList->model()->data(ui->adminRecordsList->model()->index(currentRow,6)).toString());
    ui->adminDoctorNameEdit->setCurrentIndex(ui->adminDoctorNameEdit->findText(ui->adminRecordsList->model()->data(ui->adminRecordsList->model()->index(currentRow,2)).toString()));
}

void MainWindow::on_adminEditRecord_clicked()
{
    QString doctorID=ui->adminDoctorNameEdit->currentText();
    doctorID=doctorID[doctorID.size()-1];
    statusBar()->showMessage(doctorID,3000);
    q.exec("UPDATE record SET idDoctor='"+doctorID+"', treatmentID='"+ui->adminServiceEdit->currentText()+"', workArea='"+ui->adminWorkingAreaEdit->text()+"', treatmentDetails='"+ui->adminTreatmentDetailsEdit->toPlainText()+"', charge='"+ui->adminChargeEdit->text()+"' WHERE idrecord='"+ui->adminRecordIDEdit->text()+"'");
    if(!q.lastError().isValid())
        statusBar()->showMessage("Updated",3000);
    else
        statusBar()->showMessage("Error occurred, not saved",3000);
    on_adminSearchRecordButton_clicked();
}

void MainWindow::on_adminInvoiceEditPatient_clicked()
{
    int currentRow=ui->adminInvoiceSearchByList->selectionModel()->currentIndex().row();
    int patientID=ui->adminInvoiceSearchByList->model()->data(ui->adminInvoiceSearchByList->model()->index(currentRow,0)).toInt();
    QStringList labels;
    labels << tr("Invoice Number")<<tr("Doctor ID") << tr("Date Paid")<<tr("Paid");
    ui->adminInvoiceList->setRowCount(0);
    ui->adminInvoiceList->horizontalHeader()->show();
    ui->adminInvoiceList->setHorizontalHeaderLabels(labels);
    q.exec("SELECT idinvoice 'Invoice Number', iddoctor 'Doctor ID',datedone 'Date Paid',amountpaid 'Paid' FROM INVOICE where patientid='"+QString::number(patientID)+"'");
    ui->adminInvoicePatientStack->setCurrentIndex(1);
    while(q.next()){
        ui->adminInvoiceList->insertRow(0);
        ui->adminInvoiceList->setItem (0,0,new QTableWidgetItem(q.value(0).toString())); //Invoice number
        ui->adminInvoiceList->item(0,0)->setFlags(ui->adminInvoiceList->item(0,0)->flags() & ~Qt::ItemIsEditable);
        ui->adminInvoiceList->setItem (0,1,new QTableWidgetItem(q.value(1).toString())); //Doctor ID
        ui->adminInvoiceList->item(0,1)->setFlags(ui->adminInvoiceList->item(0,1)->flags() & ~Qt::ItemIsEditable);
        ui->adminInvoiceList->setItem (0,2,new QTableWidgetItem(q.value(2).toString())); //Date
        ui->adminInvoiceList->item(0,2)->setFlags(ui->adminInvoiceList->item(0,2)->flags() & ~Qt::ItemIsEditable);
        ui->adminInvoiceList->setItem (0,3,new QTableWidgetItem(q.value(3).toString())); //Paid
    }
}

void MainWindow::on_adminInvoiceSearchByButton_clicked()
{
    QString searchByKey=ui->adminInvoiceSearchKey->text().toUpper();
    int searchByVariable=ui->adminInvoiceSearchVariable->currentIndex();
    switch (searchByVariable){
    case 0:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where fname='"+searchByKey+"'");
        break;
    case 1:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where lname='"+searchByKey+"'");
        break;
    case 2:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where gender='"+searchByKey+"'");
        break;
    case 3:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where pstate='"+searchByKey+"'");
        break;
    case 4:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT',dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where contactnumber='"+searchByKey+"'");
        break;
    case 5:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT',dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where poscode='"+searchByKey+"'");
        break;
    case 6:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT',dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient");
        break;
    }
    QSqlQueryModel *list= new QSqlQueryModel;
    list->setQuery(q);
    ui->adminInvoiceSearchByList->setModel(list);
}

void MainWindow::on_adminInvoiceCommitChangeCheckBox_clicked()
{
    if(ui->adminInvoiceCommitChange->isEnabled())
        ui->adminInvoiceCommitChange->setEnabled(0);
    else
        ui->adminInvoiceCommitChange->setEnabled(1);
}

void MainWindow::on_adminInvoiceDeleteInvoiceCheckBox_clicked()
{
    if(ui->adminInvoiceDeleteInvoice->isEnabled())
        ui->adminInvoiceDeleteInvoice->setEnabled(0);
    else
        ui->adminInvoiceDeleteInvoice->setEnabled(1);
}

void MainWindow::on_adminInvoiceCommitChange_clicked()
{
    for(int i=0;i<ui->adminInvoiceList->rowCount();i++)
        q.exec("UPDATE invoice SET amountPaid='"+ui->adminInvoiceList->item(i,3)->text()+"' WHERE idinvoice='"+ui->adminInvoiceList->item(i,0)->text()+"'");
    adminUncheck();
    on_adminInvoiceEditPatient_clicked();
}

void MainWindow::on_adminInvoiceDeleteInvoice_clicked()
{
    int currentRow=ui->adminInvoiceList->selectionModel()->currentIndex().row();
    QString toDelete=ui->adminInvoiceList->model()->data(ui->adminInvoiceList->model()->index(currentRow,0)).toString();
    q.exec("DELETE FROM invoice WHERE idinvoice='"+toDelete+"'");
    adminUncheck();
    on_adminInvoiceEditPatient_clicked();
}

void MainWindow::on_adminInvoiceChangeStack_clicked()
{
    ui->adminInvoicePatientStack->setCurrentIndex(0);
    adminUncheck();
}

void MainWindow::on_adminInvoiceList_clicked()
{
    ui->adminInvoiceCommitChangeCheckBox->setEnabled(1);
    ui->adminInvoiceDeleteInvoiceCheckBox->setEnabled(1);
}

void MainWindow::adminClearRecord()
{
    ui->adminRecordIDEdit->clear();
    ui->adminPatientNameEdit->clear();
    ui->adminTreatmentDetailsEdit->clear();
    ui->adminWorkingAreaEdit->clear();
    ui->adminServiceEdit->setCurrentIndex(-1);
    ui->adminChargeEdit->clear();
    ui->adminDoctorNameEdit->clear();
}

void MainWindow::doctorClearRecord()
{
    ui->doctorRecordIDEdit->clear();
    ui->doctorPatientNameEdit->clear();
    ui->doctorTreatmentDetailsEdit->clear();
    ui->doctorWorkingAreaEdit->clear();
    ui->doctorServiceEdit->setCurrentIndex(-1);
    ui->doctorChargeEdit->clear();
    ui->doctorDoctorNameEdit->clear();
}

void MainWindow::on_doctorSearchByButton_clicked()
{
    QString searchByKey=ui->doctorSearchKey->text().toUpper();
    int searchByVariable=ui->doctorSearchVariable->currentIndex();
    ui->doctorEditPatient->setEnabled(0);
    ui->doctorPatientRecordStack->setCurrentIndex(0);
    switch (searchByVariable){
    case 0:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where fname='"+searchByKey+"'");
        break;
    case 1:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where lname='"+searchByKey+"'");
        break;
    case 2:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where gender='"+searchByKey+"'");
        break;
    case 3:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT', dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where pstate='"+searchByKey+"'");
        break;
    case 4:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT',dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where contactnumber='"+searchByKey+"'");
        break;
    case 5:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT',dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient where poscode='"+searchByKey+"'");
        break;
    case 6:q.exec("select idpatient AS 'PATIENT ID', fname as 'FIRST NAME',lname as 'LAST NAME',GENDER,icpassport as 'IC/PASSPORT',dob as 'DATE OF BIRTH',contactnumber as 'CONTACT NUMBER', ADDRESS,pstate as 'STATE', POSCODE,MEDICALNOTES AS 'MEDICAL NOTES', REGISTERDATE AS 'REGISTER DATE' from patient");
        break;
    }
    QSqlQueryModel *list= new QSqlQueryModel;
    list->setQuery(q);
    ui->doctorSearchByList->setModel(list);
}
void MainWindow::on_doctorEditPatient_clicked()
{
    if(ui->doctorPatientRecordStack->currentIndex()==0){
        int currentRow=ui->doctorSearchByList->selectionModel()->currentIndex().row();
        QString patientID=ui->doctorSearchByList->model()->data(ui->doctorSearchByList->model()->index(currentRow,0)).toString();
        q.exec("select r.idrecord 'Record ID',CONCAT(p.fname, ' ',p.lname) 'Patient Name', CONCAT(CONCAT(s.fName, ' ', s.lName),' ID:',s.staffid) 'Doctor Name',r.workarea 'Working Area',r.treatmentid 'Service', r.treatmentdetails 'Treatment Details', r.charge 'Charge', r.dateDone 'Date' from record r inner join staff s on s.staffID=r.idDoctor inner join patient p on p.idPatient=r.idPatient where r.idPatient='"+patientID+"'");
        QSqlQueryModel *list= new QSqlQueryModel;
        list->setQuery(q);
        ui->doctorRecordsList->setModel(list);
        doctorClearRecord();
        ui->doctorPatientRecordStack->setCurrentIndex(1);
        ui->doctorEditPatient->setEnabled(0);
    }
    else{
        on_doctorRecordsList_doubleClicked();
    }
}


void MainWindow::on_doctorEditRecord_clicked()
{
    QString doctorID=ui->doctorDoctorNameEdit->currentText();
    doctorID=doctorID[doctorID.size()-1];
    statusBar()->showMessage(doctorID,3000);
    q.exec("UPDATE record SET idDoctor='"+doctorID+"', treatmentID='"+ui->doctorServiceEdit->currentText()+"', workArea='"+ui->doctorWorkingAreaEdit->text()+"', treatmentDetails='"+ui->doctorTreatmentDetailsEdit->toPlainText()+"', charge='"+ui->doctorChargeEdit->text()+"' WHERE idrecord='"+ui->doctorRecordIDEdit->text()+"'");
    if(!q.lastError().isValid()){
        int currentRow=ui->doctorSearchByList->selectionModel()->currentIndex().row();
        QString patientID=ui->doctorSearchByList->model()->data(ui->doctorSearchByList->model()->index(currentRow,0)).toString();
        q.exec("select r.idrecord 'Record ID',CONCAT(p.fname, ' ',p.lname) 'Patient Name', CONCAT(CONCAT(s.fName, ' ', s.lName),' ID:',s.staffid) 'Doctor Name',r.workarea 'Working Area',r.treatmentid 'Service', r.treatmentdetails 'Treatment Details', r.charge 'Charge' from record r inner join staff s on s.staffID=r.idDoctor inner join patient p on p.idPatient=r.idPatient where r.idPatient='"+patientID+"'");
        QSqlQueryModel *list= new QSqlQueryModel;
        list->setQuery(q);
        ui->doctorRecordsList->setModel(list);
        doctorClearRecord();
        statusBar()->showMessage("Updated",3000);
    }
    else
        statusBar()->showMessage("Error occurred, not saved",3000);
    on_adminSearchRecordButton_clicked();
}

void MainWindow::on_doctorChangeDoctorStack_clicked()
{
    ui->doctorPatientRecordStack->setCurrentIndex(0);
    ui->doctorEditPatient->setEnabled(0);
}

void MainWindow::on_doctorRecordsList_doubleClicked()
{
    int currentRow=ui->doctorRecordsList->selectionModel()->currentIndex().row();
    ui->doctorDoctorNameEdit->clear();
    q.exec("SELECT CONCAT(CONCAT(fName, ' ', lName),' ID:',staffid) FROM   staff where position='DOCTOR'");
    while(q.next()){
        ui->doctorDoctorNameEdit->addItem(q.value(0).toString());
    }
    ui->doctorRecordIDEdit->setText(ui->doctorRecordsList->model()->data(ui->doctorRecordsList->model()->index(currentRow,0)).toString());
    ui->doctorPatientNameEdit->setText(ui->doctorRecordsList->model()->data(ui->doctorRecordsList->model()->index(currentRow,1)).toString());
    ui->doctorDoctorNameEdit->setCurrentIndex(ui->doctorDoctorNameEdit->findText(ui->doctorRecordsList->model()->data(ui->doctorRecordsList->model()->index(currentRow,2)).toString()));
    ui->doctorWorkingAreaEdit->setText(ui->doctorRecordsList->model()->data(ui->doctorRecordsList->model()->index(currentRow,3)).toString());
    ui->doctorServiceEdit->setCurrentIndex(ui->doctorServiceEdit->findText(ui->doctorRecordsList->model()->data(ui->doctorRecordsList->model()->index(currentRow,4)).toString()));
    ui->doctorTreatmentDetailsEdit->document()->setPlainText(ui->doctorRecordsList->model()->data(ui->doctorRecordsList->model()->index(currentRow,5)).toString());
    ui->doctorChargeEdit->setText(ui->doctorRecordsList->model()->data(ui->doctorRecordsList->model()->index(currentRow,6)).toString());
}

void MainWindow::on_doctorSearchByList_doubleClicked()
{
    on_doctorEditPatient_clicked();
}

void MainWindow::on_doctorSearchByList_clicked()
{
    ui->doctorEditPatient->setEnabled(1);
}



void MainWindow::on_receptionistQueueOption_currentIndexChanged(int index)
{
    if (index==0){
        ui->receptionistDoctorOption->setEnabled(1);
        q.exec("SELECT CONCAT(CONCAT(fName, ' ', lName),' ID:',staffid) FROM staff where position='DOCTOR'");
        while(q.next())
            ui->receptionistDoctorOption->addItem(q.value(0).toString());
        ui->receptionistDoctorOption->addItem("Any doctor");
    }
    else{
        ui->receptionistDoctorOption->clear();
        ui->receptionistDoctorOption->setEnabled(0);
    }
}

void MainWindow::on_doctorCurrentRecordList_doubleClicked(const QModelIndex &index)
{

}
