#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#include <queue>
#include <stack>
using namespace std;
/* portul folosit */
#define PORT 5591
fd_set Clienti;
int maxClienti;
int BUFFER_BD[100];//salveaza rezultatul returnat de baza de date
char STRADA_BD[1000];
char STRAZI_BD[100][100];//folosit pt a trimite toate strazile clientului
int NR;//contor folosit de BUFFER_BD
/* codul de eroare returnat de anumite apeluri */
extern int errno;
  sqlite3 *db;

typedef struct thData
{
	int idThread; //id-ul thread-ului care va deservi clientul
	int cl; //descriptorul intors de accept
	int viteza;//viteza de circulatie
	char NumarMasina[100];//identificarea unica prin placuta de inmatriculare;
	char strada[100]; // strada actuala
	char plecare[100]; // punctul de plecare
	char destinatie[100]; //destinatia finala
  char peco[5];
}thData;



static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */

void raspunde (void *);
void VerificareViteza(thData *);
void InfoSport(thData *);
void InfoVreme(thData *);
void InfoPeco(thData *);
void UpdateStrada(thData *);

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 5) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread
      socklen_t length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	if(client>maxClienti)
		maxClienti=client;
        /* s-a realizat conexiunea, se astepta mesajul */

	int idThread; //id-ul threadului
	int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));
	td->idThread=i++;
	td->cl=client;
	FD_SET(td->cl,&Clienti);//adaugam descriptorul noului client la setul de descriptori activi
	pthread_create(&th[i], NULL, &treat, td);//am creat thread-ul care v-a servi noul client

	}//while
};

int callback(void *data, int argc, char **argv, char **azColName)
{
  
  BUFFER_BD[NR++]=atoi(argv[0]);
  return 0;
}

int callback1(void *data, int argc, char **argv, char **azColName)
{
  
  strcpy(STRADA_BD,argv[0]);
  return 0;
}

