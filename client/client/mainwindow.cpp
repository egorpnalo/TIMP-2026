#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "client.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(Client::getInstance(), &Client::msgFromServer,
            this, &MainWindow::onServerResponse);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onServerResponse(const QString &response)
{
    ui->resultLabel->setText(response);
}
