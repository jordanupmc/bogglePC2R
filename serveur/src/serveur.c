#define _XOPEN_SOURCE 700
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <fnmatch.h>
#include <pthread.h>
#include <error.h>

#include <sys/stat.h>
#include <fcntl.h>

#include "serveur.h"
#include "grille.h"
#include "game.h"
#include "threadSafeList.h"
#include "synchro.h"
 
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condEmptyClient = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutSort = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condEmptySort = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutTime = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condBeginSession = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutChat = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condEmptyChat = PTHREAD_COND_INITIALIZER;

int nbTourSession;
nodePropose * proposition;
int immediat;
unsigned char randomGrille;
char ** grillesUser;
char * curTirage;

/*TODEF*/
void fillData(data * d, unsigned char type, char * nom, char* mot, char * trajectoire, int s){
  if(!d)
    return;

  d->type = type;
  d->fromSock = s;

  if(nom){
    //memset(d->nom, 0, MAX);
    strncpy(d->nom, nom, strlen(nom) < MAX ? strlen(nom): MAX-1);
  }
  if(mot){
    //memset(d->mot, 0, MAX);
    strncpy(d->mot, mot, strlen(mot)< MAX ? strlen(mot): MAX-1);
  }
  if(trajectoire){
    // memset(d->traj, 0, MAX_TRAJ);
    strncpy(d->traj, trajectoire, strlen(trajectoire)<MAX_TRAJ-1? strlen(trajectoire):MAX_TRAJ-1 );
  }

}
void fillDataChat(data * d,unsigned char type, char * msg, char * user, int s ){
  if(!d)
    return;

  d->type = type;
  d->fromSock = s;
  if(msg){
    //memset(d->msg, 0, MAX);
    strncpy(d->msg, msg, strlen(msg)< MAX? strlen(msg): MAX-1);
  }
  if(user){
    // memset(d->nom, 0, MAX);
    strncpy(d->nom, user, strlen(user) < MAX? strlen(user):MAX-1);
  }
 
}

void debug_printData(data * d){
  if(!d){
    printf("data is null\n");
    return;
  }
  printf("DATA: type=%d, ", d->type);
  if(d->nom)
    printf("nom=%s",d->nom);
  putchar('\n');
}


char isFormatCorrect(char * req, int size){
  if(!req)
    return 0;
  if( !fnmatch("CONNEXION/*/", req,FNM_NOESCAPE ) || !fnmatch("SORT/*/", req,FNM_NOESCAPE ) || !fnmatch("TROUVE/*/*/", req,FNM_NOESCAPE ) || !fnmatch("ENVOI/*/", req,FNM_NOESCAPE ) || !fnmatch("PENVOI/*/*/", req,FNM_NOESCAPE ) )
    return 1;

  return 0;
}


data * parseRequest(char * req, int size, int newSock ){
  if( !isFormatCorrect(req, size) ){
    printf("Ce format de requete n'est pas reconnu\n->%s\nErr SIZE=%d\n", req,size);
    return NULL;
  }

  //printf("SIZE REQ=%d\n", size);

  char * it = strtok(req, "/");
  data * res = malloc( sizeof(data) );
  int type=-1;
  unsigned char cpt =0;
  char user [MAX];
  char mot[MAX/2];
  char traj[MAX_TRAJ];
  char msg[MAX];
 
  memset(user, 0, MAX);  
  memset(msg, 0, MAX);
  memset(mot, 0, MAX/2);
  memset(traj, 0, MAX_TRAJ);

  while(it){    
    
    //printf("%s %d\n", it, strlen(it));
    if( !strcmp(it, "CONNEXION") && cpt == 0 ){
      type=0;
    }
    else if( !strcmp(it, "SORT") && cpt == 0){
      type=1;
    }
    else if( !strcmp(it, "TROUVE") && cpt == 0){
      type=2;
    }
    else if( !strcmp(it, "ENVOI") && cpt == 0 ){
      type=3;
    }
    else if( !strcmp(it, "PENVOI") && cpt == 0 ){
      type=4;
    }
    else if ( type <= 1 && cpt==1 ){
      //snprintf(user, strlen(it), "%s", it);
      strncpy(user, it, strlen(it) );
      fillData(res, type, user, NULL, NULL, newSock);
    }
    else if( type == 2 && cpt==1){
      strncpy(mot, it, strlen(it) );
    }
    else if( type ==2 && cpt==2){
      strncpy(traj, it, strlen(it) );
      fillData(res, type, NULL, mot, traj, newSock);
    }
    else if( type ==3 && cpt==1){
      strncpy(msg, it, strlen(it) );
      fillDataChat(res, type,msg,NULL, newSock);
    }
    else if( type ==4 && cpt==1){
      strncpy(user, it, strlen(it) );
    }
    else if( type ==4 && cpt==2){
      strncpy(msg, it, strlen(it) < MAX ? strlen(it): MAX-1 );
      fillDataChat(res, type,msg, user,newSock);
    }
    cpt++;
    it = strtok(NULL, "/");
  }

  if(!res)
    fprintf(stderr,"Error parseRequest: res is NULL\n");

  return res;
}
int containsNewLine(char * tab, int size){
  int i;
  for(i=0; i<size; i++)
    if( tab[i]=='\n' )
      return i;
  return 0;
}