int traseu(thData *client)//calculeaza traseul clinetului
{
  int vecini[200];
  int start,stop;
  int dist[200];
  int drum[200];
  int viz[200];
  int in_coada[100];
  queue<int> coada;
  //calculam traseul folosind alg lui Bellman Ford
  char interogare[200];
  bzero(interogare,200);
  strcpy(interogare,"select Id_strada  from strazi where Nume_strada='");
  strcat(interogare,client->plecare);
  strcat(interogare,"';");
  char *errMsg;
  NR=0;
    if(sqlite3_exec(db,interogare,callback,NULL,&errMsg)!=SQLITE_OK)//cautam id strazii de plecare
   {
      printf("SQL error: %s\n", errMsg);
      sqlite3_free(errMsg);
      return 0;
   }
  start=BUFFER_BD[0];
  bzero(interogare,200);
  printf("Id strazii plecare: %d\n",start);
  strcpy(interogare,"select Id_strada  from strazi where Nume_strada='");
  strcat(interogare,client->destinatie);
  strcat(interogare,"';");
  NR=0;
    if(sqlite3_exec(db,interogare,callback,NULL,&errMsg)!=SQLITE_OK)//cautam id strazii destinatie
   {
      printf("SQL error: %s\n", errMsg);
      sqlite3_free(errMsg);
      return 0;
   }
   stop=BUFFER_BD[0];
   printf("Id strazii destinatie: %d\n",stop);
   int nod;
   coada.push(start);
   for(int i=0;i<100;i++)
    {
      in_coada[i]=0;
      dist[i]=9999;
      drum[i]=0;
      viz[i]=0;
    }
    dist[start]=0;
   while(coada.size()!=0)
   {
      nod=coada.front();
      in_coada[nod]=0;
      viz[nod]=1;
      NR=0;
      char s[3];
      strcpy(interogare,"select Limita_viteza from Strazi where Id_strada=");
      sprintf(s,"%d",nod);
      strcat(interogare,s);
       if(sqlite3_exec(db,interogare,callback,NULL,&errMsg)!=SQLITE_OK)//limita de viteza pentru strada actuala(nod)
       {
           printf("SQL error: %s\n", errMsg);
          sqlite3_free(errMsg);
          return 0;
       }
       int speed=BUFFER_BD[0];
      strcpy(interogare,"select Id_legatura from Leg_strazi where Id_strada=");
      sprintf(s,"%d",nod);
      strcat(interogare,s);
      strcat(interogare," union select Id_strada from Leg_strazi where Id_legatura=");
      strcat(interogare,s);
      NR=0;
      if(sqlite3_exec(db,interogare,callback,NULL,&errMsg)!=SQLITE_OK)//strazile vecine cu strada actuala
       {
           printf("SQL error: %s\n", errMsg);
          sqlite3_free(errMsg);
          return 0;
       }
       int i;
      for( i=0;i<NR;i++)
        {  
          if(in_coada[BUFFER_BD[i]]==0 && viz[BUFFER_BD[i]]==0 && BUFFER_BD[i]!=start)
          { 
            coada.push(BUFFER_BD[i]);
            in_coada[BUFFER_BD[i]]=1;
          }
          if(speed+dist[nod]<dist[BUFFER_BD[i]])
            {
              dist[BUFFER_BD[i]]=speed+dist[nod];
              drum[BUFFER_BD[i]]=nod;
              viz[nod]=0;
            }
        }
      coada.pop();
   }
      int i=stop;
      char s[3];
      stack<int> stiva;
      stiva.push(stop);
      while(drum[i]!=start)
      {
        stiva.push(drum[i]);
        i=drum[i];
      }
      stiva.push(drum[i]);
      int x=stiva.size();
      if (write (client->cl,&x,sizeof(int)) <= 0)//scriem nr de strazi ale traseului
      {
        printf("[Thread %d] ",client->idThread);
         perror ("[Thread]Eroare la write() catre client.\n");
         return 0;
      }
      while(!stiva.empty())
      {
        nod=stiva.top();
        stiva.pop();
        strcpy(interogare,"select Nume_strada from Strazi where Id_strada=");
      sprintf(s,"%d",nod);
      strcat(interogare,s);
      NR=0;
      if(sqlite3_exec(db,interogare,callback1,NULL,&errMsg)!=SQLITE_OK)//strazile vecine cu strada actuala
       {
           printf("SQL error: %s\n", errMsg);
          sqlite3_free(errMsg);
          return 0;
       }
       int count=strlen(STRADA_BD);
       if (write (client->cl,&count,sizeof(int)) <= 0)//scriem lungimea strazii
      {
         printf("[Thread %d] ",client->idThread);
         perror ("[Thread]Eroare la write() catre client.\n");
         return 0;
      }
       if (write (client->cl,STRADA_BD,count) <= 0)//scriem nr de strazi ale traseului
      {
        printf("[Thread %d] ",client->idThread);
         perror ("[Thread]Eroare la write() catre client.\n");
         return 0;
    }
  }

    return 1;
}

int callback2(void *data, int argc, char **argv, char **azColName)
{
  
  sprintf(STRAZI_BD[NR++],"%s",argv[0]);
  return 0;
}

static void *treat(void *arg) /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
{
			struct thData tdL;
		tdL= *((struct thData*)arg);
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);
		pthread_detach(pthread_self());
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		printf("[Thread %d] Clientul s-a deconectat\n",tdL.idThread);
    FD_CLR(((struct thData*)arg)->cl,&Clienti); // Scoatem descriptorul clientului din lista de descriptori activi
		close (((struct thData*)arg)->cl);
		return(NULL);
};

 void trimite_strazi(thData *client)
 {
   if(sqlite3_open("database.db",&db)!=SQLITE_OK)//deschidem BD
    {
      printf( "Eroare la deschiderea bazei de date: %s\n", sqlite3_errmsg(db));
      return ;
    }
    else printf("Conectare la baza de date cu succes\n");
    
  char interogare[100];
  bzero(interogare,100);
  strcpy(interogare,"select Nume_strada from Strazi");
  char *errMsg;
  NR=0;
    if(sqlite3_exec(db,interogare,callback2,NULL,&errMsg)!=SQLITE_OK)//cautam id strazii de plecare
   {
      printf("SQL error: %s\n", errMsg);
      sqlite3_free(errMsg);
      return ;
   }
int nr;
   if (write (client->cl,&NR,sizeof(int)) <= 0)//scriem nr de strazi
      {
         printf("[Thread %d] ",client->idThread);
         perror ("[Thread]Eroare la write() catre client.\n");
         return;
      }
   for(int i=0;i<NR;i++)
   {
        nr=strlen(STRAZI_BD[i]);
       if (write (client->cl,&nr,sizeof(int)) <= 0)//scriem lungimea strazii
      {
         printf("[Thread %d] ",client->idThread);
         perror ("[Thread]Eroare la write() catre client.\n");
         return;
      }
       if (write (client->cl,STRAZI_BD[i],nr) <= 0)//scriem strada
      {
        printf("[Thread %d] ",client->idThread);
         perror ("[Thread]Eroare la write() catre client.\n");
         return;

      }
   }
}

