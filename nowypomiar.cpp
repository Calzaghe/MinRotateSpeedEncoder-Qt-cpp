#include "nowypomiar.h"
#include "ui_nowypomiar.h"

NowyPomiar::NowyPomiar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NowyPomiar)
{
    ui->setupUi(this);

    connect(ui->offset_textbox,SIGNAL(textEdited(QString)),this,SLOT(sprawdzKalibracje()));
    connect(ui->dlugoscLufy_textbox,SIGNAL(textEdited(QString)),this,SLOT(sprawdzKalibracje()));
    connect(ui->czasbadania_textbox,SIGNAL(textEdited(QString)),this,SLOT(sprawdzKalibracje()));

    qDebug()<<"Otwarto \t"<<this<<" ID OKNA: "<<QString::number(this->winId()); //////////////////
    this->setAttribute(Qt::WA_DeleteOnClose); ////////////////////////
    this->setWindowTitle("Okno pomiarowe ID " + QString::number(this->winId()));//////////////////


    ui->wykres->setBackground(Qt::darkGray);
    ui->wykres->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);


    //wszystko zeby okienko wychylenie bylo przezroczyste, nic waznego
    QPalette palette = ui->wychylenie_textbox->palette();
    palette.setBrush(QPalette::Base, Qt::transparent);
    ui->wychylenie_textbox->setPalette(palette);
    ui->wychylenie_textbox->setAttribute(Qt::WA_OpaquePaintEvent, false);


    //wszystko zeby okienko min predkosc bylo przezroczyste, nic waznego
    QPalette palette1 = ui->minPredkosc_textbox->palette();
    palette1.setBrush(QPalette::Base, Qt::transparent);
    ui->minPredkosc_textbox->setPalette(palette1);
    ui->minPredkosc_textbox->setAttribute(Qt::WA_OpaquePaintEvent, false);


    //wszystko zeby okienko wysuniecie linki bylo przezroczyste, nic waznego
    QPalette palette2 = ui->wysuniecieLinki_textbox->palette();
    palette2.setBrush(QPalette::Base, Qt::transparent);
    ui->wysuniecieLinki_textbox->setPalette(palette2);
    ui->wysuniecieLinki_textbox->setAttribute(Qt::WA_OpaquePaintEvent, false);


    //wzystko zeby okeinko czas pomiaru bylo przezroczyste, nic waznego
    QPalette palette3 = ui->czasPomiaruLicznik->palette();
    palette3.setBrush(QPalette::Base, Qt::transparent);
    ui->czasPomiaruLicznik->setPalette(palette3);
    ui->czasPomiaruLicznik->setAttribute(Qt::WA_OpaquePaintEvent, false);

    //konfiguracja wykresu
    ui->wykres->addGraph();

    ui->wykres->xAxis->setLabel("Próbki");
    ui->wykres->yAxis->setLabel("Rad/s");

    ui->wykres->xAxis->setRange(0, 500);
    ui->wykres->yAxis->setRange(-1, 1);


    ui->startNagrywania_button->setStyleSheet("background-color:lightGreen");
    ui->stopNagrywania_button->setStyleSheet("background-color:red");
    ui->zapiszPrzebieg_button->setStyleSheet("background-color:lightBlue");
    ui->kalibruj_button->setStyleSheet("background-color:lightBlue");
    ui->zapiszObraz_button->setStyleSheet("background-color:lightBlue");


    timeLoop = new QTimer(this);


    ui->dlugoscLufy_textbox->setValidator(new QIntValidator(0,100000000,this));
    //ui->offset_textbox->setValidator(new QIntValidator(0,100000000,this));
    ui->czasbadania_textbox->setValidator(new QIntValidator(0,100000000,this));

    ui->wyslijPomiar_button->setEnabled(false);
    ui->startNagrywania_button->setEnabled(false);
    ui->zapiszObraz_button->setEnabled(false);
    ui->zapiszPrzebieg_button->setEnabled(false);
    ui->stopNagrywania_button->setEnabled(false);

    ui->kalibruj_button->setEnabled(false);


}

NowyPomiar::~NowyPomiar()
{
    qDebug()<<"Zamknieto \t"<<this<<" ID OKNA: "<<QString::number(this->winId()); ////////
    emit zamknieto(this->winId()); /////////////

    delete ui;

}


