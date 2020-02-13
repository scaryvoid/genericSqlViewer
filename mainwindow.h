#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QtSql>
#include "ui_mainwindow.h"
#include "connectdialog.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow, Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString hostname, databasename, username, password;
    QSqlTableModel *model;
    Ui::MainWindow *ui;
    int maxRows;
    void refresh(QString table, QString filter);
    QList<QTableWidgetItem*> editList;
    QMap<QString, int> mapKeys;

private slots:
    void on_comboBox_Table_currentIndexChanged(QString table);
    void search();
    void on_pbSubmit_clicked();
    void addToEditList();

private:
    QTableWidget *tw;
    int searching;
    static void query();
};

#endif // MAINWINDOW_H
