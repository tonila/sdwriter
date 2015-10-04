#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QTime>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_fileSelector_clicked();
    void on_writeButton_clicked();
    void onProgress();
    void onTimeout();
    void readFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:

    Ui::MainWindow *ui;
    QProcess *m_reader;
    QTime m_time;
    QTimer *m_timer;
};

#endif // MAINWINDOW_H