//button kalibracji aktywny tylko jak wszsytkie 3 pola sa wypelnione
void NowyPomiar::sprawdzKalibracje()
{

    if(!ui->offset_textbox->text().isEmpty()&!ui->dlugoscLufy_textbox->text().isEmpty()&!ui->czasbadania_textbox->text().isEmpty()){
        ui->kalibruj_button->setEnabled(true);
    }

}

//slot odebrania danych z mainwindow z wysunieciem sznura - triggeruje sie za kazdym razem jak otrzyma nowa wartosc wysuniecia sznura - co za tym idzie triggeruje wykonanie obliczen
void NowyPomiar::odbiorDanych(QString dlugoscLinki, QString czasRamek)
{
    //przypisanie wartosci
    wysuniecieSznura=dlugoscLinki.toDouble();
    czasMiedzyWartosciami = czasRamek.toDouble()/1000;

    //wyswietlenie wysuniecia sznura
    ui->wysuniecieLinki_textbox->setPlainText(QString::number(wysuniecieSznura));


    //warunek ze kalibracja ,msu byc wykonana
    if(kalibracja)
    {

        //OFFSET - obliczenia odpalaja sie dopiero jak lina wysunie sie o abs offsetu od momentuy kalibracji

        //wykonanie obliczen
        if(startObliczen&&offsetPrzekroczony) Obliczenia();
    }
}

//button kalibracji, ustawia wartosci katow dla ktorych kąt linki enkodera z lufa maja 90 stopni, dlugosc lufy itp.
void NowyPomiar::on_kalibruj_button_clicked()
{

    ui->wyslijPomiar_button->setEnabled(false);
    ui->startNagrywania_button->setEnabled(true);
    ui->zapiszObraz_button->setEnabled(false);
    ui->zapiszPrzebieg_button->setEnabled(false);
    ui->stopNagrywania_button->setEnabled(false);

    offsetPrzekroczony=false;

    //dlugosc lufy pobrana z textbox podawana w mm
    dlugoscLufy=ui->dlugoscLufy_textbox->text().toDouble();

    //TODO: wstepnie odffest zrobiony
    offset=ui->offset_textbox->text().toDouble();

    wysuniecieSznuraOFFSET = wysuniecieSznura;

    //z pitagorasa, odleglosc enkodera od srodka działka W MOMENCIE OBLICZANIA ZAKLADAMY ZE LINKA-LUFA MA 90 STOPNI (poprostu kalibracja)
    odlegloscEnkoderSrodekDziala = qSqrt(pow(dlugoscLufy,2)+pow(wysuniecieSznura,2));


    //OBLICZANIE WARTOSCI KATOW
    //zapisanie wartosci, ktora reprezentuje kalibracyjne zero dla wartosci kata obrotu działka w momencie konfiguracji (90 stopni linka=lufa)
    RadianyObrotDziala_WIRTUALNE_ZERO = acos((pow(odlegloscEnkoderSrodekDziala, 2)+pow(dlugoscLufy, 2)-pow(wysuniecieSznura, 2))/(2*odlegloscEnkoderSrodekDziala*dlugoscLufy));

    // PONIZEJ TE WARTOSCI KĄTÓW NICZEMU NIE SLUZA DO OBLICZEN, POPROSTU FAJNIE WYGLADAJA JAK KLIKNIESZ KALIBRACJA:

    //wartosc kata przy lufie
    RadianyLufa =acos((pow(wysuniecieSznura, 2)+pow(dlugoscLufy, 2)-pow(odlegloscEnkoderSrodekDziala, 2))/(2*wysuniecieSznura*dlugoscLufy));

    //wartosc kata przy enkoderze
    RadianyEnkoder =acos((pow(wysuniecieSznura, 2)+pow(odlegloscEnkoderSrodekDziala, 2)-pow(dlugoscLufy, 2))/(2*wysuniecieSznura*odlegloscEnkoderSrodekDziala));


    ui->info_textbox->setPlainText("KALIBRACJA");
    ui->info_textbox->append("==================================");
    ui->info_textbox->append("Enkoder-Srodek Działa: \t"+QString::number(odlegloscEnkoderSrodekDziala)+" mm");
    ui->info_textbox->append("Wysuniecie linki: \t"+QString::number(wysuniecieSznura)+" mm");
    ui->info_textbox->append("Wartość Offsetu: \t"+QString::number(offset)+" mm");
    ui->info_textbox->append("Kąt przy działku: \t"+QString::number(RadianyObrotDziala_WIRTUALNE_ZERO*180/M_PI)+" °");
    ui->info_textbox->append("Kąt przy wylocie lufy: \t"+QString::number(RadianyLufa*180/M_PI)+" °");
    ui->info_textbox->append("Kąt przy enkoderze: \t"+QString::number(RadianyEnkoder*180/M_PI)+" °");
    ui->info_textbox->append("==================================");


    // wyswietlenie 0.00 tekst w momencie kalibracji
    ui->wychylenie_textbox->setPlainText("0.00 °");

    if(ui->czasbadania_textbox->text()=="2137"){
        ui->info_textbox->setPlainText("GODZINA PAPIEŻOWA");
    }

    kalibracja = true;
}

