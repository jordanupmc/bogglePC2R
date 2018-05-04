#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "game.h"
#include "threadSafeList.h"
/*
void afficher(Entier * e){
  if(!(e))
    return;
  printf("%d\n",e->x);
  afficher(e->suiv);
}
*/
pthread_mutex_t mutList = PTHREAD_MUTEX_INITIALIZER;

ljoueur* addPlayertoPropose(ljoueur * l, joueur * j ){
  ljoueur* tmp;
  
  tmp = malloc( sizeof(ljoueur) );
  tmp->joueur = j;
  tmp->suiv =NULL;
  
  if( l )
    tmp->suiv = l;
  
  
  return tmp;
}
int sizeLJoueur(ljoueur *lj){
  if(!lj)
    return 0;
  return 1+sizeLJoueur(lj->suiv);
}
void addProposeNotSafe(nodePropose ** l,char * mot, joueur* j){
 nodePropose *tmp, *it;
 //pthread_mutex_lock(&mutList);
  //printf("WTF ADDPROPOSE %s\n",mot); OK
  if(!l || !*l){
    (*l)=malloc(sizeof(nodePropose));
    (*l)->players = addPlayertoPropose((*l)->players, j);
    strncpy((*l)->mot, mot, strlen(mot));
    
    (*l)->mot[strlen(mot)]='\0';
    printf("VRAIMENT ADD = %s mot =%s %d\n", (*l)->mot, mot, strlen(mot));
    (*l)->nbPlayer= 1;
    (*l)->suiv=NULL;
    //printf("OK addPropose\n");
    //  pthread_mutex_unlock(&mutList);
    return;
  }
  it = *l;
  int smot =strlen(mot);
  int cur;
  while(it){
    cur = strlen(it->mot);
    //2eme proposition d'un mot -> on ajoute le joueur a la liste
    if( smot == cur && !strncmp(it -> mot, mot, cur)  ){
      it->players = addPlayertoPropose(it->players, j);
      (it->nbPlayer)++;
      //(*l)->nbPlayer = 
      //printf("ADDPROPOSE nbPLay=%d %s\n", (*l)->nbPlayer, it->mot);
      //  pthread_mutex_unlock(&mutList);
      //printf("DOUBLON!\n");
      return;
    }
    it = it -> suiv;
  }
  tmp=malloc(sizeof(nodePropose));
  tmp->suiv=*l;
  *l=tmp;
  (*l)->players = addPlayertoPropose((*l)->players, j);
  ((*l)->nbPlayer)=1;
  strncpy((*l)->mot, mot, strlen(mot));
  (*l)->mot[strlen(mot)]='\0';
  printf("VRAIMENT ADD = %s mot=%s %d\n", (*l)->mot,mot , strlen(mot));
 
}

void addPropose(nodePropose ** l,char * mot, joueur* j){
  pthread_mutex_lock(&mutList);
  addProposeNotSafe(l,mot,j);
  pthread_mutex_unlock(&mutList);
}


int nb_Elem(nodePropose *l){
  if(!l)
    return 0;
  return nb_Elem(l->suiv)+1;
  //return cpt;
}

void detruirePlayers(ljoueur * l ){
   if(!l)
     return;
   ljoueur *tmp=l->suiv;
   free(l);
   detruirePlayers(tmp);
}
void detruire(nodePropose** l){
  nodePropose * tmp;
  nodePropose * tmp2;
  
  if((*l)==NULL)
    return;
 
  tmp=(*l)->suiv;
  detruirePlayers( (*l)->players);
  tmp2 = tmp;
  
  while( tmp2 ){
    tmp = tmp2->suiv;
    free(tmp2);
    tmp2=tmp;
  }
  *l=NULL;
}

char * allWords(nodePropose* l){
  if(!l)
    return NULL;
  nodePropose * it = l;
  int n =nb_Elem(it);
  
  char * res = (char*) malloc( (n *16)+16 );
  int offset = 0;
  
  while(it){
    if(it->nbPlayer > 1){
      it=it->suiv;
      continue;
    }
    //strncpy(res+offset, it->mot, 16);
    snprintf(res+offset,16, "%s", it->mot);
    //printf("wesh %s offset =%d res = %s res+offset=%s\n",it->mot, offset, res, res+offset);
    offset+=strlen(it->mot)+1;
    if(it->suiv)
      res[offset-1]= ALL_WORDS_DELIM;
    it = it->suiv;
  }
  //res[offset]='\0';

  printf("allWords : %s\n",res);
  return res;
}

/*TEST & SET*/
unsigned char containsMotThenAdd(nodePropose** l, char * mot, joueur* j){
  pthread_mutex_lock(&mutList);
  int slen = strlen(mot);
  unsigned char tmp = aux_containsMot( *l, mot, slen);
  if(!tmp)
    addProposeNotSafe(l, mot, j);
  pthread_mutex_unlock(&mutList);
  
  return tmp;
}

unsigned char aux_containsMot(nodePropose* l, char * mot, int slen){
  if(!l)
    return 0;
  int clen = strlen(l->mot);
  if( !strncmp(l->mot, mot, clen < slen? clen: slen) )
    return 1;
  return aux_containsMot(l->suiv, mot, slen);
}
