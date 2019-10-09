#ifndef __LEADER_H__
#define __LEADER_H__

#include <stdbool.h>
#include <pthread.h>

#include "util.h"
#include "llip.h"
#include "lln.h"
#include "file.h"

typedef struct _leader_args {
    celula_ip *alive_list;
    celula_n *work_list;
    pthread_mutex_t *work_list_mutex;
    pthread_mutex_t *alive_list_mutex;

} leader_args;


int leader();

void *communist_leader( void *args);

bool nextLeader();
#endif
