
#ifndef SYNCHRO_H_INCLUDED
#define SYNCHRO_H_INCLUDED
extern pthread_mutex_t mut;//= PTHREAD_MUTEX_INITIALIZER;
extern pthread_cond_t condEmptyClient; //= PTHREAD_COND_INITIALIZER;

extern pthread_mutex_t mutSort;// = PTHREAD_MUTEX_INITIALIZER;
extern pthread_cond_t condEmptySort; //= PTHREAD_COND_INITIALIZER;

extern pthread_mutex_t mutTime;// = PTHREAD_MUTEX_INITIALIZER;
extern pthread_cond_t condBeginSession; //= PTHREAD_COND_INITIALIZER;

extern pthread_mutex_t mutJoueur;// = PTHREAD_MUTEX_INITIALIZER;
extern pthread_cond_t condEmptyJoueur;// = PTHREAD_COND_INITIALIZER;

extern pthread_mutex_t mutTrouve ;//= PTHREAD_MUTEX_INITIALIZER;
extern pthread_cond_t condEmptyTrouve;// = PTHREAD_COND_INITIALIZER;

extern pthread_mutex_t mutChat;//= PTHREAD_MUTEX_INITIALIZER;
extern pthread_cond_t condEmptyChat;// = PTHREAD_COND_INITIALIZER;

extern pthread_mutex_t mutVerif ;//= PTHREAD_MUTEX_INITIALIZER;
extern pthread_cond_t condEmptyVerif;// = PTHREAD_COND_INITIALIZER;
extern pthread_cond_t condEndVerif;

#endif
