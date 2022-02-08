
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    port = new QSerialPort();

    //polaczenie sygnalu z portu ze gotowe sa dane - czytanie portu za pomaca slotu odbior danych z portu
    connect(port,SIGNAL(readyRead()),this,SLOT(odbiorDanychzPortu()));
    connect(this,SIGNAL(sygnalDoSredniej()),this,SLOT(obliczanieSredniej()));
}


MainWindow::~MainWindow()
{
    //usuwa wszystkie utworzone obiekty pamiarowe w wektorze obiektów 'pomiary' z pamieci
    foreach(NowyPomiar* a,pomiary)
    {
        //rozlaczenie sygnału z wartosciami
        disconnect(this,SIGNAL(gotoweWartosci(QString,QString)),a,SLOT(odbiorDanych(QString,QString)));

        delete a;
    }

    delete port;
    delete ui;
}


void MainWindow::on_odswiez_button_clicked()
{
    //wyczysc combobox
    ui->dostepnePorty_combobox->clear();

    //wylaczneie buttona "rozlacz" - bo klikniety
     ui->rozlaczPort_button->setEnabled(false);

     //wylaczneie buttona "nowy pomiar"
      ui->nowyPomiar_button->setEnabled(false);

    //zamyka port jak otwarty
    if(port->isOpen())
    {
        port->close();
    }

    dostepnePorty = QSerialPortInfo::availablePorts();

    //jak brak portow
    if(dostepnePorty.count() == 0)
    {
       qDebug()<<"Nie znaleziono urządzeń";
       ui->logi_textbox->append("Nie znaleziono urządzeń");

       //po co ma byc aktywne i generowac potencjalne errory jak nie ma z czym sie polaczyc
       ui->polaczPort_button->setEnabled(false);
    }

    //jak jakies sa
    else
    {
        //aktywacja buttona polacz
        ui->polaczPort_button->setEnabled(true);

        foreach(QSerialPortInfo a,dostepnePorty)
        {
            qDebug()<<"Znaleziono urządzenie: " + a.description();
            ui->logi_textbox->append("Znaleziono urządzenie: " + a.description());
            ui->dostepnePorty_combobox->addItem(a.portName()+ "\t" + a.description());
        }
    }
}


void MainWindow::on_rozlaczPort_button_clicked()
{
    //wlaczneie buttona "polacz"
     ui->polaczPort_button->setEnabled(true);

     //wylaczneie buttona "rozlacz" - bo klikniety
      ui->rozlaczPort_button->setEnabled(false);

      //wylaczneie buttona "nowy pomiar"
       ui->nowyPomiar_button->setEnabled(false);


     if(port->isOpen())
     {
         port->close();
         qDebug()<<"Zamknięto port";
         ui->logi_textbox->append("Zamknięto port");
     }
     else
     {
         qDebug()<<"Brak otwartego portu";
         ui->logi_textbox->append("Brak otwartego portu");
     }

}


void MainWindow::on_polaczPort_button_clicked()
{
    QString nazwaPortu = ui->dostepnePorty_combobox->currentText().split("\t").first();

    port->setPortName(nazwaPortu);
    port->setBaudRate(QSerialPort::Baud115200);
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);

    if(port->open(QSerialPort::ReadOnly))
    {
       qDebug()<<"Nawiązano połączenie z: " + nazwaPortu;
       ui->logi_textbox->append("Nawiązano połączenie z: " + nazwaPortu);

       //wlaczneie buttona "rozlacz"
        ui->rozlaczPort_button->setEnabled(true);

        //wlaczneie buttona "nowy pomiar"
         ui->nowyPomiar_button->setEnabled(true);

         //wylaczneie buttona "polacz" - bo juz polaczone
          ui->polaczPort_button->setEnabled(false);
    }

    else
    {
       qDebug()<<"Nie udało sie nawiązać połączenia z: " + nazwaPortu;
       ui->logi_textbox->append("Nie udało sią nawiązac połączenia z: " + nazwaPortu);

       //wylaczneie buttona "rozlacz" - jak nei ma sie z czym rozpaczac to po co ma byc aktywny
        ui->rozlaczPort_button->setEnabled(false);

        //wylaczneie buttona "nowy pomiar"
         ui->nowyPomiar_button->setEnabled(false);
    }
}


void MainWindow::on_czyscLogi_button_clicked()
{
    ui->logi_textbox->clear();
}


void MainWindow::on_nowyPomiar_button_clicked()
{

    okna.push_back(new NowyPomiar());
    connect(this,SIGNAL(gotoweWartosci(QString,QString)),okna[okna.size()-1],SLOT(odbiorDanych(QString,QString)));
    connect(okna[okna.size()-1], SIGNAL(zamknieto(ulong)), this, SLOT(oknoZamkniete(ulong)));
    connect(okna[okna.size()-1], SIGNAL(daneStatystyka(ulong,double)), this, SLOT(odbiorDanych(ulong,double)));
    okna[okna.size()-1]->show();

}

