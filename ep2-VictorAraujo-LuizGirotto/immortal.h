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


void immortal( int file_number, char **out_files);

#endif
