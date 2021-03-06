#include "grille.h"
#include <math.h> 
#include "serveur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/socket.h>


char mAdjacence[16][16];

/*Les des en versions international
  On crée une fonction pour eviter d'avoir une variable globale
*/
char getEngDes(int i, int j){
  static char des [16][6]={
    { 'E', 'T', 'U', 'K', 'N', 'O'  },
    { 'E', 'V', 'G', 'T', 'I', 'N' },
    { 'D', 'E', 'C', 'A', 'M', 'P' },
    { 'I', 'E', 'L', 'R', 'U', 'W' },
    { 'E', 'H', 'I', 'F', 'S', 'E' },
    { 'R', 'E', 'C', 'A', 'L', 'S' },
    { 'E', 'N', 'T', 'D', 'O', 'S' },
    { 'O', 'F', 'X', 'R', 'I', 'A' },
    { 'N', 'A', 'V', 'E', 'D', 'Z' },
    { 'E', 'I', 'O', 'A', 'T', 'A' },
    { 'G', 'L', 'E', 'N', 'Y', 'U' },
    { 'B', 'M', 'A', 'Q', 'J', 'O' },
    { 'T', 'L', 'I', 'B', 'R', 'A' },
    { 'S', 'P', 'U', 'L', 'T', 'E' },
    { 'A', 'I', 'M', 'S', 'O', 'R' },
    { 'E', 'N', 'H', 'R', 'I', 'S' }
  };
  return des[i][j];  
}
/*TO FREE*/
char * getTirage(){
  int face = rand()%NB_rand;
  char * res = malloc( NB_des+1 );
  int i;

  for(i=0; i< NB_des; i++){
    res[i] = getEngDes(i, face );
    face = rand()%NB_rand;
  }
  res[NB_des]='\0';
  return res;
}
void debug_printTirage(char * t){
  int i;
  for(i=0; i< NB_des; i++){
  
      printf("%c, ", t[i]);
      if( i> 0 &&  (i+1)%4 == 0)
	printf("\n");
  }
  putchar('\n');
}

int distance( point p, point q ){
  
  char s = (q.x - p.x)*(q.x - p.x) + (q.y - p.y)*(q.y - p.y);
  s = sqrt(s);
  return (int)floor(s);
}


void createMatriceAdjacence(){
  
  static point points[16] = { {0,0},{0,1}, {0,2},{0,3},
		       {1,0},{1,1}, {1,2},{1,3},
		       {2,0},{2,1}, {2,2},{2,3},
		       {3,0},{3,1}, {3,2},{3,3}};

  int i,j;

  for(i=0; i<16; i++){
    for(j=0; j<16; j++){
      mAdjacence[i][j]=0;
    }
  }
  
  for(i=0; i<16; i++){
    for(j=0; j<16; j++){
      if( (points[i].x != points[j].x ||  points[i].y != points[j].y) && 
	   distance(points[i], points[j]) == 1 )
	mAdjacence[i][j]=1;
      else
	mAdjacence[i][j] =0;
    }
  }
		       
}


unsigned char checkTrajectoire(int * traj, char nbCase){
  if(! traj || nbCase> 16 || nbCase<= 0 || traj[0]>=16 || traj[0]<0 ){
    printf("Precondition pas respecter checkTrajectoire\n");
    return 0;
  } 
  int i, j;
  for(i=0; i< nbCase; i++){
    for(j=0; j<nbCase; j++){
      if(i != j && traj[i] == traj[j]){
	printf("Precondition pas respecter: trajectoire connexe at checkTrajectoire\n");
	return 3;
      }
    }
  }
  
  int tmp = traj[0];
  for(i=1; i< nbCase; i++){
    if( traj[i] <0 || traj[i] >= 16 || !mAdjacence[tmp][traj[i]] ){// si non ADJACENT
      printf("Trajectoires non adjacente %d->%d\n", tmp, traj[i]);
      return 2;
    }
    tmp = traj[i];
  }

  //SUCCESS
  return 1;
}

int safeStrtol( char * s ){
  char * endptr;

  int val = strtol(s, &endptr, 10);
  
  errno = 0;
  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
      || (errno != 0 && val == 0)) {
    perror("strtol");
    return -1;
  }

  if (endptr == s) {
    fprintf(stderr, "Pas un entier\n");
    return -1;
  }
  
  return val;
}

//TO FREE
int * convertClientTrajectoireToPosition( char * trajArg, int n ){
  if(!trajArg || n<=0)
    return NULL;
  
  int i, j, control=0;
  char offset=-1;
  int * tab = malloc( sizeof(int)*n );
  char ** traj = malloc( sizeof( char*) * n);
  
  for(i=0; i< n; i++)
    traj[i] = malloc( 2 );
  
  for(i = 0, j=0; i< n; i++, j+=2){
    traj[i][0] = trajArg[j];
    control = safeStrtol( trajArg+(j+1) );
    if(control == -1 )
      break;
    traj[i][1] = control;
  }
  
  if(control == -1 || control > 4 || control < 1){

    free(tab);
    for(i = 0; i< n; i++ )
      free(traj[i]);  
    free(traj);
    
    return NULL;
  }

  if(!tab){
    perror("malloc at convertUserTrajectoireToPosition ");
    return NULL;
  }

  for(i=0; i< n; i++){

    switch (traj[i][0]){
    case 'A':
      offset = 0;
      break;
    case 'B':
      offset = 4;
      break;
    case 'C':
      offset = 8;
      break;
    case 'D':
      offset = 12;
      break;
    default :
      offset = -1;
    }
    if( offset < 0 ){
      printf("Position %c inconnu\n", traj[i][0]);
      free(tab);

      for(i = 0; i< n; i++ )
	free(traj[i]);  
      free(traj);

      return NULL;
    }
    tab[i] = offset + traj[i][1]-1;
  }

  
  for(i = 0; i< n; i++ )
    free(traj[i]);  
  free(traj);

  return tab;
}

/*check si un mot former a partir d'une trajectoire correspond a la grille courante
 0-> ne match pas
 1-> match
*/
unsigned char checkTrajMatchMot(char * mot, int* pos, char * tirage){
  if( !mot || !pos || !tirage)
    return 0;
  
  int i;
  for(i=0; i<strlen(mot); i++)
    if( pos[i]<0 || pos[i]>15 || mot[i] != tirage[pos[i]] )
      return 0;
  
  return 1;
}

/*check si un mot a une definition dans le serveur de mot
  -1 -> erreur
  2  -> pas de definition
  1  -> il existe une definition
*/

char checkMot(char * mot, int size, int sock){
  char nbR;
  char buf[MAX];
  char res[MAX];

  snprintf(buf, MAX-1, "CHECK %s\n",mot);


  if( ! (write( sock , (const void *)buf, strlen(buf)))){
    perror("Error write outchan server");
    shutdown(sock, 2);
    close(sock);
    return -1;
  }

  if( ! (nbR=(char)readInChan(sock , res, 3 )) ){
    shutdown(sock, 2);
    close(sock);
    return nbR;
  }

  shutdown(sock, 2);
  close(sock);

  if( !strncmp(res, "KO",2) )
    return 2;
  
  else if( !strncmp(res, "OK",2) )
    return 1;
 
  return 3;
}