//triggerowanan funkcja za kazdym razem jak jest pomier, tutaj jest cala magia szuru buru
void NowyPomiar::Obliczenia()
{

    //N A J W A Z N I E J S Z E
    //obliczanie wartosci kata przy srodku dzialka na bierzaco z 3 dlugosci bokow
    RadianyObrotDziala =RadianyObrotDziala_WIRTUALNE_ZERO - acos((pow(odlegloscEnkoderSrodekDziala, 2)+pow(dlugoscLufy, 2)-pow(wysuniecieSznura, 2))/(2*odlegloscEnkoderSrodekDziala*dlugoscLufy));

    //obrotowa kulka XD
    ui->wirtualnaLufa->setValue(90+RadianyObrotDziala*180/M_PI);

    //w wektorze musza byc 2 wartosci kata, aby obliczyc predkosc katowa
    //dlatego jezeli nie ma nic albo 1 wartosc to nastepuje wstrzykniecie nowej wartosci zeby byly dwie
    if(wektorKatow.size()==0||wektorKatow.size()==1) wektorKatow.push_back(RadianyObrotDziala);

    //jak sa 2 wartosci kata, obliczamy predkosc katawa z delta zmiany kata przez czas
    if(wektorKatow.size()==2)
    {
        //(nowa wartosc - stara wartosc) / czas miedzy pomiarami
        PredkoscKatowaObrotuDziala = (wektorKatow[1]-wektorKatow[0])/czasMiedzyWartosciami;

        //usuniecie starej wartosci kata w wektorze, robienie miejsca na nowwa wartosc
        wektorKatow.erase(wektorKatow.begin());
    }

    //MINIMAJLNA WARTOSC
    //minimalna wartosc abs nowej wartosci mniejsza niz abs obecnej to wyswietla
    if(abs(PredkoscKatowaMin)>abs(PredkoscKatowaObrotuDziala)&&PredkoscKatowaObrotuDziala!=0)
    {
        PredkoscKatowaMin=PredkoscKatowaObrotuDziala;
        ui->minPredkosc_textbox->setPlainText(QString::number(abs(PredkoscKatowaMin*1000)));
    }


    //wychylenie katowe - napis w kulce
    ui->wychylenie_textbox->setPlainText(QString::number(RadianyObrotDziala*180/M_PI,'f',2)+" °");
    //dodanie do wykresu pomiaru
    ui->wykres->graph(0)->addData(iloscProbek,PredkoscKatowaObrotuDziala*-1);
    //mnozone przez -1 bo wtedy osie wykresu sa cycuś pizdeczka delikatesik #januszmode
    wektorPomiarow.push_back(PredkoscKatowaObrotuDziala);
    ui->wykres->rescaleAxes();
    ui->wykres->replot();

    iloscProbek++;
}

void NowyPomiar::on_startNagrywania_button_clicked()
{
    ui->stopNagrywania_button->setEnabled(true);

    //reset wartosic - nowy pomiar
    iloscProbek=0;
    PredkoscKatowaMin=30000;
    wektorPomiarow.clear();
    wektorKatow.clear();
    ui->wykres->graph(0)->data()->clear();

    //doapsowanie rozdzielczosci wykresu do czasu pomiaru
    ui->wykres->xAxis->setRange(0, (ui->czasbadania_textbox->text().toDouble())/czasMiedzyWartosciami);
    ui->wykres->replot();


    if(timeLoop->isActive())
    {
         timeLoop->stop();
    }

    connect(timeLoop,SIGNAL(timeout()),this,SLOT(check()));

    timeLoop->setInterval(czasMiedzyWartosciami);
    timeLoop->start();
}