void raspunde(void *arg)
 {
        int nr, i=0,ok=0;
	struct thData tdL;
	tdL= *((struct thData*)arg);
  char raspuns[200];
	char nr_inmatriculare[100];
  char plecare[100];
  char destinatie[100];
  char msg[200];
  int viteza;
bzero(raspuns,200);
bzero(nr_inmatriculare,100);
bzero(plecare,100);
bzero(destinatie,100);
bzero(msg,200);
//trimitem strazile din baza de date
  trimite_strazi(&tdL);

  //citim nr inmatriculare
	if (read (tdL.cl, &nr,sizeof(int)) <= 0)
			{
			  printf("[Thread %d]\n",tdL.idThread);
			  perror ("Eroare la read() de la client.\n");
        return;
			}
  if (read (tdL.cl, &nr_inmatriculare,nr) <= 0)
       {
         printf("[Thread %d]\n",tdL.idThread);
          perror ("Eroare la read() de la client.\n");
          return;
       }
printf ("[Thread %d]Numar masina %s\n",tdL.idThread,nr_inmatriculare);
  //citim punct plecare
  if (read (tdL.cl, &nr,sizeof(int)) <= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
        return;
      }
  if (read (tdL.cl, &plecare,nr) <= 0)
       {
         printf("[Thread %d]\n",tdL.idThread);
          perror ("Eroare la read() de la client.\n");
          return;
       }
  printf ("[Thread %d]Punct plecare %s\n",tdL.idThread,plecare);
  //citim destinatie
  if (read (tdL.cl, &nr,sizeof(int)) <= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
        return;
      }
  if (read (tdL.cl, &destinatie,nr) <= 0)
       {
         printf("[Thread %d]\n",tdL.idThread);
          perror ("Eroare la read() de la client.\n");
          return;
       }
  printf ("[Thread %d]Destinatie %s\n",tdL.idThread,destinatie);
  //citim preferinte peco
  if (read (tdL.cl, &nr,sizeof(int)) <= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
        return;
      }
  if (read (tdL.cl, &msg,nr) <= 0)
       {
         printf("[Thread %d]\n",tdL.idThread);
          perror ("Eroare la read() de la client.\n");
          return;
       }
  strcpy(tdL.peco,msg);
  //citim viteza initiala
  if (read (tdL.cl, &viteza,sizeof(int)) <= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
        return;
      }
  printf ("[Thread %d]Viteza initiala %d\n",tdL.idThread,viteza);
	printf ("[Thread %d]Datele initiale au fost receptionate...\n",tdL.idThread);
  
		      /*pregatim mesajul de raspuns */
	printf("[Thread %d]Trimitem mesajul inapoi...\n",tdL.idThread);

strcpy(tdL.NumarMasina,nr_inmatriculare);
strcpy(tdL.plecare,plecare);
strcpy(tdL.destinatie,destinatie);
tdL.viteza=viteza;
printf("[Thread %d] S-a conectat clientul cu urmatoarele date:\nNr inmatriculare: %s\nPlecare: %s\nDestinatie: %s\n",tdL.idThread,tdL.NumarMasina,tdL.plecare,tdL.destinatie);
  
    ok=traseu(&tdL);
    if(ok==0)
        printf("Traseu a returnat eroare\n");
    else printf("Traseu s-a terminat cu succes\n");
  
    if (read (tdL.cl, &nr,sizeof(int)) <= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
        return;
      }
    if (read (tdL.cl,msg,nr )<= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
        return;
      }
    while(strcmp(msg,"sfarsit")!=0)
    {
      if(msg[0]=='0')
      {
         int x;
         x=atoi(msg+1);
         tdL.viteza=x;
         VerificareViteza(&tdL);
      }
      else
        if(msg[0]=='1')
        {
          strcpy(tdL.strada,msg+1);
          UpdateStrada(&tdL);
        }
        else
          if(msg[0]=='2')
              InfoVreme(&tdL);
          else
            if(msg[0]=='3')
                InfoSport(&tdL);
            else//inseamna ca avem un raport,
            {
              for(int i=0;i<maxClienti;i++)
                    if(FD_ISSET(i,&Clienti))
                    {
                      if (write (i,&nr,sizeof(int)) <= 0)//scriem lungimea strazii
                      {
                           perror ("[Thread]Eroare la write() catre client.\n");
                           exit(1);
                      }
                      if (write (i,msg,nr) <= 0)//scriem nr de strazi ale traseului
                       {
                           perror ("[Thread]Eroare la write() catre client.\n");
                             exit(1);
                        }

                   }
            }
        bzero(msg,200);
    if (read (tdL.cl, &nr,sizeof(int)) <= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
        return;
      }
    if (read (tdL.cl,msg,nr )<= 0)
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");
        return;
      }     
    }
    bzero(msg,100);
    strcpy(msg,"sfarsit");
    nr=strlen(msg);
    if (write (tdL.cl, &nr, sizeof(int)) <= 0)
    {
     printf("[Thread %d] ",tdL.idThread);
     perror ("[Thread]Eroare la write() catre client.\n");
     return;
    }
   if (write (tdL.cl,msg,nr) <= 0)
    {
     printf("[Thread %d] ",tdL.idThread);
     perror ("[Thread]Eroare la write() catre client.\n");
     return;
    }
