#include "worker.h"

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
#include <stdbool.h>

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

#define MINPORT 2000
#define MAXPORT 8000

int
worker() {
    bool all_is_done = false;
    bool work_done = false;
    pthread_mutex_t *work_done_mutex;
    pthread_mutex_init( work_done_mutex, NULL);

    /*Só roubei do EP01*/
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    pid_t childpid;
    char recvline[MAXLINE + 1];
    ssize_t n;


    if ((listenfd = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        perror( "socket :(\n");
        exit( 2);
    }

    bzero( &servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));
    if (bind( listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror( "bind :(\n");
        exit( 3);
    }

    if (listen( listenfd, LISTENQ) == -1) {
        perror( "listen :(\n");
        exit( 4);
    }
    /*Fim do roubo*/

    while (all_is_done) {
        if ((connfd = accept( listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
            perror( "accept :(\n");
            exit( 5);
        }
        if ((childpid = fork()) == 0) {
            close( listenfd);
            n = read( connfd, recvline, MAXLINE);
            recvline[n] = 0;
            /*
             * Agora é ler a mensagem e ver o que fazer.
            */
            if (!strncmp( recvline, "001", 3 * sizeof( char))) {
                printf("SOU O LIDER PORRA\n");
            }
            else if (!strncmp( recvline, "002", 3 * sizeof( char))) {
                printf("Posso trampar ?");
                pthread_mutex_lock( work_done_mutex);
                if (work_done) {
                    write( connfd, "204\r\n", 5 * sizeof( char));
                }
                else {
                    write( connfd, "201\r\n", 5 * sizeof( char));
                }
                pthread_mutex_unlock( work_done_mutex);

            }
            exit( 0);
        }
    }

    return 0;
}


void
work(void *args) {
    work_args *arg = (work_args *) args;
    char work_number[1000];
    char in[1000];
    char out[1000];

    strncpy( in, "splitIn", 1000 * sizeof( char));
    strncpy( out, "splitOut", 1000 * sizeof( char));
    snprintf( work_number, 10000 * sizeof( char), "%d", arg->work_number);
    strncat( in, work_number, 1000 * sizeof( char));
    strncat( out, work_number, 1000 * sizeof( char));
    strncat( in, ".txt", 1000 * sizeof( char));
    strncat( out, ".txt", 1000 * sizeof( char));

    orderFile( in, out);

    pthread_mutex_lock( arg->work_done_mutex);
    *(arg->work_done) = true;
    pthread_mutex_unlock( arg->work_done_mutex);
    /*Mandar pro lider o trabalho*/
    return;
}