int readInChan(int sock, char * buf, int size){
  memset(buf, 0, size);
  int r = read(sock, buf, 1);
  int tmp;
  if( r < 0 ){
    perror("Error readInChan 1");
    return 0;
  }

  while( r<size && buf[r-1] != '\n' ){
    tmp =read(sock, buf+r, 1);
    r+=tmp;
    if( tmp < 0 ){
      perror("Error readInChan 2");
      return 0;
    }
  }
  if(r > size)
    return 0;

  buf[r-1] = 0; 
  return r-1;
}
/*int readInChan(int sock, char * buf, int size){
  
  memset(buf, 0, size);
  int r = read(sock, buf, size);
  int tmp;
  if( r < 0 ){
    perror("Error readInChan 1");
    return 0;
  }

  int pos;
  while( !(pos=containsNewLine(buf, size)) && r<size ){
    tmp =read(sock, buf+r, size-r);
    r+=tmp;
    if( tmp < 0 ){
      perror("Error readInChan 2");
      return 0;
    }
  }
  if(r > size)
    return 0;

  buf[pos] = 0; 
  return pos-1;
}*/

void handleConnexionRequest(data * r, int sock, joueur** arr,  int * btime){
  if(!r)
    printf("Handle argument NULL\n");
  else{
    //printf("Enregistrement de %s\n", r->nom);
    
    //REVOIR LA SC//
    addNewJoueur(sock, r->nom, arr);
    
    /*SC**FOR TIMER*/
    pthread_mutex_lock(&mutTime);
    *btime = 1;
    pthread_cond_signal( &condBeginSession );
    pthread_mutex_unlock(&mutTime);
    /****/
    char * scores = getScores(arr);
    if(!scores)
      return;
    char* res = malloc(MAX_REP_BIENVENUE + strlen(scores));
    if(!res){
      free(scores);
      return;
    }
    //memset( res, 0, MAX_REP_BIENVENUE+ strlen(scores) );
    
    if(randomGrille)
      snprintf(res, MAX_REP_BIENVENUE+strlen(scores)-1, "BIENVENUE/%s/%s/\n", curTirage, scores);
    else
      snprintf(res, MAX_REP_BIENVENUE+strlen(scores)-1, "BIENVENUE/%s/%s/\n", grillesUser[getTour()], scores);
    debug_printAllPlayer(arr);

    //printf("ENVOI %s\n",res);

    //FIN SC
    // printf("%s %d\n",res,(int)strlen(res));
    if( ! (write( sock , (const void *)res, strlen(res)))){
      perror("Error write outchan server");
    }

    free(scores);
    scores=NULL;
    
    free(res);
    res=NULL;
    //SIGNAL A TOUT LES CLIENTS QU'UNE NOUVELLE CONNEXION A EU LIEU
    char tmpBroadcast[MAX/2];
    // memset( tmpBroadcast, 0, MAX/2 );
    snprintf(tmpBroadcast, (MAX/2)-1,"CONNECTE/%s/\n",r->nom);
    //DEBUT SC
    int i;
    for( i =0; i< getNbPlayer(); i++){
      if( arr[i] && !(arr[i]->badExit)  && arr[i]->sock != sock)
	if( !(write( arr[i]->sock , (const void *)tmpBroadcast, strlen(tmpBroadcast)))){
	  perror("Error Broadcast write outchan server");
	  setBadExit(arr[i]);
	}
    }
    //FIN SC

  }
}

void* job_pool(void* a){
  poolArg * arg = (poolArg *) a;
  int sock, i, nbR;
  char buf[MAX];
  data * res;

  while(1){
    /*SC*/
    pthread_mutex_lock(&mut);
    while( *(arg->nbWaiting) == 0 )
      pthread_cond_wait(&condEmptyClient, &mut);
    sock = arg->clients[0];
    /*Decalage -> suppression du premier element*/
    for(i=0; i< MAX_POOL_CONNEXION-1 ; i++)
      arg->clients[i] = arg->clients[i+1];
    (*(arg->nbWaiting))--;
    printf("client pris en charge par t%d waitingReq=%d\n", arg->threadID, *(arg->nbWaiting));
    pthread_mutex_unlock(&mut);
    /********SC*******/
    
    
    if( ! (nbR=readInChan(sock , buf, MAX )) ){//A revoir
      continue;
    }
    
    if( ! (res=parseRequest(buf, nbR-1, sock)) ){
      fprintf(stderr,"parseRequest fail !\n");
    }
    else if(res->type == 0){
      handleConnexionRequest(res, sock, arg->players, arg->boolTimer);
    }
    

    if(res)
      free(res);
    res = NULL;
  }

  pthread_exit(NULL);
  return NULL;
}

