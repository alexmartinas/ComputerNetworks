#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char msg[200];		// mesajul trimis/primit

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[client] Eroare la socket().\n");
      return errno;
    }
  

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

  /* citirea datelor de la client */
  bzero (msg, 200);
  int ok=1,nr;
  printf ("[client]Introduceti numarul de inmatriculare: ");
  fflush (stdout);
  read(1, &msg,100);//citim nr de inmatriculare
  nr=strlen(msg)-1;
  /* trimiterea nr inmatriculare  la server */
   if (write (sd, &nr, sizeof(int)) <= 0)
    {
     perror ("[cliect]Eroare la write() catre server.\n");
     return errno;     
    }
  if (write (sd, msg, nr) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }

  printf ("[client]Introduceti puntul de plecare: ");
  fflush (stdout);
  bzero(msg,200);
  read(1, &msg,100);//citim punctul de plecare
  nr=strlen(msg)-1;
  /* trimiterea punctului de plecare la server */
   if (write (sd, &nr, sizeof(int)) <= 0)
    {
     perror ("[cliect]Eroare la write() catre server.\n");
     return errno;     
    }
  if (write (sd, msg, nr) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }

  printf ("[client]Introduceti destinatia: ");
  fflush (stdout);
  bzero(msg,200);
  read(1,&msg,100);//citim destinatia
  nr=strlen(msg)-1;
      /* trimiterea destinatiei la server */
   if (write (sd, &nr, sizeof(int)) <= 0)
    {
     perror ("[cliect]Eroare la write() catre server.\n");
     return errno;     
    }
  if (write (sd, msg, nr) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }
  char peco[5];
  printf("[client]Doriti sa primiti notificari cu privire la statiile peco?");
  fflush(stdout);
  bzero(peco,5);
  read(1,&peco,5);
  nr=strlen(peco)-1;
  /*trimitem preferinta cu privire la statiile peco*/
  if (write (sd, &nr, sizeof(int)) <= 0)
    {
     perror ("[cliect]Eroare la write() catre server.\n");
     return errno;     
    }
  if (write (sd, peco, nr) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }

int viteza;
viteza=rand()%40+30;//viteza initiala a clientului

      /* trimiterea mesajului la server */
   if (write (sd, &viteza, sizeof(int)) <= 0)
    {
     perror ("[cliect]Eroare la write() catre server.\n");
     return errno;     
    }

  /* citirea raspunsului dat de server */
  
   int n,count,i;
   char traseu[100][100];
   if (read (sd,&n,sizeof(int)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
   for(i=0;i<n;i++)
   {
      if (read (sd,&count,sizeof(int)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
      if (read (sd,traseu[i],count) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
   }
   printf("Traseul este: ");
   for(i=0;i<n;i++)
     printf("%s-",traseu[i]);
   printf("\n");
   for(i=0;i<n;i++)//traseul propriu zis
   {
      bzero(msg,200);
      strcpy(msg,"1");
      strcat(msg,traseu[i]);
      nr=strlen(msg);
       if (write (sd, &nr, sizeof(int)) <= 0)
      {
          perror ("[cliect]Eroare la write() catre server.\n");
          return errno;     
      }
       if (write (sd, msg, nr) <= 0)//trimitem strada actuala
      {
           perror ("[client]Eroare la write() spre server.\n");
           return errno;
      }
      bzero(msg,200);
       if (read (sd, &nr,sizeof(int)) < 0)//citim limita de viteza pe strada actuala
      {
           perror ("[client]Eroare la read() de la server.\n");
           return errno;
      }
      if (read (sd, msg,nr) < 0)//citim limita de viteza pe strada actuala
      {
           perror ("[client]Eroare la read() de la server.\n");
           return errno;
      }

      printf("%s\n",msg);
       if(strcmp(peco,"Da"))
       {
        bzero(msg,200);
          if (read (sd,&nr,sizeof(int)) < 0)//citim informatiile despre peco pe strada actuala
          {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
          }
          if (read (sd,msg,nr) < 0)
          {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
          }
          printf("%s\n",msg);//afisam informatii peco
       }

       for(int j=1;j<=3;j++)
        {
            bzero(msg,200);
            int x=rand()%40+30;
            sprintf(msg,"0%d",x);
            nr=strlen(msg);
            if (write (sd, &nr, sizeof(int)) <= 0)//trimitem viteza cu care circulam
      {
          perror ("[cliect]Eroare la write() catre server.\n");
          return errno;     
      }
        if (write (sd, msg, nr) <= 0)//trimitem semnal sfarsit
      {
           perror ("[client]Eroare la write() spre server.\n");
           return errno;
      }
            bzero(msg,200);
            if (read (sd,&nr,sizeof(int)) < 0)//citim raspunsul server-ului
          {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
          }
          if (read (sd,msg,nr) < 0)
          {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
          }
          printf("%s\n",msg);//afisam mesajul
          fflush(stdout);
          sleep(3);
        }
       if(i==n-1)
    { 
      bzero(msg,200);
       strcpy(msg,"sfarsit");
       nr=strlen(msg);
        if (write (sd, &nr, sizeof(int)) <= 0)
      {
          perror ("[cliect]Eroare la write() catre server.\n");
          return errno;     
      }
        if (write (sd, msg, nr) <= 0)//trimitem semnal sfarsit
      {
           perror ("[client]Eroare la write() spre server.\n");
           return errno;
      }
    }
   }
  /* inchidem conexiunea, am terminat */
  close (sd);
}


