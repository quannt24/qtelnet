#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qdebug.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tracker = NULL;

    // Connect signals and slots
    connect(ui->pbConnect, SIGNAL(clicked(bool)), this, SLOT(onConnectClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnectClicked()
{
    qDebug() << "Connect clicked";

    if (!tracker) {
        qDebug() << "Create tracker";
        tracker = new qtelnet;
    } else {
        qDebug() << "Remove tracker";
        delete tracker;
        tracker = NULL;
    }
}