void writeToJournal(char * scores, int tour){
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  if(!scores)
    return;
  int lenS = strlen(scores)+70;
  
  char * buf = malloc( lenS );
  if(!buf)
    return;
  snprintf(buf, lenS-1,"\nS %d %d/%d/%d-%d:%d:%d\nU %s\nF\n", tour, tm.tm_mday, tm.tm_mon + 1,tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec,scores);
  int fd=open(JOURNAL_PATH, O_CREAT|O_APPEND|O_WRONLY, 00600);
  
  if(fd == -1){
    perror("Open fail ");
    free(buf);
    return;
  }
  
  if( !(write(fd , buf, lenS ))){
    perror("Error Broadcast write outchan server");
  }
  close(fd);
  
  free(buf);
}

void * job_Timer(void * arg){
  timerArg * ta = (timerArg *)arg;
 
  while(1){  
    pthread_mutex_lock(&mutTime);
    while( !(*(ta->bool)) ){
      pthread_cond_wait(&condBeginSession, &mutTime);
    } 
    
    *(ta->bool) = 0;
    pthread_mutex_unlock(&mutTime);
    //DEBUG time 10 sinon TIME_TOUR
    printf("Reflexion !\n");
    sleep(25);
    //Entrez en phase verification
    pthread_mutex_lock(&mutTime);
    *(ta->bool) = 2;
    pthread_mutex_unlock(&mutTime);

    pthread_mutex_lock(&mutJoueur);
    int i;
    for(i=0;i<getNbPlayer();i++){
      if( ta->players[i]->badExit || !(write( ta->players[i]->sock ,"RFIN/\n" , 6))){
	perror("Error Broadcast write outchan server");
	setBadExit(ta->players[i]);
      }
    }
    
    printf("VERIF\n"); 
    if(!immediat){
      pthread_mutex_lock(&mutVerif);
      (*(ta->nbVerif)) = getNbPlayer();
      (*(ta->nbVerif2)) = (*(ta->nbVerif));
      pthread_cond_broadcast(&condEmptyVerif);
      
      pthread_mutex_unlock(&mutVerif);

    //WAIT la verification 
      pthread_mutex_lock(&mutVerif);
      if( (*(ta->nbVerif)) != 0 )
	pthread_cond_wait(&condEndVerif, &mutVerif);
      pthread_mutex_unlock(&mutVerif);
    }
    
    //nodePropose * it= proposition;
    char * wordsProp = allWords(proposition); //TOFREE
    char * scores = getScoresNotSafe(ta->players);
    char *motsProp; 
    int sizeProp=0;
    
    if(!wordsProp && scores){
      sizeProp=strlen(scores)+15;
      motsProp= malloc(sizeProp);
      snprintf( motsProp, sizeProp-1, "BILANMOTS//%s/\n", scores);
    }
    else if(!wordsProp && !scores){
      sizeProp=15;
      motsProp= malloc(sizeProp);
      snprintf( motsProp, sizeProp-1, "BILANMOTS///\n");
    }
    else if(wordsProp && !scores){
      sizeProp = strlen(wordsProp)+15;
      motsProp= malloc(sizeProp);
      snprintf( motsProp, sizeProp-1, "BILANMOTS/%s//\n", wordsProp);
    }else{
      sizeProp= strlen(wordsProp)+strlen(scores)+15;
      motsProp= malloc(sizeProp);
      snprintf( motsProp, sizeProp-1, "BILANMOTS/%s/%s/\n", wordsProp, scores);
    }
    for(i=0; i<getNbPlayer(); i++)
      if( ta->players[i]->badExit || !(write(ta->players[i]->sock , motsProp, strlen(motsProp)< sizeProp? strlen(motsProp):sizeProp-1 ))){
	perror("Error Broadcast write outchan server");
	setBadExit(ta->players[i]);
      }
    
    detruire( &proposition );
    printf("BILAN  = %s\n", motsProp);
    if(motsProp)
      free(motsProp);
    motsProp=NULL;
    // puis incr le tour, si dernier tout -> fin session-> debut new session
  
    setTour(getTour()+1);
    if(getTour() == nbTourSession){
      printf("Fin de session !\n");
      char * finSess;
      int sizeFs;
      if(scores){
	sizeFs = strlen(scores)+14;
	finSess=malloc(sizeFs);
	snprintf( finSess, sizeFs-1, "VAINQUEUR/%s/\n", scores);
      }else{
	sizeFs = 14;
	finSess=malloc(sizeFs);
	snprintf( finSess, sizeFs-1, "VAINQUEUR//\n");
      }
      for(i=0; i<getNbPlayer(); i++)
	if( ta->players[i]->badExit || !(write(ta->players[i]->sock , finSess, strlen(finSess)< sizeFs ? strlen(finSess) : sizeFs-1 ))){
	  perror("Error Broadcast write outchan server");
	  setBadExit(ta->players[i]);
	}

      if(finSess)
	free(finSess);
      finSess=NULL;
      
      for(i=0; i<getNbPlayer(); i++)
	if( ta->players[i]->badExit || !(write(ta->players[i]->sock , "SESSION/\n", 9 ))){
	  perror("Error Broadcast write outchan server");
	  setBadExit(ta->players[i]);
	} 
      
      //journalisation
      writeToJournal(scores, getTour());
      
      //RESET TOUR
      setTour(0);   
      resetPlayerScore(ta->players);
    }

    emptyPlayerPropositionNotSafe(ta->players);

    sleep(10);
    if(randomGrille){
      free(curTirage);
      curTirage = getTirage();
    }
    else
      curTirage= grillesUser[getTour()];
    /*memset(motsProp, 0, strlen(motsProp));
     */
    char tourBuf[25];
    snprintf( tourBuf, 24, "TOUR/%s/\n", curTirage );
    printf("NEXT TOUR = %s\n", tourBuf);
    
    for(i=0; i<getNbPlayer(); i++)
      if(ta->players[i]->badExit || !(write(ta->players[i]->sock , tourBuf, strlen(tourBuf) ))){
	perror("Error Broadcast write outchan server"); 
	setBadExit(ta->players[i]);
      }
    pthread_mutex_unlock(&mutJoueur);//revoir SC
    if(wordsProp)
      free(wordsProp);
    wordsProp=NULL;
    if(scores)
      free(scores);
    scores =NULL;
  }
  pthread_exit(NULL);
  return NULL;
}

