#define _XOPEN_SOURCE 700
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>
#include <string.h>
#include <fnmatch.h>
#include <math.h> 
#include <errno.h>
#include <limits.h>

#include "serveur.h"
#include "grille.h"
unsigned char checkTrajMatchMotCP(char * mot, int* pos, char * tirage){
  if( !mot || !pos || !tirage)
    return 0;
  
  int i;

  for(i=0; i<strlen(mot); i++)
    if( pos[i]<0 || pos[i]>15 || mot[i] != tirage[pos[i]] ){
      printf("%d %c %c\n",pos[i], mot[i], tirage[pos[i]] );
      return 0;
    }
  printf("OK\n");
  return 1;
}

int main(int argc, char** argv){
  srand(time(NULL));
  char * t = getTirage();
  debug_printTirage(t);

  
  createMatriceAdjacence();
  char traj[] = { "A1B1B2B3B4C4D4/" };
  int i, nbCase;
  
  for(i=0; traj[i]!= '/'; i++);
  i/=2;
  nbCase = i;
  
  //tab contient la trajectoire sous forme d'entier
  int * tab = convertClientTrajectoireToPosition( traj, i ); 
  if( ! tab )
    printf("Mince \n");
  {
    int j;
    for(j=0; j< nbCase; j++)
      printf("tab[%d] = %d\n", j,tab[j]);
  };
  //printf("NB CASE = %d \n", i);
  if( checkTrajectoire( tab, nbCase) == 1 ){
    printf("Cool trajectoire correcte\nSelection is :");
    for( i = 0; i < nbCase; i++){
      printf("%c", t[tab[i]]);
    }
    putchar('\n');
  }
 
  free(tab);
  free(t);
  return 0;
}
