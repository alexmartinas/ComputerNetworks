#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <QTimer>
#include <QStyle>
#include <QDesktopWidget>
#include <QObject>
using namespace std;

int sd;//descriptorul de socket
char TRASEU[100][100];//memoreaza traseul clientului
int NR=0;//iterator pentru traseu
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::setFixedSize(360,300);
    MainWindow::setGeometry(QStyle::alignedRect(
                                Qt::LeftToRight,
                                Qt::AlignCenter,
                                this->size(),
                                qApp->desktop()->availableGeometry()
                            ));
    ui->Ecran->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::CitireServer()//citim informatiile de la server
{
    int nr=0;
    char msg[1000];
    if (read (sd, &nr, sizeof(int)) <= 0)
     {
      QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
     }
   if (read (sd,msg, nr) <= 0)
     {
       QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");;
     }
    if(strcmp(msg,"sfarsit")==0)
    {
        sleep(4);
        timerCitire->stop();
        ui->Ecran->insertPlainText("Ati ajuns la destinatie.\nVa multumim pentru utilizare\nLa revedere!\n");
    }
   ui->Ecran->insertPlainText(msg);
   ui->Ecran->insertPlainText("\n");
}

void MainWindow::SchimbareStrada()//trimitem strada pe care ne aflam
{

    int nr;
    char msg[100];
    QString destinatie=ui->Destinatie->currentText();
    nr=strlen(TRASEU[NR]);
    if(strcmp(TRASEU[NR],destinatie.toStdString().c_str()))
    if (write (sd, &nr, sizeof(int)) <= 0)
     {
      QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
     }
   if (write (sd,TRASEU[NR++], nr) <= 0)
     {
       QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");;
     }
    else
   {
       timerStrada->stop();
       timerViteza->stop();
       timerSport->stop();
       timerVreme->stop();
       strcpy(msg,"sfarsit");
       nr=strlen(msg);
       if (write (sd, &nr, sizeof(int)) <= 0)
        {
         QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
        }
      if (write (sd,msg, nr) <= 0)
        {
          QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");;
        }

   }
}

void MainWindow::InfoSport()
{
    char msg[100];
    bzero(msg,100);
    int nr;
    strcpy(msg,"3sport");
    nr=strlen(msg);
    if (write (sd, &nr, sizeof(int)) <= 0)
     {
      QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
     }
   if (write (sd,msg, nr) <= 0)
     {
       QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");;
   }
}

void MainWindow::InfoVreme()
{
    char msg[100];
    bzero(msg,100);
    int nr;
    strcpy(msg,"2vreme");
    nr=strlen(msg);
    if (write (sd, &nr, sizeof(int)) <= 0)
     {
      QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
     }
   if (write (sd,msg, nr) <= 0)
     {
       QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");;
   }
}

void MainWindow::Viteza()
{
    char msg[100];
    bzero(msg,100);
    int nr,viteza;
    viteza=rand()%80+20;
    sprintf(msg,"3%d",viteza);
    nr=strlen(msg);
    if (write (sd, &nr, sizeof(int)) <= 0)
     {
      QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
     }
   if (write (sd,msg, nr) <= 0)
     {
       QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");;
     }
}

