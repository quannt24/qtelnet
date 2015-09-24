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

void MainWindow::onDataRecv(const char *data, int size, void *bundle)
{
    char buf[size + 1];
    strncpy(buf, data, size);
    buf[size] = '\0';

    QLabel *console = (QLabel*) bundle;
    console->setText(console->text().append(buf));
}

void MainWindow::onConnectClicked()
{
    int r;

    ui->pbConnect->setEnabled(false); // Temporary disable button

    if (!tracker) {
        qDebug() << "Connecting";
        tracker = new qtelnet; // Create new tracker for new connection
        tracker->set_data_recv_callback(&(MainWindow::onDataRecv));
        tracker->set_data_recv_bundle((void*) ui->lbConsole);

        r = qtelnet::telnet_connect(*tracker, "127.0.0.1", "23");
        if (r == 0) {
            // Connection success
            ui->pbConnect->setText("Disconnect");
        } else {
            qtelnet::telnet_disconnect(*tracker);
            delete tracker;
            tracker = NULL;
        }
    } else {
        qDebug() << "Disconnect";
        qtelnet::telnet_disconnect(*tracker);
        delete tracker;
        tracker = NULL;
        ui->pbConnect->setText("Connect");
    }

    ui->pbConnect->setEnabled(true);
}
