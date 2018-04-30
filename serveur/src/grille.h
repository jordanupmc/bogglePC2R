#include <pthread.h>
typedef struct point{
  char x;
  char y;
} point ;

extern char mAdjacence[16][16];

/*Return un tirage aleatoire*/
extern char * getTirage();

/*DEBUG: affiche un tirage*/
extern void debug_printTirage(char * t);

/*return la distance entre 2 points*/
extern int distance( point p, point q );

/*initialise la matrice mAdjacence*/
extern void createMatriceAdjacence();

/*check si une trajectoire est correcte
Valeur de retour
-0 precondition pas respecter:
 traj est NULL
 nbCase>16
 nbCase<=0
 traj[0]>=16
 traj[0]<0
-3 la trajectoire contient deux fois la même case
-2 la trajectoire contient deux cases qui ne sont pas adjacente
-1 la trajectoire est correcte
*/
extern unsigned char checkTrajectoire(int * traj, char nbCase);

/*Convertis un string en int, return -1 si le string ne peut pas être convertis*/
extern int safeStrtol( char * s );

/*Convertis une trajectoire venant du client en tableau de position (pour la matrice)
 exemple: A1A2A3B3 -> 0,1,2,6
 n etant le nombre de cases/position
 */
extern int * convertClientTrajectoireToPosition( char * trajArg, int n );

extern unsigned char checkTrajMatchMot(char * mot, int* pos, char * tirage);

/*check si un mot existe sur le serveur de mots
  sock etant la socket du serveur de mots
  Return
  -1 -> erreur
  2  -> pas de definition
  1  -> il existe une definition
 */
extern char checkMot(char * mot, int size, int sock);

extern  pthread_mutex_t mutDico;
