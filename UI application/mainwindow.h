#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTime>
#include <QObject>
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
    void on_Start_clicked();
public slots:
    void CitireServer();
    void SchimbareStrada();
    void InfoSport();
    void InfoVreme();
    void Viteza();
private:
    Ui::MainWindow *ui;
    QTimer *timerCitire,*timerSport,*timerVreme,*timerViteza,*timerStrada;
};

#endif // MAINWINDOW_H
