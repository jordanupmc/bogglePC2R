#include <pthread.h>
#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED
#define MAX_CONNECTED_PLAYER 10
#define TIME_TOUR 60 * 3
#define MAX_PLAYER_PROP 1024
#define MAX_NAME 100
#define MAX_MOT 18

extern int getTour();
extern int getSession();

extern void setTour(int x);
extern void setSession(int x);

typedef struct joueur{
  int sock;
  char nom[MAX_NAME];
  int score;
  int nbTrouve;
  char traj[MAX_PLAYER_PROP][MAX_MOT];
  char mots[MAX_PLAYER_PROP][MAX_MOT];
  char badExit;
} joueur;

/*Return un joueur a partir de son nom SAFE*/
extern joueur* getJoueurByName(char * name,joueur** tab);

/* *FIX bizarre* Return un joueur a partir de son numero SAFE*/
extern joueur* getJoueurByNum(int x,joueur** tab);

/*return un joueur par son nom sans prendre de mutex*/
extern joueur* getJoueurByNameNotSafe(char * name,joueur** tab);

extern joueur* getJoueurByNumNotSafe(int x, joueur** tab);

/*Return un joueur par son num de Socket SAFE*/
extern joueur* getJoueurBySocket(int sock, joueur** tab);

extern joueur* getJoueurBySocketNotSafe(int sock, joueur** tab);

/*FIX*/
extern void addJoueur(joueur* j, joueur** tab );

/*Ajoute un joueur connecte*/
extern void addNewJoueur(int sock, char* nom, joueur** tab );

/*Change la valeur du champs badExit d'un joueur*/
extern void setBadExit(joueur *j);

//SAFE
extern void removeJoueur(joueur * j,joueur** tab);
//SAFE
extern void removeJoueurByName(char * name,joueur** tab);
//SAFE
/*supprime un joueur par son num de Socket*/
extern void removeJoueurBySocket(int sock,joueur** tab);

/*Ajoute un mot trouve par un joueur a son tableau de mot et la trajectoire
associe*/
extern char addTrouveToPlayer(int sockId, char* mot, char * traj, joueur** tab);

extern void debug_printAllPlayer(joueur ** tab);

/*Initialise le tableau des joueurs */
extern joueur** initJoueurArray();

//TOFREE
//SAFE
/*A partir du tableau de joueur return les scores actuel au format du protocole*/
extern char* getScores(joueur ** tab);

/*Return le nombre de joueur*/
extern int getNbPlayer();


extern void * job_AddTrouve(void * arg);

/*Vide les propositions de tout les joueurs*/
extern void emptyPlayerProposition(joueur** tab);

/*Remet a 0 le score de tout les joueurs*/
extern void resetPlayerScore(joueur** tab);

//TOFREE
//return getScores mais pas thread-safe
extern char *getScoresNotSafe(joueur** tab); 

extern void emptyPlayerPropositionNotSafe(joueur** tab);

/*Return le score associe a length (la taille d'un mot)*/
extern int strlenToScore(int length);
#endif