printf("[Thread %d] S-a deconectat clientul cu urmatoarele date:\nNr inmatriculare: %s\nPlecare: %s\nDestinatie: %s\n",tdL.idThread,tdL.NumarMasina,tdL.plecare,tdL.destinatie);

}

void InfoVreme(thData *client)
{
int temperatura,precipitatii,vant;
char nori[20];
bzero(nori,20);
char raspuns [200];
bzero(raspuns,200);
temperatura=rand()%40;
precipitatii=rand()%101;
vant=rand()%121;
if(vant%3==0)
   strcpy(nori,"Cer senin");
else
  if(vant%3==1)
      strcpy(nori,"Cer partial noros");
    else strcpy(nori,"Cer noros");
sprintf(raspuns,"Informatii vreme:\nTemperatura medie: %d grade Celsius\nSanse Precipitatii: %d%% \nViteza vant: %dkm/h%s\n",temperatura,precipitatii,vant,nori);
int nr;
nr=strlen(raspuns);
   if (write (client->cl, &nr, sizeof(int)) <= 0)
    {
     printf("[Thread %d] ",client->idThread);
     perror ("[Thread]Eroare la write() catre client.\n");
     return;
    }
   if (write (client->cl,raspuns,nr) <= 0)
    {
     printf("[Thread %d] ",client->idThread);
     perror ("[Thread]Eroare la write() catre client.\n");
     return;
    }
printf("[Thread %d]Am trimis informatii despre vreme\n",client->idThread);


}

void InfoSport(thData *client)
{
char *errMsg;
char interogare[200];
char s[10];
bzero(s,10);
bzero(interogare,200);
int x=rand()%4+1;
strcpy(interogare,"select Info_Sport from Sport where Id=");
sprintf(s,"%d",x);
strcat(interogare,s);
   if(sqlite3_exec(db,interogare,callback1,NULL,&errMsg)!=SQLITE_OK)//limita de viteza pentru strada actuala
    {
        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return ;
    }
int nr;
nr=strlen(STRADA_BD);
   if (write (client->cl, &nr, sizeof(int)) <= 0)
    {
     printf("[Thread %d] ",client->idThread);
     perror ("[Thread]Eroare la write() catre client.\n");
     return;
    }
   if (write (client->cl,STRADA_BD,nr) <= 0)
    {
     printf("[Thread %d] ",client->idThread);
     perror ("[Thread]Eroare la write() catre client.\n");
     return;
    }
printf("[Thread %d]Am trimis informatii sportive\n",client->idThread);


}

