#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    ConnectDialog *dialog = new ConnectDialog;
    dialog->exec();

    // write settings to config file
    QSettings settings(QCoreApplication::applicationDirPath() + "/conf.ini", QSettings::IniFormat);
    settings.setValue("host", dialog->lineEditHost->text());
    settings.setValue("db", dialog->lineEditDatabase->text());
    settings.setValue("user", dialog->lineEditUser->text());

    db.setHostName(dialog->lineEditHost->text());
    db.setDatabaseName(dialog->lineEditDatabase->text());
    db.setUserName(dialog->lineEditUser->text());
    db.setPassword(dialog->lineEditPw->text());

    // bail if no connection
    if(!db.open())
    {
        QMessageBox::warning(this, "Error", "Unable to connect to database:" + db.lastError().text());
        exit(1);
    }

    model = new QSqlTableModel(this, db);

    // populate combobox table
    QSqlQuery queryTables("show tables");
    int nTables = 0;
    while(queryTables.next())
    {
        comboBox_Table->addItem(queryTables.value(0).toString());
        nTables++;
    }

    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(pbSearch, SIGNAL(clicked()), this, SLOT(search()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_comboBox_Table_currentIndexChanged(const QString &table)
{
    model->setTable(table);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    QSqlQuery queryCount(QString("select count(*) from %1").arg(table));
    int rowCount = 0;
    while (queryCount.next())
        rowCount = queryCount.value(0).toInt();

    if(rowCount > 50000)
        sbWindow->showMessage(QString("Rows Limited"));
    else
        model->select();

    tableView->setModel(model);
    tableView->resizeColumnsToContents();
    tableView->show();

    // populate combobox_columns
    comboBox_column->clear();
    QSqlQuery queryColumns(QString("desc %1").arg(table));
    int nColumns = 0;
    while(queryColumns.next())
    {
        comboBox_column->addItem(queryColumns.value(0).toString());
        nColumns++;
    }
}

void Worker::process()
{
    QString column = wp->getColtext();
    QString letext = wp->getLetext();
    QString filter = column + " like '%" + letext + "%'";
    wp->model->setFilter(filter);
    wp->model->select();
    emit alldone();
}

void MainWindow::search()
{    
    sbWindow->clearMessage();
    WaitingSpinnerWidget* spinner = new WaitingSpinnerWidget(this);
    spinner->setRoundness(70.0);
    spinner->setMinimumTrailOpacity(15.0);
    spinner->setTrailFadePercentage(70.0);
    spinner->setNumberOfLines(12);
    spinner->setLineLength(10);
    spinner->setLineWidth(5);
    spinner->setInnerRadius(10);
    spinner->setRevolutionsPerSecond(1);
    spinner->setColor(QColor(81, 4, 71));
    spinner->start();
    //QThread *th = QThread::create(query);
    QThread *th = new QThread();
    Worker *worker = new Worker(this);
    worker->moveToThread(th);
    connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(th, SIGNAL(started()), worker, SLOT(process()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(th, SIGNAL(finished()), th, SLOT(deleteLater()));
    connect(worker, SIGNAL(alldone()), th, SLOT(quit()));
    th->start();

    while (th->isRunning())
    {
        qApp->processEvents();
        usleep(100000);
    }
    spinner->stop();
}

void MainWindow::on_pbSubmit_clicked()
{
    model->submitAll();
}