void MainWindow::on_Start_clicked()
{
    if(ui->NrInmatriculare->text()==NULL || ui->Plecare->currentText()=="Alegeti" || ui->Destinatie->currentText()=="Alegeti")
    {
        QMessageBox::critical(this,"Eroare","Nu ati introdus punctul de plecare/destinatie/nr inmatriculare");
        return ;
    }
    else if(ui->Plecare->currentText()==ui->Destinatie->currentText())
            {
                 QMessageBox::critical(this,"Eroare","Punctul de plecare trebuie sa fie diferit de destinatie");
                 return ;
            }
    ui->NrInmatriculare->hide();
    ui->Destinatie->hide();
    ui->Plecare->hide();
    ui->NotificariPeco->hide();
    ui->Start->hide();
    ui->label->hide();
    ui->label_2->hide();
    ui->label_3->hide();
    ui->Ecran->show();
    //marim si centram fereastra
    MainWindow::setFixedSize(800,600);
    MainWindow::setGeometry(QStyle::alignedRect(
                                Qt::LeftToRight,
                                Qt::AlignCenter,
                                this->size(),
                                qApp->desktop()->availableGeometry()
                            ));
    int sd;			// descriptorul de socket
    struct sockaddr_in server;	// structura folosita pentru conectare
    char msg[200];		// mesajul trimis/primit
    int port=5591;
    /* cream socketul */
      if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
        {
          QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
        }
      /* umplem structura folosita pentru realizarea conexiunii cu serverul */
        /* familia socket-ului */
        server.sin_family = AF_INET;
        /* adresa IP a serverului */
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
        /* portul de conectare */
        server.sin_port = htons (port);

        /* ne conectam la server */
          if (::connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
            {
              QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
            }
       int nr;
       QString NrInmatriculare,Plecare,Destinatie;
       NrInmatriculare=ui->NrInmatriculare->text();
       nr=strlen(NrInmatriculare.toStdString().c_str());
       /* trimiterea nr inmatriculare  la server */
          if (write (sd, &nr, sizeof(int)) <= 0)
           {
            QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
           }
         if (write (sd, NrInmatriculare.toStdString().c_str(), nr) <= 0)
           {
              QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
           }
        Plecare=ui->Plecare->currentText();
       nr=strlen(Plecare.toStdString().c_str());
       /*trimitem punctul de plecare la server*/
       if (write (sd, &nr, sizeof(int)) <= 0)
        {
          QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
        }
      if (write (sd,Plecare.toStdString().c_str(), nr) <= 0)
        {
           QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
        }

      Destinatie=ui->Destinatie->currentText();
      nr=strlen(Destinatie.toStdString().c_str());
      /*trimitem destinatia la server*/
      if (write (sd, &nr, sizeof(int)) <= 0)
       {
         QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
       }
     if (write (sd, Destinatie.toStdString().c_str(), nr) <= 0)
       {
         QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
       }

     /*trimitem preferinta cu privire la notificari*/
     if(ui->NotificariPeco->isChecked())
         strcpy(msg,"Da");
     else strcpy(msg,"Nu");
     nr=strlen(msg);
     if (write (sd, &nr, sizeof(int)) <= 0)
      {
         QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
     }
    if (write (sd, msg, nr) <= 0)
      {
        QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
      }
    int viteza;
    viteza=rand()%80+20;//viteza initiala a clientului

          /* trimiterea vitezei initiale la server */
       if (write (sd, &viteza, sizeof(int)) <= 0)
        {
          QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
        }
     //citim traseul de la server
       int i,n,count;
          if (read (sd,&n,sizeof(int)) < 0)
           {
               QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
           }
          for(i=0;i<n;i++)
          {
             if (read (sd,&count,sizeof(int)) < 0)
           {
             QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
           }
             if (read (sd,TRASEU[i],count) < 0)
           {
              QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
           }
          }

      //afisam traseul clientului
            ui->Ecran->insertPlainText("Traseul dumneavoastra este:\n");
            ui->Ecran->insertPlainText(TRASEU[0]);
          for(i=1;i<n;i++)
          {
              ui->Ecran->insertPlainText("-");
              ui->Ecran->insertPlainText(TRASEU[i]);
          }
          int pid;
        //incepem sa parcurgem traseul si sa comunicam cu serverul
          pid=fork();
          if(pid==-1)
              QMessageBox::critical(this,"Eroare","Oops!Ceva neasteptat s-a intamplat");
          else
            if(pid==0)
            {
                timerStrada=new QTimer(this);
                QObject::connect(timerStrada,SIGNAL(timeout()),this,SLOT(SchimbareStrada()));   // schimbam strada la fiecare 1 mmin
                timerStrada->start(60000);

                timerSport=new QTimer(this);
                QObject::connect(timerSport,SIGNAL(timeout()),this,SLOT(InfoSport()));   // cerem informatii sportive
                timerStrada->start(2000);

                timerVreme=new QTimer(this);
                QObject::connect(timerVreme,SIGNAL(timeout()),this,SLOT(InfoVreme()));   // cerem informatii despre vreme
                timerVreme->start(20000);

                timerViteza=new QTimer(this);
                QObject::connect(timerViteza,SIGNAL(timeout()),this,SLOT(Viteza()));   // trimitem viteza cu care se circula la fiecare 10 secunde
                timerViteza->start(1000);
            }
            else
            {
                int flags = fcntl(sd, F_GETFL, 0);   //facem read-ul neblocant
                fcntl(sd, F_SETFL, flags | O_NONBLOCK);
                timerCitire=new QTimer(this);
                QObject::connect(timerCitire,SIGNAL(timeout()),this,SLOT(CitireServer()));   //citim informatii de la server
                timerCitire->start(100);//citim la fiecare 100 milisecunde
            }

}