void* job_AddTrouveImmediat(void * arg){
  trouveArg* ta = (trouveArg*) arg;
  data * curr;
  int i;
  char buf[NB_des+11];
  char bufErr[MAX/2];
  unsigned char cTraj, match=0;
  joueur * player;

  while(1){  
    pthread_mutex_lock(&mutTrouve);
    while( (*(ta->nbTrouve)) == 0 )
      pthread_cond_wait(&condEmptyTrouve, &mutTrouve);
    
    curr = ta->tabReq[0];
    for(i=0; i< MAX-1 ; i++)
      ta->tabReq[i] = ta->tabReq[i+1];
    (*(ta->nbTrouve))--;
    pthread_mutex_unlock(&mutTrouve);
  
    pthread_mutex_lock(&mutTime);
    /*On est en phase de verification-> on jette la requete*/
    if( *(ta->timerStat) == 2 ){
      printf("Verif state ! req refuser\n");
      free(curr);
      pthread_mutex_unlock(&mutTime);
      continue;
    }
    pthread_mutex_unlock(&mutTime);
    
    //addTrouveToPlayer(curr->fromSock, curr->mot, curr->traj, ta->players);
    memset(buf,0, NB_des+10);
    memset(bufErr,0, MAX/2);
    int sizeMot = strlen(curr->mot);
    int nbCase= (int)(strlen(curr->traj)/2);

    player=getJoueurBySocket(curr->fromSock, ta->players);
    if(!player)
      continue;

    if(nbCase != sizeMot){
      printf("GRAVE: NB CASE DIFF de sizeMot ! %d %d\n",nbCase,sizeMot);
      
      snprintf( bufErr, (MAX/2)-1, "MINVALIDE/POS nombre case differente de la taille du mot/\n");
      if( !(write(curr->fromSock , bufErr, strlen(bufErr) ))){
	perror("Error Broadcast write outchan server"); 
      }
      continue;
    }

    int * pos = convertClientTrajectoireToPosition(curr->traj,
						   nbCase);
    if(pos){
      if( (cTraj=checkTrajectoire( pos, nbCase )) == 1 &&  (match=checkTrajMatchMot(curr->mot, pos, curTirage)) ==1 ){
	printf("Cool trajectoire correcte\nVerif dans dico...\n");
	 
	char cm = checkMot(curr->mot, sizeMot, ta->sockDico);
	if(cm == 1){
	  
	  if(containsMotThenAdd(&proposition, curr->mot, player)){
	    snprintf( bufErr, (MAX/2)-1, "MINVALIDE/PRI mot deja propose/\n");
	    if( !(write(curr->fromSock , bufErr, strlen(bufErr) ))){
	      perror("Error Broadcast write outchan server"); 
	      setBadExit(player);
	    }
	  }
	  else{
	    snprintf( buf, NB_des+10, "MVALIDE/%s/\n", curr->mot);
	    if( !(write(curr->fromSock , buf, strlen(buf) ))){
	      perror("Error Broadcast write outchan server");
	      setBadExit(player);
	    }
	    //addPropose(&proposition, curr->mot, player);
	    player->score+=strlenToScore(sizeMot);
	    printf("score %d\n", player->score);
	  }
	}
	else if(cm == 2){
	  snprintf( bufErr, (MAX/2)-1, "MINVALIDE/DIC/\n");
	  if( !(write(curr->fromSock , bufErr, strlen(bufErr) ))){
	    perror("Error Broadcast write outchan server");
	    setBadExit(player);
	  }
	}
	else{
	  fprintf(stderr, "Error serveur mots exit...");
	  exit(1);
	}
      }
      else if(cTraj == 2){
	snprintf( bufErr, (MAX/2)-1, "MINVALIDE/POS Trajectoires non adjacente/\n");
	if( !(write(curr->fromSock , bufErr, strlen(bufErr) ))){
	  perror("Error Broadcast write outchan server");
	  setBadExit(player);
	}
      }
      else if(cTraj == 3){
	snprintf( bufErr, (MAX/2)-1, "MINVALIDE/POS Case utilisé plusieurs fois/\n");
	if( !(write(curr->fromSock , bufErr, strlen(bufErr) ))){
	  perror("Error Broadcast write outchan server");
	  setBadExit(player);
	}
      }
      free(pos);
    }
    else {
      snprintf( bufErr, (MAX/2-1), "MINVALIDE/POS trajectoire invalide\n");
      if( !(write(curr->fromSock , bufErr, strlen(bufErr) ))){
	perror("Error Broadcast write outchan server");
	setBadExit(player);
      }
    }
  }
  free(curr);
  pthread_exit(NULL);
  return NULL;
}

