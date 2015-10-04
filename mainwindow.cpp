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
            if (parts[i].contains("usb")) {
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

void MainWindow::on_fileSelector_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select file"), QDir::homePath());
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

void MainWindow::readFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << exitCode;
    if (exitStatus == QProcess::CrashExit) {
        ui->textBrowser->setPlainText("----SSD WRITE FAILED----");
    } else {
        ui->textBrowser->setPlainText("----SSD WRITE SUCCESSFULL----");
    }
    ui->progressBar->setValue(0);
    ui->progressBar->setRange(0, 2147484);
    ui->writeButton->setEnabled(true);
}

void MainWindow::on_writeButton_clicked()
{
    ui->writeButton->setDisabled(true);
    QString dev = ui->comboBox->currentText();
    QString filename = ui->filenameEdit->text();
    qDebug() << dev;
    qDebug() << filename;

    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);

    QDir dir;
    qDebug() << dir.absolutePath();

    m_reader = new QProcess(this);

    connect(m_reader, SIGNAL(readyReadStandardError()), this, SLOT(onProgress()));
    connect(m_reader, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(readFinished(int, QProcess::ExitStatus)));

    m_reader->start(dir.absolutePath() + "/write.sh", QStringList() << filename << "2G" << dev );

    // Wait for process to start
    if(!m_reader->waitForStarted()) {
        qDebug() << "ERROR: could not start";
        return;
    }



}
