#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui { class Window; }
QT_END_NAMESPACE

class Window : public QMainWindow {
    Q_OBJECT

public:
    Window(QWidget *parent = nullptr);
    ~Window();

private slots:
    void on_submit_clicked();
    void on_find_clicked();
    void on_download_clicked();

private:
    Ui::Window *ui;
    QSqlDatabase db;

    QString getFormattedDate(QComboBox *day, QComboBox *month, QComboBox *year);
};

#endif // WINDOW_H