void* job_AddTrouve(void * arg){
  trouveArg* ta = (trouveArg*) arg;
  data * curr=NULL;
  int i;
  while(1){  
    pthread_mutex_lock(&mutTrouve);
    while( (*(ta->nbTrouve)) == 0 )
      pthread_cond_wait(&condEmptyTrouve, &mutTrouve);
    
    //printf("TROUVE PASS!\n");
    curr = ta->tabReq[0];
    for(i=0; i< MAX-1 ; i++)
      ta->tabReq[i] = ta->tabReq[i+1];
    (*(ta->nbTrouve))--;
    pthread_mutex_unlock(&mutTrouve);
  
    pthread_mutex_lock(&mutTime);
    /*On est en phase de verification-> on jette la requete*/
    if( *(ta->timerStat) == 2 ){
      printf("Verif state ! req refuser\n");
      if(curr)
	free(curr);
      curr=NULL;
      pthread_mutex_unlock(&mutTime);
      continue;
    }
    pthread_mutex_unlock(&mutTime);
    if(curr)
    addTrouveToPlayer(curr->fromSock, curr->mot, curr->traj, ta->players);
    if(curr)
      free(curr);
    curr=NULL;
  }
  pthread_exit(NULL);
  return NULL;
}


void reduceScore(){
  //reduce score
  nodePropose * it= proposition;
  while( it ){
    ljoueur * itj = it->players;
    if(it->nbPlayer > 1){
      while(itj){
	itj->joueur->score =  itj->joueur->score - strlenToScore(strlen( it->mot ));
	itj->joueur->score = itj->joueur->score < 0 ? 0: itj->joueur->score;
	printf("RS score = %d\n",itj->joueur->score);
	itj = itj->suiv;
      }
      
    }
    it = it->suiv;
  }

}

void* job_Chat(void * arg){
  chatArg* ca = (chatArg*)arg;
  data * curr;
  int i;
  char buf[MAX];
  joueur * dest;
  while(1){
    memset(buf,0, MAX);
    pthread_mutex_lock(&mutChat);
    while( *(ca->nbChat) == 0 )
      pthread_cond_wait(&condEmptyChat, &mutChat);
    
    curr = ca->tabReq[0];
    for(i=0; i< MAX_POOL_CHAT-1 ; i++)
      ca->tabReq[i] = ca->tabReq[i+1];
    (*(ca->nbChat))--;
    pthread_mutex_unlock(&mutChat);
   
    if(curr->type == 3){
      snprintf(buf, MAX-1, "RECEPTION/%s/\n", curr->msg);
      for(i=0; i<getNbPlayer(); i++)
	if( !(write(ca->players[i]->sock , buf, strlen(buf) ))){
	  perror("Error Broadcast write outchan server");
	  setBadExit(ca->players[i]);
	}
    }
    else{
      snprintf(buf, MAX-1, "PRECEPTION/%s/%s/\n", curr->msg, curr->author);
      printf("buf = %s go\n", buf);
      dest = getJoueurByNameNotSafe(curr->nom, ca->players);
      if( !dest || !(write(dest->sock , buf, strlen(buf) ))){
	perror("Error Broadcast write ou le dest n'existe pas");
	setBadExit(dest);
      }
    }
  } 
  pthread_exit(NULL);
  return NULL;
}

