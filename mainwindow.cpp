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
    maxRows = 50000;

    // write settings to config file
    QSettings settings(QCoreApplication::applicationDirPath() + "/conf.ini", QSettings::IniFormat);
    settings.setValue("host", dialog->lineEditHost->text());
    settings.setValue("db", dialog->lineEditDatabase->text());
    settings.setValue("user", dialog->lineEditUser->text());

    QSqlDatabase *db = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL"));
    db->setHostName(dialog->lineEditHost->text());
    db->setDatabaseName(dialog->lineEditDatabase->text());
    db->setUserName(dialog->lineEditUser->text());
    db->setPassword(dialog->lineEditPw->text());

    // bail if no connection
    if(!db->open())
    {
        QMessageBox::warning(this, "Error", "Unable to connect to database:" + db->lastError().text());
        exit(1);
    }

    // populate combobox table
    QSqlQuery queryTables("show tables");
    while(queryTables.next())
        comboBox_Table->addItem(queryTables.value(0).toString());

    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(pbSearch, SIGNAL(clicked()), this, SLOT(search()));
    connect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(addToEditList()));

    refresh("", "");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refresh(QString table, QString filter)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QSqlQuery contentQuery;
    QSqlQuery nRowsQuery;
    int tableRows;
    editList.clear();
    mapKeys.clear();
    disconnect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(addToEditList()));

    // if no table, pick one
    if(table.isEmpty())
        table = comboBox_Table->currentText();

    // get columns
    QSqlQuery nColumnQuery(QString("select count(*) from information_schema.columns where table_name = '%1'").arg(table));
    nColumnQuery.next();
    int nColumns = nColumnQuery.value(0).toInt();
    tableWidget->setColumnCount(nColumns);

    // populate column combo box
    QSqlQuery queryColumns(QString("desc %1").arg(table));
    while(queryColumns.next())
        comboBox_column->addItem(queryColumns.value(0).toString());

    // fill out header list
    QStringList headerList;
    QSqlQuery headerQuery(QString("select column_name from information_schema.columns where table_name='%1'").arg(table));
    for(int header=0;header<nColumns;header++)
    {
        headerQuery.next();
        headerList.append(headerQuery.value(0).toString());
    }
    tableWidget->setHorizontalHeaderLabels(headerList);

    // get primary key
    QSqlQuery primaryKey(QString("select column_name from information_schema.key_column_usage where constraint_name='PRIMARY' and table_name='%1'").arg(table));
    while(primaryKey.next())
        mapKeys.insert(primaryKey.value(0).toString(), headerList.indexOf(primaryKey.value(0).toString()));

    // get row count
    if(filter.isEmpty())
        nRowsQuery = QSqlQuery(QString("select count(*) from %1").arg(table));
    else
        nRowsQuery = QSqlQuery(QString("select count(*) %1").arg(filter));

    // build query/set table up for nRows
    nRowsQuery.next();
    int nRows = nRowsQuery.value(0).toInt();
    QList<QString> listKeys = mapKeys.keys();
    if(nRows > maxRows)
    {
        if(filter.isEmpty())
            contentQuery = QSqlQuery(QString("select * from %1 order by %2 limit %3").arg(table).arg(listKeys.join(',')).arg(maxRows));
        else
            contentQuery = QSqlQuery(QString("select * %1 order by %2 limit %3").arg(filter).arg(listKeys.join(',')).arg(maxRows));
        sbWindow->showMessage(QString("Rows Limited"));
        tableWidget->setRowCount(maxRows);
        tableRows = maxRows;
    }
    else
    {
        if(filter.isEmpty())
            contentQuery = QSqlQuery(QString("select * from %1 order by %2").arg(table).arg(listKeys.join(',')));
        else
            contentQuery = QSqlQuery(QString("select * %1 order by %2").arg(filter).arg(listKeys.join(',')));
        tableWidget->setRowCount(nRows);
        tableRows = nRows;
        sbWindow->clearMessage();
    }

    // fill out cells
    for(int row=0;row<tableRows;row++)
    {
        contentQuery.next();
        for(int col=0;col<nColumns;col++)
            tableWidget->setItem(row,col,new QTableWidgetItem(contentQuery.value(col).toString()));
    }
    tableWidget->resizeColumnsToContents();
    QApplication::restoreOverrideCursor();
    connect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(addToEditList()));
}

void MainWindow::on_comboBox_Table_currentIndexChanged(QString table)
{
    refresh(table, "");
}

void MainWindow::search()
{
    QString filter = QString("from %1 where %2 like '%").arg(comboBox_Table->currentText()).arg(comboBox_column->currentText());
    filter = filter + lineEdit->text() + "%'";
    refresh(comboBox_Table->currentText(), filter);
}

void MainWindow::on_pbSubmit_clicked()
{
    for(int i=0;i<editList.length();i++)
    {
        QTableWidgetItem *item = editList[i];
        QString colName = tableWidget->horizontalHeaderItem(item->column())->text();
        QString primarykeyValue = tableWidget->itemAt(mapKeys.value(colName), item->row())->text();
        QString queryStr = QString("update %1 set %2='%3' where %4='%5'").arg(comboBox_Table->currentText()).arg(colName).arg(item->text()).arg(mapKeys.key(0)).arg(primarykeyValue);
        QSqlQuery query;
        query.exec(queryStr);
    }
}

void MainWindow::addToEditList()
{
    editList.append(tableWidget->currentItem());
}
