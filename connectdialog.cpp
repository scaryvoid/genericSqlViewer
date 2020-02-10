#include "connectdialog.h"
#include "ui_connectdialog.h"
#include <QSettings>
#include <QFile>

ConnectDialog::ConnectDialog(QWidget *parent) :
    QDialog(parent, Qt::Window)
{
    setupUi(this);
    lineEditPw->setEchoMode(QLineEdit::Password);

    // import last settings if found
    QFile config(QCoreApplication::applicationDirPath() + "/conf.ini");
    if (config.exists())
    {
        QSettings settings(QCoreApplication::applicationDirPath() + "/conf.ini", QSettings::IniFormat);
        QString host = settings.value("host").toString();
        QString dbname = settings.value("db").toString();
        QString user = settings.value("user").toString();
        lineEditHost->setText(host);
        lineEditUser->setText(user);
        lineEditDatabase->setText(dbname);
        lineEditPw->setFocus();
    }
}

ConnectDialog::~ConnectDialog()
{
}

void ConnectDialog::on_buttonBox_accepted()
{

}

void ConnectDialog::on_buttonBox_rejected()
{

}