void* job_Verif(void * arg){
  verifArg* va = (verifArg*)arg;
  int tmp;
  int i;
  int nbCase;
  int sizeMot;
  char buf[NB_des+11];
  char bufErr[MAX/2];
  unsigned char cTraj, match=0;
  
  while(1){
   
    pthread_mutex_lock(&mutVerif);
    while( *(va->nbVerif) == 0 )
      pthread_cond_wait(&condEmptyVerif, &mutVerif);
    if( (*(va->nbVerif)) == (*(va->nbVerif2))){
      (*(va->cptJob))=0;
      printf("INIT verif2\n");
    }
    tmp= --(*(va->nbVerif));
    pthread_mutex_unlock(&mutVerif);
    
    for(i=0; va->players[tmp] && i< va->players[tmp]->nbTrouve; i++){
      
      memset(buf,0, NB_des+10);
      memset(bufErr,0, MAX/2);
      sizeMot = strlen(va->players[tmp]->mots[i]);
      nbCase= (int)((strlen(va->players[tmp]->traj[i]))/2);
      /*Mauvais client*/

      if(nbCase != sizeMot){
	fprintf(stderr,"GRAVE: NB CASE DIFF de sizeMot ! %d %d\n",nbCase,sizeMot);
	
	snprintf( bufErr, (MAX/2)-1, "MINVALIDE/POS nombre case differente de la taille du mot/\n");
	if( !(write(va->players[tmp]->sock , bufErr, strlen(bufErr) ))){
	  perror("Error Broadcast write outchan server"); 
	  setBadExit(va->players[tmp]);
	}
	
	continue;
      }
      //Le dico contient uniquement des mots de taille x, 3 <=  x <= 16
      //donc si on depasse on a une erreur DIC pour x < 3 et POS pour  x < 16 (on revient sur une case)

      int * pos = convertClientTrajectoireToPosition(va->players[tmp]->traj[i],
						     nbCase);

      if(pos){
	//printf("nbCASE =%d %s\n",nbCase, va->players[tmp]->traj[i]);
	if( (cTraj=checkTrajectoire( pos, nbCase )) == 1 && (match=checkTrajMatchMot(va->players[tmp]->mots[i], pos, curTirage))==1 ){
	 
	  char cm = checkMot(va->players[tmp]->mots[i], sizeMot, va->sockDico);
	  if(cm == 1){
	  
	    snprintf( buf, NB_des+10, "MVALIDE/%s/\n", va->players[tmp]->mots[i]);
	   
	    
	    if( !(write(va->players[tmp]->sock , buf, strlen(buf) ))){
	      perror("Error Broadcast write outchan server");
	      setBadExit(va->players[tmp]);
	    }
	    else{
	      //SCORE++
	      addPropose(&proposition, va->players[tmp]->mots[i], va->players[tmp]);
	      va->players[tmp]->score+=strlenToScore(sizeMot);
	      printf("Propose score %d\n",va->players[tmp]->score);
	    }
	  }
	  //Le mot n'existe pas dans le dico
	  else if(cm == 2){
	    snprintf( bufErr, (MAX/2)-1, "MINVALIDE/DIC/\n");
	    if( !(write(va->players[tmp]->sock , bufErr, strlen(bufErr) ))){
	      perror("Error Broadcast write outchan server");
	      setBadExit(va->players[tmp]);
	    }
	  }
	  //La connexion avec le serveur de mot a échoué on ne peut pas continuer
	  else{
	    fprintf(stderr, "Error serveur mots exit...");
	    exit(1);
	  }
	}
	else if(cTraj == 2){
	  snprintf( bufErr, (MAX/2)-1, "MINVALIDE/POS Trajectoires non adjacente/\n");
	  if( !(write(va->players[tmp]->sock , bufErr, strlen(bufErr) ))){
	    perror("Error Broadcast write outchan server");
	    setBadExit(va->players[tmp]);
	  }
	}
	else if(cTraj == 3){
	  snprintf( bufErr, (MAX/2)-1, "MINVALIDE/POS Case utilisé plusieurs fois/\n");
	  if( !(write(va->players[tmp]->sock , bufErr, strlen(bufErr) ))){
	    perror("Error Broadcast write outchan server");
	    setBadExit(va->players[tmp]);
	  }
	}
	else if(!match){
	  snprintf( bufErr, (MAX/2)-1, "MINVALIDE/POS le mot ne match pas avec la grille courante/\n");
	  if( !(write(va->players[tmp]->sock , bufErr, strlen(bufErr) ))){
	    perror("Error Broadcast write outchan server");
	    setBadExit(va->players[tmp]);
	  }
	}
	if(pos)
	  free(pos);
	pos=NULL;
      }
      else {
	snprintf( bufErr, (MAX/2)-1, "MINVALIDE/POS trajectoire invalide\n");
	if( !(write(va->players[tmp]->sock , bufErr, strlen(bufErr) ))){
	  perror("Error Broadcast write outchan server");
	  setBadExit(va->players[tmp]);
	}
      }
    }
    pthread_mutex_lock(&mutVerif);
    (*(va->cptJob))++;

    //Si je suis le dernier thread je reduit les scores et je debloque le timer
    if( (*(va->cptJob)) == (*(va->nbVerif2))){
      reduceScore();
      (*(va->cptJob))=0;
      pthread_cond_broadcast(&condEndVerif);
    }
    pthread_mutex_unlock(&mutVerif);
  }
  
  pthread_exit(NULL);
  return NULL;
}

int connectWithWordsServer(){
  int sockDico = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr;
  if( sockDico < 0){
    perror("connectWithWordsServer fail socket");
    return sockDico;
  }
  memset(&addr,0 , sizeof(addr));
  addr.sin_port=htons(PORT_DICO);
  addr.sin_family=AF_INET;
  addr.sin_addr.s_addr=inet_addr("127.0.0.1") ;
  if(connect(sockDico, (struct sockaddr *)&addr, sizeof(addr))<0){
    perror("connectWithWordsServer fail ");
    return -1;
  }
  return sockDico;
}

