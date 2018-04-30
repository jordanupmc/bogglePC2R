#include "game.h"
#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#define ALL_WORDS_DELIM ' '
struct ljoueur;

typedef struct nodePropose{
  struct ljoueur * players; /*La liste des joueurs qui ont propose ce mot*/
  int nbPlayer; /*Le nombre de joueur qui ont propose ce mot*/
  char mot[17];
  struct nodePropose * suiv;
}nodePropose;

typedef struct ljoueur{
  joueur * joueur;
  struct ljoueur* suiv;
}ljoueur;

/*Fonction classique de manipulation de liste chaine*/
extern ljoueur* addPlayertoPropose(ljoueur * l, joueur * j );

extern void addPropose(nodePropose ** l,char * mot, joueur* j);

extern int nb_Elem(nodePropose *l);

extern void detruire(nodePropose** l);

extern char * allWords(nodePropose* l);
/***********************/

/*Test & Set*/
extern unsigned char containsMotThenAdd(nodePropose** l, char * mot, joueur* j);

extern unsigned char aux_containsMot(nodePropose* l, char * mot, int slen);
#endif
