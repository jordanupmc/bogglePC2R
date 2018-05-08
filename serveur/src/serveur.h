#include <pthread.h>
#include "game.h"
#include "threadSafeList.h"

#ifndef SERVEUR_H_INCLUDED
#define SERVEUR_H_INCLUDED
#define MAX 256
#define MAX_CLIENT 8
#define NB_rand 6
#define NB_des 16
#define MAX_TRAJ 32
#define MAX_POOL_CONNEXION 8
#define MAX_POOL_TROUVE 2
#define MAX_POOL_VERIF 4
#define MAX_POOL_CHAT 2
#define MAX_REP_BIENVENUE 35
#define PORT_DICO 5500
#define PORT_DEFAULT 2018
#define NB_TOUR_DEFAULT 5
#define JOURNAL_PATH "journal/journal.jou"

typedef struct data{
  unsigned char type;
  int fromSock;
  char nom[MAX];
  char author[MAX];
  char mot[MAX];
  char traj[MAX_TRAJ];
  char msg[MAX];
} data;

typedef struct poolArg  {
  int * nbWaiting;
  int * clients;
  joueur ** players;
  int threadID;
  int * boolTimer;
} poolArg;

typedef struct verifArg  {
  int * nbVerif;
  int * nbVerif2;
  int * cptJob;
  joueur ** players;
} verifArg;

typedef struct timerArg  {
  int * bool;
  int * nbVerif;
  int * nbVerif2;
  joueur ** players;
} timerArg;

typedef struct trouveArg {
  joueur ** players;
  data ** tabReq;
  int * nbTrouve;
  int * timerStat;
} trouveArg;

typedef struct chatArg{
  joueur ** players;
  data ** tabReq;
  int * nbChat;
}chatArg;


extern nodePropose * proposition;
extern int nbTourSession;

extern char isFormatCorrect(char * req, int size);
extern data * parseRequest(char * req, int size, int newSock);
extern int readInChan(int sock, char * buf, int size);
extern void reduceScore();
extern int connectWithWordsServer();
extern char isWordsServerReady();
#endif
