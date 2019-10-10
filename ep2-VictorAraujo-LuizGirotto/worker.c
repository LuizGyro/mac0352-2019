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
    char ip[INET_ADDRSTRLEN];


    pid_t leader_pid;

    bool work_left = true;
    bool work_done = true;

    work_args *arg = malloc( sizeof( work_args));
    if (arg == NULL) {
        fprintf( stderr, "WK-ERROR: Could not allocate memory\n");
        exit( EXIT_FAILURE);
    }
    pthread_mutex_t *work_done_mutex = malloc( sizeof( pthread_mutex_t));
    if (work_done_mutex == NULL) {
        fprintf( stderr, "WK-ERROR: Could not allocate memory\n");
        exit( EXIT_FAILURE);
    }
    pthread_t *thread = malloc ( sizeof( pthread_t));
    if (thread == NULL) {
        fprintf( stderr, "WK-ERROR: Could not allocate memory\n");
        exit( EXIT_FAILURE);
    }

    if (pthread_mutex_init( work_done_mutex, NULL)) {
        fprintf( stderr, "WK-ERROR: Could not initialize mutex\n");
        exit( EXIT_FAILURE);
    }

    arg->work_done_mutex = work_done_mutex;
    arg->work_done = &work_done;

    /* Prepara socket pra comunicacao inicial com o imortal */
    int sockfd, len;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr, servaddr_im;
    if ((sockfd = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf( stderr, "WK-ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr_im, sizeof( servaddr_im));
    servaddr_im.sin_family = AF_INET;
    servaddr_im.sin_port = htons( IMMORTAL_PORT);

    if (inet_pton( AF_INET, getImmortalIP(), &servaddr_im.sin_addr) != 1){
        fprintf( stderr, "WK-ERROR: Some problem with inet_pton\n");
        exit( EXIT_FAILURE);
    }

    /* Comunica-se com o imortal */
    while (connect( sockfd, (struct sockaddr *) &servaddr_im, sizeof( servaddr_im)) < 0) {
        fprintf( stderr, "WK-%s\n", strerror( errno));
    }
    write( sockfd, "202\r\n", 5 * sizeof( char));
    len = read( sockfd, recvline, MAXLINE);
    recvline[len] = 0;
    if (!strncmp( recvline, "000\r\n", 5 * sizeof( char))) {
        getIP( ip, sizeof( ip));
        write( sockfd, ip, sizeof( ip));
    }
    close( sockfd);

    /*Só roubei do EP01*/
    int listenfd, connfd;
    ssize_t n;

    if ((listenfd = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf( stderr, "WK-ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr, sizeof( servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(WORKER_PORT);

    if (bind( listenfd, (struct sockaddr *) &servaddr, sizeof( servaddr)) == -1) {
        fprintf( stderr, "WK-ERROR: Could not bind socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    if (listen( listenfd, LISTENQ) == -1) {
        fprintf( stderr, "WK-ERROR: Could not listen on port, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }
    /*Fim do roubo*/


    while (work_left) {
        if ((connfd = accept( listenfd, (struct sockaddr *) NULL, NULL)) == -1) {
            fprintf( stderr, "WK-ERROR: Could not accept connection, %s\n", strerror( errno));
            continue;
        }
        n = read( connfd, recvline, MAXLINE);
        recvline[n] = 0;
        printf("[WK] recebeu: %s", recvline);
        if (!strncmp( recvline, "001\r\n", 5 * sizeof( char))) {
            if ((leader_pid = fork()) == 0) {
                close( connfd);
                close( listenfd);
                leader();
            }
        }
        else if (!strncmp( recvline, "102\r\n", 5 * sizeof( char))) {
            pthread_mutex_lock( work_done_mutex);
            if (work_done) {
                write( connfd, "200\r\n", 5 * sizeof( char));
                read( connfd, buffer, MAXLINE);
                arg->work_number = atoi(buffer);
                makeFileNameIn( arg->work_number, buffer);

                if ((fd = fopen( buffer, "w")) == NULL) {
                    fprintf(stderr, "WK-ERROR: Could not open file, %s\n", strerror( errno));
                    write( connfd, "211\r\n", 5 * sizeof( char));
                    exit( EXIT_FAILURE);
                }
                write( connfd, "200\r\n", 5 * sizeof( char));
                while ((read( connfd, buffer, MAXLINE)) > 0) {
                    fprintf( fd, "%s", buffer);
                }
                fclose( fd);
                if (pthread_create( thread, NULL, work, arg)) {
                    fprintf(stderr, "WK-ERROR: Could not create thread\n");
                }
            }
            else {
                write( connfd, "210\r\n", 5 * sizeof( char));
            }
            pthread_mutex_unlock( work_done_mutex);
        }
        else if (!strncmp( recvline, "003\r\n", 5 * sizeof( char))) {
            write( connfd, "203\r\n", 5 * sizeof( char));
        }
        else if (!strncmp( recvline, "007\r\n", 5 * sizeof( char))) {
            write( connfd, "200\r\n", 5 * sizeof( char));
            read( connfd, buffer, MAXLINE);
            printf("Meu novo lider é: %s", buffer);
        }
        close( connfd);
    }
    close( listenfd);
    pthread_mutex_destroy( work_done_mutex);
    exit( EXIT_SUCCESS);
}


void *
work(void *args) {
    work_args *arg = (work_args *) args;
    char in[1000];
    char out[1000];

    pthread_mutex_lock( arg->work_done_mutex);
    *(arg->work_done) = false;
    pthread_mutex_unlock( arg->work_done_mutex);

    makeFileNameIn( arg->work_number, in);
    makeFileNameOut( arg->work_number, out);
    orderFile( in, out);

    /*Mandar pro imortal o trabalho*/
    int sockfd, n;
    char recvline[MAXLINE + 1];
    char buffer[MAXLINE + 1];

    struct sockaddr_in servaddr;

    if ((sockfd = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf( stderr, "WK-ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr, sizeof( servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( IMMORTAL_PORT);

    if (inet_pton( AF_INET, getImmortalIP(), &servaddr.sin_addr) != 1){
        fprintf(stderr, "WK-ERROR: Some problem with inet_pton\n");
        exit( EXIT_FAILURE);
    }

    connect( sockfd, (struct sockaddr *) &servaddr, sizeof( servaddr));
    /* Send work to immortal */
    write( sockfd, "212\r\n", 5 * sizeof( char));
    n = read( sockfd, recvline, MAXLINE);
    recvline[n] = 0;
    if (!strncmp( recvline, "000\r\n", 5 * sizeof( char))) {
        snprintf( buffer, MAXLINE, "%d", arg->work_number);
        write( sockfd, buffer, sizeof( buffer));
        n = read( sockfd, recvline, MAXLINE);
        recvline[n] = 0;
        if (!strncmp( recvline, "000\r\n", 5 * sizeof( char))) {
            sendFile( out, sockfd);
        }
    }
    close( sockfd);
    pthread_mutex_lock( arg->work_done_mutex);
    *(arg->work_done) = true;
    pthread_mutex_unlock( arg->work_done_mutex);
    return NULL;
}
