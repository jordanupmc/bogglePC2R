#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include "serveur.h"
#include "game.h"
#include "synchro.h"

static int tour = 0;
static int session = 0;
static int nbPlayer = 0;

pthread_mutex_t mutJoueur = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condEmptyJoueur = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutTrouve = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condEmptyTrouve = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutVerif = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condEmptyVerif = PTHREAD_COND_INITIALIZER;
pthread_cond_t condEndVerif =  PTHREAD_COND_INITIALIZER;

inline int getTour(){return tour;}
inline int getSession(){return session;}

inline void setTour(int x){tour = x;}
inline void setSession(int x){tour = x;}

int getNbPlayer(){ return nbPlayer;}
joueur** initJoueurArray(){
  return  malloc( sizeof( joueur*) * MAX_CONNECTED_PLAYER );
}

joueur* getJoueurByName(char * name,joueur** tab){
  joueur* tmp;
  pthread_mutex_lock(&mutJoueur);
  tmp = getJoueurByNameNotSafe(name, tab);
  pthread_mutex_unlock(&mutJoueur);
  return tmp;
}
joueur* getJoueurByNameNotSafe(char * name,joueur** tab){
  int i;
  for(i=0; i< nbPlayer; i++){
    if( !strcmp(tab[i]->nom, name) )
      return tab[i];
  }
  return NULL;
}
joueur* newJoueur(int sock, char* nom){
  joueur * j = malloc( sizeof(joueur) );
  if( !j ){
    perror("newJoueur malloc ");
    return NULL;
  }
  j->sock= sock;
  
  memset(j->nom, 0, 100);
  int lNom = strlen(nom);
  strncpy(j->nom, nom, lNom < 99? lNom:99);
  return j;
}

joueur* getJoueurByNum(int x, joueur** tab){
  pthread_mutex_lock(&mutJoueur);
  joueur * tmp = getJoueurByNumNotSafe(x, tab);
  pthread_mutex_unlock(&mutJoueur);
  return tmp;
}
joueur* getJoueurByNumNotSafe(int x, joueur** tab){
  if(x >= nbPlayer)
    return NULL;
  return tab[x];
}

joueur* getJoueurBySocketNotSafe(int sock, joueur** tab){
  int i;
  for(i=0; i< nbPlayer; i++){
    if( tab[i] && tab[i]->sock == sock )
      return tab[i];
  }
  return NULL;
}

joueur* getJoueurBySocket(int sock, joueur** tab){
  pthread_mutex_lock(&mutJoueur);
  joueur * tmp = getJoueurBySocketNotSafe(sock, tab);
  pthread_mutex_unlock(&mutJoueur);
  return tmp;
}


/*gros bug
void addJoueur(joueur* j, joueur** tab ){
  pthread_mutex_lock(&mutJoueur);
  tab[nbPlayer++] = j;
  if(!tab[nbPlayer-1])
    printf("CRAPS NULL\n");
  if(!j)
    printf("WTF ?\n");
  printf("TOUT VA BIEN %d\n", nbPlayer-1);
    printf("j%d nom:%s sock:%d\n",nbPlayer-1, currentJoueur[nbPlayer-1]->nom, currentJoueur[nbPlayer-1]->sock);
  pthread_cond_signal( &condEmptyJoueur );
  pthread_mutex_unlock(&mutJoueur);
  printf("MUTEX ? NOP\n");
}
*/
void addNewJoueur(int sock, char * nom, joueur** tab ){

  joueur * j = malloc( sizeof(joueur) );
  if( !j ){
    perror("newJoueur malloc :( ");
  }
  j->sock= sock;
  j->score=0;
  strncpy(j->nom, nom, strlen(nom));
  j->nom[strlen(nom)]=0;
  
  pthread_mutex_lock(&mutJoueur);
  tab[nbPlayer++] = j;

  j->badExit=0;

  pthread_cond_signal( &condEmptyJoueur );
  pthread_mutex_unlock(&mutJoueur);
}

void removeJoueurByName(char * name,joueur** tab){
  int i,j;
  if(!name)
    return;

  pthread_mutex_lock(&mutJoueur);

  for(i=0; i< nbPlayer; i++){
    if( !strcmp(tab[i]->nom, name) || tab[i]->badExit ){
      shutdown(tab[i]->sock, 2);
      close(tab[i]->sock);
      free( tab[i] );
      tab[i]=NULL;
      for(j=i; j< nbPlayer-1; j++)
	tab[j]=tab[j+1];
      nbPlayer--;
      pthread_mutex_unlock(&mutJoueur);
      return;
    }      
  } 
  pthread_mutex_unlock(&mutJoueur);
  
}