void MainWindow::oknoZamkniete(ulong index)
{
    int pozycja=0;
    int usuniete=0;

    foreach(NowyPomiar* a,okna)
    {
        if(a->winId()==index)
        {
            okna.erase(okna.begin()+pozycja);
            qDebug()<< "usunieto z wektora \t" << QString::number(a->winId());
            break;
        }
        pozycja++;
    }

    pozycja = 0;

    foreach(QString a, dane)
    {
        if(a.split("\t").first()==QString::number(index))
        {
            dane.erase(dane.begin()+(pozycja-usuniete));
            usuniete++;
        }
        pozycja++;
    }
    ui->ostatniPomiar_textbox->clear();
    foreach(QString a, dane)
    {
        ui->ostatniPomiar_textbox->append(a);
    }
    emit sygnalDoSredniej();
}


//odebranie danych do sredniej statystycznej z kilku pomiarow
void MainWindow::odbiorDanych(ulong index, double data)
{

   // sredniaStatystyczna=suma/wektorSredniej.size();//obliczenie sredniej pomiarow suma przez ilosc
    dane.push_back(QString::number(index)+"\t"+QString::number(data));
    ui->ostatniPomiar_textbox->clear();   
    emit sygnalDoSredniej();
    foreach(QString a, dane)
    {
        ui->ostatniPomiar_textbox->append(a);
       // ui->sredniaPomiarow_textbox->setPlainText(QString::number(sredniaStatystyczna));
        //wyswietlenie wartosci sredniej w odpowiednim oknie

    }

}




void MainWindow::odbiorDanychzPortu()
{

    QString ramka;
    QString wysuniecieString;
    QString czasString;

    //warunek kiedy moze czytac linie to ja czyta, while dlatego ze canReadLine zwraca bool, tldr: SZYBCIEJ i LEPIEJ
    while (port->canReadLine())
    {
        ramka = port->readLine().toHex();
    }

    //filtr bo to wyzej czasami zwraca pusty string
    if(ramka!="")
    {
       //dzielimy hexdane na 2 za pomoca "TD" czyli u nas to '5444' i odrazu wybieramy .last czyli ta 2 czesc
       wysuniecieString = ramka.split("5444").last();

       //usuwamy ostatnie 2 bajty ze eznakami specjalnymy zeby liczba byla ladna
       wysuniecieString = wysuniecieString.remove("0d0a");

       //konwersja liczby do ASCII
       wysuniecieString = QByteArray::fromHex(wysuniecieString.toLocal8Bit());

       //ROBIMY TO SAMA OPERACJE DLA CZASU

       //dzielimy hexdane na 2 za pomoca "TD" czyli u nas to '5444' i odrazu wybieramy .first czyli ta 1 czesc
       czasString = ramka.split("5444").first();

       //usuwamy ostatnie 2 bajty ze eznakami specjalnymy zeby liczba byla ladna
       czasString = czasString.remove("54");

       //konwersja liczby do ASCII
       czasString = QByteArray::fromHex(czasString.toLocal8Bit());


       //emit liczby w formacie string do mainwindow
       emit gotoweWartosci(wysuniecieString,czasString);
    }
}

void MainWindow::obliczanieSredniej()
{
    double suma=0;

    foreach(QString a, dane)
    {
        suma=suma+a.split("\t").last().toDouble();
    }
    sredniaStatystyczna = suma / dane.size();
    ui->sredniaPomiarow_textbox->setPlainText(QString::number(sredniaStatystyczna));

}





//nie robie .readAll tylko .readLine bo dane przychodza ze znakiem nowej linii \n wiec po co sb utrudniac robote
//pobieram z serialportu dane w FORAMCIE STRING HEX np : "54353154443438332e33360d0a"
//"54353154443438332e33360d0a" to dokladnie to samo co "T51TD483.36\r\n", poprostu zapisane w bajtach
//czyli poczatek: "T51TD" to to samo co "5435315444" bo wartosci znakow w hex: 'T'-54  '5'-35  '1'-31  'T'-54  'D'-44
//zapisuje to w hex dlatego zeby pozbyc sie tych ostatnich znaków \r\n (0d0a) BO ONE ZOSTAWAŁY W WARTOSCI LICZBOWEJ ,no i imo lepiej operuje sie na bajtach, PEWNIEJSZA ROBOTA


//dalszy opis nizej ale majac taki blok danych bajtow zapisanych w QString : "54353154443438332e33360d0a"
//poprostu pozbywam sie tych bajtow ktore mnie nie interesuja, zostawiamy same bajty ktore skladaja sie na wartosc wysuniecia sznurka
//czyli najpierw robie ten sam split co był tylko nie 'daneString.split("TD")' , tylko 'daneString.split("5444")' bo 'T' to to samo co bajt '54', inny zapis tylko
//dalej wybieram .last() z tak podzielonego stringa, teraz moja zmienna zawiera tylko : "3438332e33360d0a" - czyli "483.36\r\n"
//teraz w łatwy sposób usuwamy 2 ostatnie bajty '.remove("0d0a")' czyli znaki  \n (newline) i iakis inny (\r) chuj wie jaki, cos z MAC OSX
//po tych operacjach zostajemy z ladna czysta i sliczna liczba '483.36' ale zapisana w bajtach, czyli: "3438332e3336"

//no to myk konwersja znaków hex do ASCII zeby byla czytelna dla czloweka za pomoca funkcji '.toLocal8Bit()'
//po tym wszystkim mamy ladna liczbe '483.36' zapisana jako string, czyli mozna teraz uzyc funkcji do konwersji '.toFloat()' albo '.toDouble()' , wedle uznania



