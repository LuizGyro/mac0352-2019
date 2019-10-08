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


int
worker() {
    FILE *fd;
    char buffer[MAXLINE];
    char work_number[MAXLINE];

    bool work_left = true;
    bool work_done = false;
    work_args arg;
    pthread_mutex_t *work_done_mutex;
    pthread_t *thread;
    pthread_mutex_init( work_done_mutex, NULL);
    arg->work_done_mutex = work_done_mutex;
    arg->work_done = &work_done;

    /*Só roubei do EP01*/
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    pid_t childpid;
    char recvline[MAXLINE + 1];
    ssize_t n;


    listenfd = socket( AF_INET, SOCK_STREAM, 0));

    bzero( &servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));
    bind( listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    listen( listenfd, LISTENQ);
    /*Fim do roubo*/

    while (work_left) {
        connfd = accept( listenfd, (struct sockaddr *) NULL, NULL));
        n = read( connfd, recvline, MAXLINE);
        recvline[n] = 0;
        if (!strncmp( recvline, "001", 3 * sizeof( char))) {
            /*Precisa fazer o fork e chamar a função lider*/
            printf("SOU O LIDER PORRA\n");
        }
        else if (!strncmp( recvline, "102", 3 * sizeof( char))) {
            pthread_mutex_lock( work_done_mutex);
            if (work_done) {
                write( connfd, "200\r\n", 5 * sizeof( char));
                read( connfd, buffer, MAXLINE);
                arg->work_number = atoi(buffer);

                strncpy( buffer, "splitIn", MAXLINE * sizeof( char));
                snprintf( work_number, MAXLINE * sizeof( char), "%d", arg->work_number);
                strncat( buffer, work_number, MAXLINE * sizeof( char));
                strncat( buffer, ".txt", MAXLINE * sizeof( char));

                fd = fopen( buffer, "w")) == NULL;
                while ((read( connfd, buffer, MAXLINE)) > 0) {
                    fprintf( fl, "%s", buffer);
                }
                fclose( fd);
                pthread_create( thread, NULL, work, arg);
            }
            else {
                write( connfd, "210\r\n", 5 * sizeof( char));
            }
            pthread_mutex_unlock( work_done_mutex);
        }
        else if (!strncmp( recvline, "003", 3 * sizeof( char))) {
            /*Ta vivo ?*/
            /*Esse seria o broadcast, muda o que ele escreve para se o IP*/
            write( connfd, "200\r\n", 5 * sizeof( char));
        }
        close( connfd);
    }

    return 0;
}


void *
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



    /*Mandar pro lider o trabalho*/
    int sockfd, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    if ((sockfd = socket( AF_INET, SOCK_STREAM, 0)) < 0)

    bzero( &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(13);

    inet_pton( AF_INET, argv[1], &servaddr.sin_addr);

    connect( sockfd, (struct sockaddr *) &servaddr, sizeof( servaddr));
    close( sockfd);

    pthread_mutex_lock( arg->work_done_mutex);
    *(arg->work_done) = true;
    pthread_mutex_unlock( arg->work_done_mutex);
    return;
}