void removeJoueurBySocket(int sock,joueur** tab){
  int i,j;
  if(sock<=0)
    return;

  pthread_mutex_lock(&mutJoueur);

  for(i=0; i< nbPlayer; i++){
    if( tab[i]->sock == sock || tab[i]->badExit ){
      shutdown(tab[i]->sock, 2);
      close(tab[i]->sock);
      free( tab[i] );
      tab[i] = NULL;
      for(j=i; j< nbPlayer-1; j++)
	tab[j]=tab[j+1];
      nbPlayer--;
      pthread_mutex_unlock(&mutJoueur);
      return;
    }      
  } 
  pthread_mutex_unlock(&mutJoueur);
  
}

void debug_printAllPlayer(joueur ** tab){
  pthread_mutex_lock(&mutJoueur);
  int i;
  printf("NB_PLAYER = %d\n", nbPlayer);
  for(i=0; i< nbPlayer; i++){
    //if(tab[i])
      printf("j%d nom:%s sock:%d\n",i, tab[i]->nom, tab[i]->sock);
  }

  pthread_mutex_unlock(&mutJoueur);
}

//TOFREE SAFE
char* getScores(joueur ** tab){
  char * res;
  pthread_mutex_lock(&mutJoueur);
  res = getScoresNotSafe(tab);
  pthread_mutex_unlock(&mutJoueur);
  return res;
}

//tofree
char* getScoresNotSafe(joueur ** tab){
  char * res;
  
  int i, w=0;
  if(!tab || !(*tab))
    return NULL;

  for(i=0; i< nbPlayer; i++){
    w+= strlen(tab[i]->nom);
    w+=4;
  }
  if(!w)
    return NULL;
  
  w*=2;
  res = malloc( w );
  memset(res, 0, w);
  sprintf(res, "%d",getTour());
  int tmp = strlen(res);
  for(i=0; i< nbPlayer; i++){
    if( (i+1)*tmp < w)
      sprintf(res+strlen(res), "*%s*%d", tab[i]->nom, tab[i]->score);
  }
  return res;
}

//SAFE
char addTrouveToPlayer(int sockId, char* mot, char * traj, joueur** tab){
  pthread_mutex_lock(&mutJoueur);
   int i;
   for(i=0; i< nbPlayer; i++){
     if( tab[i] && tab[i]->sock == sockId && (tab[i]->nbTrouve)< MAX_PLAYER_PROP){
      
       strncpy(tab[i]->mots[tab[i]->nbTrouve], mot, strlen(mot)<17?strlen(mot):16);
       strncpy(tab[i]->traj[tab[i]->nbTrouve], traj, strlen(traj)<17?strlen(traj):16);
       (tab[i]->nbTrouve)++;
       
 
       pthread_mutex_unlock(&mutJoueur);
       return 1;
     }
   }
   pthread_mutex_unlock(&mutJoueur);
   //Le joueur n'a pas ete trouve
   return 0;
}

void emptyPlayerProposition(joueur** tab){
  pthread_mutex_lock(&mutJoueur);
  emptyPlayerPropositionNotSafe(tab);
  pthread_mutex_unlock(&mutJoueur);
}

void emptyPlayerPropositionNotSafe(joueur** tab){
  int i,j;
  for(i=0, j=0; i< nbPlayer; i++, j=0){
    if( tab[i] && tab[i]->nbTrouve ){
      for(;j<tab[i]->nbTrouve;j++){
	memset( tab[i]->traj[j], 0, 16);
	memset( tab[i]->mots[j], 0, 16);
      }
      tab[i]->nbTrouve=0;
    }
  }
}

void resetPlayerScore(joueur** tab){
 int i;
  for(i=0; i< nbPlayer; i++){
    if(tab[i]){
      tab[i]->score = 0;
    }
  }
}


int strlenToScore(int length){
  switch(length){
		
  case 3 : return 1;
  case 4 : return 1;
  case 5 : return 2;
  case 6 : return 3;
  case 7 : return 5;
  case 8 : return 11;
  case 9 : return 11;
  case 10 : return 11;
  case 11 : return 11;
  case 12 : return 11;
  case 13 : return 11;
  case 14 : return 11;
  case 15 : return 11;
  case 16 : return 11;
  default : return 0;	      
  }
}

void setBadExit(joueur * j){
  if(j)
    j->badExit = 1;
}
