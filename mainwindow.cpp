#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

#include "qdebug.h"

#include "launcher.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    Launcher* l = new Launcher();
    QString s = l->launch("ls -lbH  /dev/disk/by-id/");

    QStringList devs;
    const QStringList list = s.split("\n");
    for (int var = 0; var < list.count(); ++var) {
        QString it = list[var];
        const QStringList parts = it.split(" ");
        int len = parts.length();
        if (len >= 11) {
            int i = len-3;
            if (parts[i].contains("usb") || parts[i].contains("mmc")) {
                QStringList p2 = parts[parts.count() - 1].split("/");
                QString d = "/dev/" + p2[p2.count() - 1];
                if (!parts[i].contains("part")) {
                    devs <<  d;
                    qDebug() << parts[i] << d;
                } else {
                    // unmount device nodes, if automounted
                    l->launch("umount " + d);
                }
            }
        }
    }

    QString lsblk = l->launch("lsblk");

    ui->setupUi(this);
    ui->textBrowser->setPlainText(lsblk);
    ui->comboBox->addItems(devs);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_fileSelector_clicked() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Select file"), QDir::homePath(), tr("7z Files (*.7z)"));
    ui->filenameEdit->setText(filename);
}

void MainWindow::onProgress() {
    QByteArray bytes = m_reader->readAllStandardError();
    bool ok;
    int value = bytes.trimmed().toInt(&ok);
    if (!ok) {
        qDebug() << bytes.trimmed();
        ui->textBrowser->append(bytes);
        return;
    }
    if (value >= 100) {
        value = 0;
        ui->progressBar->setRange(0, 0);
    }
    ui->progressBar->setValue(value);
}

void MainWindow::readFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qDebug() << exitCode;
    if (exitStatus == QProcess::CrashExit) {
        ui->textBrowser->append("\n\n----SSD WRITE FAILED----");
    } else {
        ui->textBrowser->append("\n\n----SSD WRITE SUCCESSFULL----");
    }
    ui->progressBar->setValue(0);
    ui->progressBar->setRange(0, 2147484);
    ui->writeButton->setEnabled(true);
    m_timer->stop();
}

void MainWindow::onTimeout() {
    m_time = m_time.addSecs(1);
    ui->labelTime->setText(m_time.toString("hh:mm:ss"));
}

void MainWindow::on_writeButton_clicked() {
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->start(1000);

    ui->writeButton->setDisabled(true);
    m_time = QTime(0,0);
    ui->labelTime->setText("00:00:00");
    ui->textBrowser->setPlainText("");

    QString dev = ui->comboBox->currentText();
    QString filename = ui->filenameEdit->text();
    qDebug() << dev;
    qDebug() << filename;

    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);

    m_reader = new QProcess(this);

    connect(m_reader, SIGNAL(readyReadStandardError()), this, SLOT(onProgress()));
    connect(m_reader, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(readFinished(int, QProcess::ExitStatus)));

    m_reader->start("bash", QStringList() << "-c" << "7zr x -so " + filename + " | pv -ns 2G | dd of=" + dev + " bs=512 conv=noerror,sync");

    // Wait for process to start
    if(!m_reader->waitForStarted()) {
        qDebug() << "ERROR: could not start";
        return;
    }



}