void getOption(int argc, char** argv, int * port, int * tour, int * immediat){
  int i;
  char portOpt =0;
  char tourOpt =0;
  char immOpt = 0;
  char grillesOpt = 0;

  for(i=1; i<argc ;i++){
    if( !strcmp(argv[i], "-port")){
      if( argc >= i+1 ){
	*port = safeStrtol(argv[i+1]);
	if( (*port) < 0){
	  printf("Option -port prend un entier en arg\n");
	  return;
	}
	i++;
	portOpt=1;
      }
      else{
	printf("Option -port prend un entier en arg\n");
	return;
      }
    }
    else if(!strcmp(argv[i], "-immediat") ){
      *immediat = 1;
      immOpt = 1;
    }
    else if(!strcmp(argv[i], "-tour") ){
      if( argc >= i+1 ){
	*tour = safeStrtol(argv[i+1]);
	if( (*tour) <= 0){
	  printf("Option -tour prend un entier positif en arg\n");
	  return;
	}
	i++;
	tourOpt =1;
      }
      else{
	printf("Option -tour prend un entier en arg\n");
	return;
      }
    }
    else if(!strcmp(argv[i], "-grilles") ){
      if(!tourOpt || argc < i+ (*tour)){
	fprintf(stderr, "-grilles bad format: precisez le nombre de tour avant cette option\n");
	return;
      }
      grillesUser = malloc( sizeof(char*) * (*tour));
      int x;
      for(x=0;x< (*tour);x++)
	grillesUser[x]=strdup(argv[i+1+x]);
      randomGrille=0;
      grillesOpt = 1;
    }
  }
  
  if(!portOpt)
    (*port) = PORT_DEFAULT;
  if(!tourOpt)
    (*tour) = NB_TOUR_DEFAULT;
  if(!immOpt)
    (*immediat) = 0;
  if(!grillesOpt)
    randomGrille=1;
    
}

