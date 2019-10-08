#ifndef __LEADER_H__
#define __LEADER_H__

#include "util.h"

typedef struct _leader_args {
    celula_ip *ip_head;
    celula_n *n_head;
    pthread_mutex_t *work_list_mutex;
    pthread_mutex_t *alive_list_mutex;
    bool *receiving_jobs;

} leader_args;


int leader();

void *communist_leader( void *args);

#endif
