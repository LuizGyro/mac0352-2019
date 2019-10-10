#ifndef __IMMORTAL_H__
#define __IMMORTAL_H__

#include "util.h"
#include "llip.h"
#include "lln.h"
#include "file.h"
#include "leader.h"
#include "worker.h"

#include <stdbool.h>

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>


#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096
#define TIME_SLEEP 10

typedef struct _im_thread_args {
    char leader_ip[INET_ADDRSTRLEN];
    celula_ip *alive_list;
    pthread_mutex_t *alive_list_mutex;
    pthread_mutex_t *leader_ip_mutex;
} im_thread_args;



void immortal( int file_number, char **out_files);
void * heartbeat( void *args);


#endif