//czekanie na offset, jak offset przekroczony start czasu pomiaru i dane na wykres
void NowyPomiar::check()
{
    if(abs(wysuniecieSznura-(wysuniecieSznuraOFFSET+offset))>offset)
    {

        timeLoop->stop();
        disconnect(timeLoop,SIGNAL(timeout()),this,SLOT(check()));

        startObliczen=true;
        offsetPrzekroczony=true;

        timeLoop->setInterval(1000);
        connect(timeLoop,SIGNAL(timeout()),this,SLOT(updateLCD()));

        czasPomiaruSec=ui->czasbadania_textbox->text().toInt();
        ui->czasPomiaruLicznik->setPlainText(QString::number(czasPomiaruSec));//display(czasPomiaruSec);
        czasPomiaruSec--;

        timeLoop->start();
        QTimer::singleShot((ui->czasbadania_textbox->text().toInt()*1000), this, SLOT(timerStop()));
    }
}

void NowyPomiar::on_stopNagrywania_button_clicked()
{

    if(startObliczen==true)
    {
        startObliczen=false;
        ui->info_textbox->append("ZAKONCZONO POMIAR");

        ui->wyslijPomiar_button->setEnabled(true);
        ui->startNagrywania_button->setEnabled(false);
        ui->zapiszObraz_button->setEnabled(true);
        ui->zapiszPrzebieg_button->setEnabled(true);
        ui->stopNagrywania_button->setEnabled(false);
    }
    else
    {
        ui->info_textbox->append("BRAK ROZPOCZĘTEGO POMIARU");
    }
}

void NowyPomiar::timerStop()
{

    if(timeLoop->isActive())
    {
         timeLoop->stop();
    }
    ui->czasPomiaruLicznik->setPlainText(QString::number(0));
    disconnect(timeLoop,SIGNAL(timeout()),this,SLOT(updateLCD()));


    startObliczen=false;
    ui->info_textbox->append("ZAKONCZONO POMIAR");
    ui->wyslijPomiar_button->setEnabled(true);
    ui->startNagrywania_button->setEnabled(false);
    ui->zapiszObraz_button->setEnabled(true);
    ui->zapiszPrzebieg_button->setEnabled(true);
    ui->stopNagrywania_button->setEnabled(false);
}

void NowyPomiar::updateLCD()
{

    //TODO jak pojdzie kilka razy pomiar w tym smym oknie to pierwsze sekundy 2 i kolejnego pomiaru są czerwone nei czarne idk czemu, elseif tez tak samo
    if(czasPomiaruSec<=5){
        ui->czasPomiaruLicznik->setTextColor(Qt::red);
    }
    else{
        ui->czasPomiaruLicznik->setTextColor(Qt::black);
    }
    ui->czasPomiaruLicznik->setPlainText(QString::number(czasPomiaruSec));
    czasPomiaruSec--;

}

//zapisanie danych z wykresu w formie txt
void NowyPomiar::on_zapiszPrzebieg_button_clicked()
{
    if(wektorPomiarow.size()>0)
    {
        QString lokalizacja = QFileDialog::getSaveFileName(nullptr, "Okno Zapisu Przebiegu", ".", "Pliki Tekstowe ;) (*.txt)" );
        QFile f(lokalizacja);

        if (f.open( QIODevice::WriteOnly))
        {
                QTextStream plik(&f);

                plik<<"ZAPISANY PRZEBIEG\n";
                plik<<"MIN PREDKOSC: \t" + ui->minPredkosc_textbox->toPlainText()+" mrad/s\n";
                for(int i=0;i<(int)wektorPomiarow.size();i++)
                {
                    plik<<QString::number(i)+"\t";
                    plik<<QString::number(wektorPomiarow[i],'f',4)+"\n";
                }
         }
        f.close();
    }
}

//zapisywanie wykresu jako obrazu w formacie png
void NowyPomiar::on_zapiszObraz_button_clicked()
{
 QString lokalizacja = QFileDialog::getSaveFileName(nullptr, "Zapisz przebieg jako grafikę", " ");
 QFile f(lokalizacja+".png");


    if(!f.open(QIODevice::WriteOnly))
        {
            qDebug()<<f.errorString();
        }
    else
        {
        ui->wykres->savePng(lokalizacja+".png");
        }

}

void NowyPomiar::on_wyslijPomiar_button_clicked()
{
    double wartoscPomiaru = ui->minPredkosc_textbox->toPlainText().toDouble();
    emit daneStatystyka(this->winId(), wartoscPomiaru);
}


