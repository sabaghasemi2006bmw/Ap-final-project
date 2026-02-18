#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "databasemanager.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void setupTables();
    void populateTables();
    void refreshDiscountTable();;
    ~MainWindow();

private slots:
    void on_Approve_button_clicked();

    void on_Reject_button_clicked();

    void on_Reject_button_2_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    DatabaseManager *db;
};
#endif // MAINWINDOW_H