int main(int argc, char** argv){

  int port; 
  getOption(argc, argv, &port, &nbTourSession, &immediat);

  struct sockaddr_in addr;
  int s;
 
  /*CLIENT*/
  struct sockaddr_in client;
  int clientSize = sizeof(client);
 
  srand(time(NULL));
  
  addr.sin_family= AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);


  s=socket(AF_INET, SOCK_STREAM, 0);

  printf("===Server listen on port %d===\n", port);
  if(immediat)
    printf("===Option immediat active===\n");

  printf("===START web server if you need results: localhost:8888/cgi.py===\n");

  if(s< 0){
    perror("SOCKET OPEN ");
    return -1;
  }
  int enable = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
    perror("setsockopt(SO_REUSEADDR) failed");
    return -1;
  }

  if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) <0){
    perror("Bind");
    return -1;
  }

  if(listen(s, MAX_CLIENT)<0){
    perror("listen");
    return -1;
  }
  
  printf("Connect with words server...\n");
  int sockWords = connectWithWordsServer();
  if( sockWords < 0 )
    return -1;
  
  char buf[MAX];
  int nbR;
  int sockClient;
  pthread_t pool[MAX_POOL_CONNEXION];
 
  pthread_t threadTimer;
  pthread_t threadTrouve[MAX_POOL_TROUVE];
  pthread_t threadVerif[MAX_POOL_VERIF];
  pthread_t threadChat[MAX_POOL_CHAT];
  int sockets[MAX_CLIENT];
  int i, nbWaitingSocks = 0, nbVerif=0, nbChat=0;
  int nbVerif2=0;
  poolArg arg[MAX_POOL_CONNEXION];
  
  joueur ** arr= initJoueurArray();
  if(!arr){
    perror("initJoueurArray");
    return -1;
  }
  if(randomGrille)
    curTirage = getTirage();
  else 
  curTirage = grillesUser[0];
  
  char tmpBroadcast [MAX];
  int btime=0;

  data * reqTrouve[MAX];
  data * reqChat[MAX/4];

  int nbTrouve =0;

  printf("Init pool...");
  
  /*ADJACENCE INIT*/
  createMatriceAdjacence();
  
  for(i=0; i< MAX_POOL_CONNEXION; i++){
    arg[i].nbWaiting = &nbWaitingSocks;
    arg[i].clients = sockets;
    arg[i].threadID = i;
    arg[i].players = arr;
    arg[i].boolTimer= &btime;
    pthread_create(pool+i, NULL, job_pool, (void *)(arg+i));
  }
  
  /*TIMER*/
  timerArg ta= { &btime, &nbVerif, &nbVerif2, arr };
  pthread_create(&threadTimer, NULL, job_Timer, &ta);

  trouveArg tra[MAX_POOL_TROUVE];
  for(i=0; i< MAX_POOL_TROUVE;i++){
    tra[i].nbTrouve=&nbTrouve;
    tra[i].players=arr;
    tra[i].tabReq = reqTrouve;
    tra[i].timerStat = &btime;
    tra[i].sockDico=sockWords;
    if(!immediat)
      pthread_create(threadTrouve+i, NULL, job_AddTrouve, tra+i);
    else
      pthread_create(threadTrouve+i, NULL, job_AddTrouveImmediat, tra+i);
  }


  int cptJobVerif=0;
  verifArg varg = { &nbVerif, &nbVerif2, &cptJobVerif ,arr,sockWords };

  if(!immediat)
    for(i=0; i< MAX_POOL_VERIF;i++)
      pthread_create(threadVerif+i, NULL, job_Verif, &varg);
  
  chatArg charg = { arr, reqChat, &nbChat };
  for(i=0; i< MAX_POOL_CHAT;i++)
    pthread_create(threadChat+i, NULL, job_Chat, &charg);

  /*MULTIPLEXAGE*/  
  fd_set rfds;
  int retval;
  int maxFD=s;
  
  FD_ZERO(&rfds);
  
  while(1){
    
    /*SC*/
    // FD_ZERO(&rfds);
    FD_SET(s, &rfds);
    for(i=0; i<getNbPlayer(); i++){
      //  printf("ADD %d\n",arr[i]->sock);
      if( arr[i] && !(arr[i]->badExit) )
	FD_SET(arr[i]->sock, &rfds);
    }

    //printf("Serveur en attente...");
    //fflush(stdout);
    retval = select(maxFD+1, &rfds, NULL, NULL, NULL);
    if (retval == -1){
      perror("select()");
      FD_ZERO(&rfds);
      FD_SET(s,&rfds);
    }
    else if (retval){
      //printf("Data is available now.\n");
      if( FD_ISSET(s, &rfds ) ){
	if( !(sockClient=accept(s, (struct sockaddr *) &client, (socklen_t *)&clientSize ) )){
	  fprintf(stderr, "Error accept\n");
	  //exit(1);
	}
	else{
	  /* SC */
	  pthread_mutex_lock(&mut);
	  if( nbWaitingSocks < MAX_CLIENT ){
	    sockets[ nbWaitingSocks++ ] = sockClient;
	    pthread_cond_signal( &condEmptyClient );
	    maxFD = maxFD < sockClient ? sockClient: maxFD;
	    printf("Nouvelle connexion au serveur.%d\n",maxFD);
	    FD_SET(sockClient, &rfds);
	  }
	  //maxFD = maxFD < sockClient ? sockClient: maxFD;
	  pthread_mutex_unlock(&mut);
	  /****SC*****/
	}

      }
      else{
	for(i=0; i<getNbPlayer(); i++)
	  if( arr[i] && FD_ISSET(arr[i]->sock, &rfds) ){
	    /*DEBLOQ LA POOL DE REPONSE*/
	    //printf("DEBUG DECO i=%d nbP=%d\n",i, getNbPlayer());
	    //memset(buf,0,MAX);
	    if( ! (nbR=readInChan(arr[i]->sock , buf, MAX )) )
	      continue;
	    data *d  = parseRequest(buf, nbR, arr[i]->sock);

	    if(!d)
	      continue;
	    
	    //DECONNEXION "obliger" de le faire en synchrone
	    if(d->type == 1){ 
	      //printf("BROADCAST deco!\n");
	      char * cp = strdup(arr[i]->nom);
	      int tmpSock = arr[i]->sock;
	      removeJoueurBySocket(tmpSock , arr);
	      
	      //memset(tmpBroadcast,0 ,strlen(cp)+20);
	      
	      snprintf( tmpBroadcast, strlen(cp)+20-1, "DECONNEXION/%s/\n", cp);
	      int j;
	      for( j =0; j< getNbPlayer(); j++){
		//printf("Je broadcast %s!\n",tmpBroadcast);
		if( !(write( arr[j]->sock , (const void *)tmpBroadcast, strlen(tmpBroadcast)))){
		  perror("Error Broadcast write outchan server");
		  setBadExit(arr[j]);
		}
		maxFD = maxFD < arr[j]->sock ? arr[j]->sock: maxFD;
	      }
	      if(getNbPlayer() == 0)
		maxFD = s;
	      FD_CLR(tmpSock, &rfds);
	      free(cp);
	      free(d);
	    }
	    else if( d->type == 2){
	      //printf("Je rentre ici\n");
	      pthread_mutex_lock(&mutTrouve);
	      if( nbTrouve < MAX ){
		reqTrouve[nbTrouve++]=d;
		pthread_cond_signal( &condEmptyTrouve );
	      }
	      pthread_mutex_unlock(&mutTrouve);
	    }
	    else if(d->type == 3 || d->type == 4){
	      pthread_mutex_lock(&mutChat);
	      if( d->type == 4){
		joueur * author = getJoueurBySocketNotSafe(d->fromSock, arr);
		if( author ){	  
		  memset(d->author, 0, MAX);
		  strncpy(d->author, author->nom, strlen(author->nom)< MAX-1? strlen(author->nom): MAX-1 );
		  printf("getJoueur %s data =%s \n",author->nom, d->author);
		}
	      }
	      reqChat[nbChat++]=d;
	      pthread_cond_signal( &condEmptyChat );
	      pthread_mutex_unlock(&mutChat);
	    }
	   
	  }
      }
    }

  }
  
  shutdown(s, 2);
  close(s);
  
  return 0;
}
