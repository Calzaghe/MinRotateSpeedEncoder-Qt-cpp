#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QtSerialPort>
#include <QSerialPortInfo>
#include <QVector>

#include "nowypomiar.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


signals:
    void gotoweWartosci(QString,QString);
    void otwartoOkno();
    void sygnalDoSredniej();

private slots:
    void on_odswiez_button_clicked();
    void on_polaczPort_button_clicked();
    void on_czyscLogi_button_clicked();
    void on_nowyPomiar_button_clicked();

    void odbiorDanychzPortu();
    void obliczanieSredniej();

    void on_rozlaczPort_button_clicked();


    void oknoZamkniete(ulong);
    void odbiorDanych(ulong,double);



private:
    Ui::MainWindow *ui;

    QSerialPort *port;
    QList<QSerialPortInfo>dostepnePorty;

    std::vector<NowyPomiar*> pomiary;
    std::vector<NowyPomiar*> okna;
    std::vector<QString> dane;
    std::vector<double> wektorSredniej;

    double sredniaStatystyczna=0;

};
#endif // MAINWINDOW_H
