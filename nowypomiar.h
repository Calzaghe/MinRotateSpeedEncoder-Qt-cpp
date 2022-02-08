#ifndef NOWYPOMIAR_H
#define NOWYPOMIAR_H

#include <QWidget>
#include <QtMath>
#include <QDebug>
#include <QTimer>
#include <QVector>
#include <QList>
#include <QLineEdit>


namespace Ui {
class NowyPomiar;
}

class NowyPomiar : public QWidget
{
    Q_OBJECT

public:
    explicit NowyPomiar(QWidget *parent = nullptr);
    ~NowyPomiar();

private slots:
    void odbiorDanych(QString,QString);
    void check();

    void on_kalibruj_button_clicked();
    void on_startNagrywania_button_clicked();
    void on_stopNagrywania_button_clicked();

    void timerStop();
    void updateLCD();

    void on_zapiszPrzebieg_button_clicked();
    void on_zapiszObraz_button_clicked();

    void on_wyslijPomiar_button_clicked();

public slots:

    void sprawdzKalibracje();

signals:

    void zamknieto(ulong);
    void daneStatystyka(ulong,double);


private:
    Ui::NowyPomiar *ui;

    int iloscProbek=0;

    int czasPomiaruSec=0;

    double czasMiedzyWartosciami;

    double dlugoscLufy=0;
    double wysuniecieSznura;
    double odlegloscEnkoderSrodekDziala;

    double offset=0;
    double wysuniecieSznuraOFFSET=0;

    double RadianyObrotDziala;

    double RadianyObrotDziala_WIRTUALNE_ZERO;
    double RadianyLufa;
    double RadianyEnkoder;

    double PredkoscKatowaObrotuDziala=0;
    double PredkoscKatowaMin=30000;

    std::vector<double>wektorKatow;
    std::vector<double>wektorPomiarow;

    bool startObliczen=false;
    bool offsetPrzekroczony=false;
    bool kalibracja = false;

    QTimer *timeLoop;

    void Obliczenia();

};


#endif // NOWYPOMIAR_H
