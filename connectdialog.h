#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>
#include "ui_connectdialog.h"

//namespace Ui {
//class ConnectDialog;
//}

class ConnectDialog : public QDialog, public Ui::ConnectDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(QWidget *parent = 0);
    ~ConnectDialog();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    //Ui::ConnectDialog *ui;
};

#endif // CONNECTDIALOG_H
