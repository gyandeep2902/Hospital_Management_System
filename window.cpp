#include "window.h"
#include "ui_window.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileDialog>
#include <QTextStream>

Window::Window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Window)
{
    ui->setupUi(this);
    setFixedSize(600, 650);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("patients.db");
    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", db.lastError().text());
        return;
    }

    QSqlQuery query;
    // CORRECTED: The "transaction" column is now enclosed in double quotes
    // to avoid conflict with the SQL reserved keyword.
    query.exec("CREATE TABLE IF NOT EXISTS patients ("
               "id TEXT PRIMARY KEY, "
               "name TEXT, "
               "dob TEXT, "
               "doctor TEXT, "
               "blood TEXT, "
               "date_admitted TEXT, "
               "date_released TEXT, "
               "amount TEXT, "
               "payment TEXT, "
               "\"transaction\" TEXT)"); // <-- FIX HERE

    for (int d = 1; d <= 31; ++d) {
        QString s = QString("%1").arg(d, 2, 10, QChar('0'));
        ui->dateAdmit_day->addItem(s);
        ui->dateRelease_day->addItem(s);
    }
    for (int m = 1; m <= 12; ++m) {
        QString s = QString("%1").arg(m, 2, 10, QChar('0'));
        ui->dateAdmit_month->addItem(s);
        ui->dateRelease_month->addItem(s);
    }
    for (int y = 1950; y <= 2100; ++y) {
        QString s = QString::number(y);
        ui->dateAdmit_year->addItem(s);
        ui->dateRelease_year->addItem(s);
    }

    ui->comboBox_blood->addItems({"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"});
    ui->comboBox_payment->addItems({"Cash", "Card", "UPI", "Other"});

    connect(ui->submitButtonMenu, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->pageSubmit);
    });
    connect(ui->findButtonMenu, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->pageFind);
    });
    connect(ui->backButtonSubmit, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->pageMenu);
    });
    connect(ui->backButtonFind, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->pageMenu);
    });
    connect(ui->submit_button, &QPushButton::clicked, this, &Window::on_submit_clicked);
    connect(ui->find_button, &QPushButton::clicked, this, &Window::on_find_clicked);
    connect(ui->download_button, &QPushButton::clicked, this, &Window::on_download_clicked);
}

Window::~Window() {
    delete ui;
}

QString Window::getFormattedDate(QComboBox *day, QComboBox *month, QComboBox *year) {
    return QString("%1/%2/%3")
    .arg(day->currentText())
        .arg(month->currentText())
        .arg(year->currentText());
}

void Window::on_submit_clicked() {
    QString id = ui->lineEdit_id->text().trimmed();
    QString name = ui->lineEdit_name->text().trimmed();
    QString dob = ui->lineEdit_dob->text().trimmed();
    QString doctor = ui->lineEdit_doctor->text().trimmed();
    QString blood = ui->comboBox_blood->currentText();
    QString dateAdmitted = getFormattedDate(ui->dateAdmit_day, ui->dateAdmit_month, ui->dateAdmit_year);
    QString dateReleased = getFormattedDate(ui->dateRelease_day, ui->dateRelease_month, ui->dateRelease_year);
    QString amount = ui->lineEdit_amount->text().trimmed();
    QString payment = ui->comboBox_payment->currentText();
    QString transaction = ui->lineEdit_transaction->text().trimmed();

    if (id.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Patient ID and Name are required.");
        return;
    }

    QSqlQuery query;
    // CORRECTED: Also quote the "transaction" column name in the INSERT statement.
    query.prepare("INSERT INTO patients (id, name, dob, doctor, blood, date_admitted, date_released, amount, payment, \"transaction\") "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"); // <-- FIX HERE
    query.addBindValue(id);
    query.addBindValue(name);
    query.addBindValue(dob);
    query.addBindValue(doctor);
    query.addBindValue(blood);
    query.addBindValue(dateAdmitted);
    query.addBindValue(dateReleased);
    query.addBindValue(amount);
    query.addBindValue(payment);
    query.addBindValue(transaction);

    if (query.exec()) {
        QMessageBox::information(this, "Success", "Patient details submitted successfully.");
        ui->lineEdit_id->clear();
        ui->lineEdit_name->clear();
        ui->lineEdit_dob->clear();
        ui->lineEdit_doctor->clear();
        ui->lineEdit_amount->clear();
        ui->lineEdit_transaction->clear();
        ui->comboBox_blood->setCurrentIndex(0);
        ui->comboBox_payment->setCurrentIndex(0);
    } else {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
    }
}

void Window::on_find_clicked() {
    QString id = ui->lineEdit_find_id->text().trimmed();
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a Patient ID.");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM patients WHERE id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next()) {
        QString info = QString(
                           "<b>Patient ID:</b> %1<br>"
                           "<b>Name:</b> %2<br>"
                           "<b>DOB:</b> %3<br>"
                           "<b>Doctor:</b> %4<br>"
                           "<b>Blood Group:</b> %5<br>"
                           "<b>Date Admitted:</b> %6<br>"
                           "<b>Date Released:</b> %7<br>"
                           "<b>Amount Paid:</b> %8<br>"
                           "<b>Payment:</b> %9<br>"
                           "<b>Transaction ID:</b> %10")
                           .arg(query.value("id").toString())
                           .arg(query.value("name").toString())
                           .arg(query.value("dob").toString())
                           .arg(query.value("doctor").toString())
                           .arg(query.value("blood").toString())
                           .arg(query.value("date_admitted").toString())
                           .arg(query.value("date_released").toString())
                           .arg(query.value("amount").toString())
                           .arg(query.value("payment").toString())
                           // CORRECTED: Use the quoted name when retrieving the value.
                           .arg(query.value("transaction").toString()); // <-- FIX HERE
        ui->result_label->setText(info);
    } else {
        ui->result_label->setText("<font color='red'><b>No patient found with this ID.</b></font>");
    }
}

void Window::on_download_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save CSV", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file for writing.");
        return;
    }

    QTextStream out(&file);
    out << "ID,Name,DOB,Doctor,Blood Group,Date Admitted,Date Released,Amount,Payment,Transaction ID\n";

    QSqlQuery query("SELECT * FROM patients");
    while (query.next()) {
        QStringList fields;
        for (int i = 0; i < 10; ++i)
            fields << "\"" + query.value(i).toString().replace("\"", "\"\"") + "\"";
        out << fields.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "Saved", "Patient data exported successfully.");
}
