#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "game.h"
#include "threadSafeList.h"


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

void addProposeNotSafe(nodePropose **l,char * mot, joueur* j){
  nodePropose *tmp, *it, *l2;
  l2 = *l;
  
  if(!l2){
    l2=malloc(sizeof(nodePropose));
    l2->players = addPlayertoPropose(l2->players, j);
    strncpy(l2->mot, mot, strlen(mot));
    
    l2->mot[strlen(mot)]='\0';
    //printf("VRAIMENT ADD = %s mot =%s %d\n", (*l)->mot, mot, strlen(mot));
    l2->nbPlayer= 1;
    l2->suiv=NULL;
    *l=l2;
    return;
  }
  it = l2;
  int smot =strlen(mot);
  int cur;
  while(it){
    cur = strlen(it->mot);
    //2eme proposition d'un mot -> on ajoute le joueur a la liste
    if( smot == cur && !strncmp(it -> mot, mot, cur)  ){
      (it->players) = addPlayertoPropose(it->players, j);
      (it->nbPlayer)++;
      //printf("ADDPROPOSE nbPLay=%d %s\n", (*l)->nbPlayer, it->mot);
      return;
    }
    it = it -> suiv;
  }
  tmp=malloc(sizeof(nodePropose));
  tmp->suiv=l2;
  l2=tmp;
  l2->players = addPlayertoPropose(l2->players, j);
  l2->nbPlayer=1;
  strncpy(l2->mot, mot, strlen(mot));
  l2->mot[strlen(mot)]='\0';
  *l=l2;
  //printf("VRAIMENT ADD = %s mot=%s %d\n", (*l)->mot,mot , strlen(mot));
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

void detruirePlayers(ljoueur *l ){
  if(!l)
    return;
  ljoueur *tmp;
  /* free(l);
  detruirePlayers(tmp);*/
  while(!l){
    tmp = l->suiv;
    free(l);
    l=tmp;
  }
  
}

void detruire(nodePropose** l){
  nodePropose * tmp;
  nodePropose * tmp2;
  
  if(! (*l))
    return;
 
  tmp=(*l)->suiv;
  detruirePlayers((*l)->players);
  (*l)->players = NULL;
  
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
    snprintf(res+offset,16, "%s", it->mot);
    offset+=strlen(it->mot)+1;

    if(it->suiv)
      res[offset-1]= ALL_WORDS_DELIM;
    it = it->suiv;
  }
  res[offset]='\0';
  
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
  int clen;
  nodePropose * tmp=l;
  
  while(tmp){
    clen = strlen(tmp->mot);
    if( clen == slen && !strcmp(tmp->mot, mot) )
      return 1;
    tmp = tmp->suiv;
  }
  
  return 0;
}
