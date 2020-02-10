#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QtSql>
#include "ui_mainwindow.h"
#include "connectdialog.h"
#include "waitingspinnerwidget.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow, Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    //QSqlDatabase *db;
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    QSqlTableModel *model;
    Ui::MainWindow *ui;
    QString getColtext() {return comboBox_column->currentText();}
    QString getLetext() {return lineEdit->text();}

private slots:
    void on_comboBox_Table_currentIndexChanged(const QString &table);
    void search();
    void on_pbSubmit_clicked();

private:
    QTableWidget *tw;
    int searching;
    static void query();
};

class Worker : public QObject
{
    Q_OBJECT
public:
    Worker(MainWindow *windowpointer): QObject()
    {
        wp = windowpointer;
    }

    ~Worker(){};
    MainWindow *wp;
public slots:
    void process();

signals:
    void finished();
    void error(QString err);
    void alldone();

};

#endif // MAINWINDOW_H