void UpdateStrada(thData *client)
{
char *errMsg;
char interogare[200];
bzero(interogare,200);
strcpy(interogare,"select Limita_viteza from Strazi where Nume_strada='");
strcat(interogare,client->strada);
strcat(interogare,"';");
NR=0;
   if(sqlite3_exec(db,interogare,callback,NULL,&errMsg)!=SQLITE_OK)//limita de viteza pentru strada actuala
    {
        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return ;
    }
char raspuns[200];
bzero(raspuns,200);
sprintf(raspuns,"Limita de viteza pe aceasta strada este de %d km\n",BUFFER_BD[0]);
int nr;
nr=strlen(raspuns);
   if (write (client->cl, &nr, sizeof(int)) <= 0)
    {
     printf("[Thread %d] ",client->idThread);
     perror ("[Thread]Eroare la write() catre client.\n");
     return;
    }
   if (write (client->cl,raspuns,nr) <= 0)
    {
     printf("[Thread %d] ",client->idThread);
     perror ("[Thread]Eroare la write() catre client.\n");
     return;
    }
printf("[Thread %d]Am trimis limita de viteza\n",client->idThread);
  if(strcmp(client->peco,"Da")==0)
      InfoPeco(client);

}

void InfoPeco(thData *client)
{
char *errMsg;
char interogare[200];
bzero(interogare,200);
bzero(STRADA_BD,1000);
strcpy(interogare,"select Peco from Strazi where Nume_strada='");
strcat(interogare,client->strada);
strcat(interogare,"';");
   if(sqlite3_exec(db,interogare,callback1,NULL,&errMsg)!=SQLITE_OK)//limita de viteza pentru strada actuala
    {
        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return ;
    }
char raspuns[200];
bzero(raspuns,200);
if(strcmp(STRADA_BD,"Nu")==0)
    sprintf(raspuns,"Nu exista statii peco pe %s.\n",client->strada);
else
{
  int motorina,benzina,gaz;
  motorina=rand()%100;
  benzina=rand()%100;
  gaz=rand()%100;
  sprintf(raspuns,"Preturile carburantilor pe %s sunt:\nMotorina 5.%d\nBenzina 5.%d\nGaz 3.%d\n",client->strada,motorina,benzina,gaz);
}
int nr;
nr=strlen(raspuns);
   if (write (client->cl, &nr, sizeof(int)) <= 0)
    {
     printf("[Thread %d] ",client->idThread);
     perror ("[Thread]Eroare la write() catre client.\n");
     return;
    }
   if (write (client->cl,raspuns,nr) <= 0)
    {
     printf("[Thread %d] ",client->idThread);
     perror ("[Thread]Eroare la write() catre client.\n");
     return;
    }
printf("[Thread %d]Am trimis informatiile despre statiile peco\n",client->idThread);

}

void VerificareViteza(thData *client)
{
char *errMsg;
char interogare[200];
strcpy(interogare,"select Limita_viteza from Strazi where Nume_strada='");
strcat(interogare,client->strada);
strcat(interogare,"';");
   if(sqlite3_exec(db,interogare,callback,NULL,&errMsg)!=SQLITE_OK)//limita de viteza pentru strada actuala
    {
        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return ;
    }
char raspuns[200];
bzero(raspuns,200);
if(client->viteza<=BUFFER_BD[0])
    sprintf(raspuns,"Sunteti pe %s.Conduceti in limita legala impusa pe acesta strada\n",client->strada);
else
    sprintf(raspuns,"Sunteti pe %s.Conduceti cu %dkm/h depasind limita legala de %dkm/h.Va rugam sa incetiniti!\n",client->strada,client->viteza,BUFFER_BD[0]);
int nr;
nr=strlen(raspuns);
   if (write (client->cl, &nr, sizeof(int)) <= 0)
    {
     printf("[Thread %d] ",client->idThread);
     perror ("[Thread]Eroare la write() catre client.\n");
     return;
    }
   if (write (client->cl,raspuns,nr) <= 0)
    {
     printf("[Thread %d] ",client->idThread);
     perror ("[Thread]Eroare la write() catre client.\n");
     return;
    }
printf("[Thread %d]Am raspuns corespunzator vitezei cu care circula clientul\n",client->idThread);

}
