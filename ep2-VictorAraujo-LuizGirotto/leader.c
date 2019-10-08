#include "leader.h"

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

#include "llip.h"
#include "lln.h"


#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

int
leader() {
    FILE *fd;
    char buffer[MAXLINE];
    char work_number[MAXLINE];

    celula_ip *alive_list = malloc( sizeof( celula_ip));
    celula_n *work_list malloc( sizeof( celula_n));
    pthread_mutex_t *work_list_mutex = malloc( sizeof( pthread_mutex_t));
    pthread_mutex_t *alive_list_mutex = malloc( sizeof( pthread_mutex_t))

    bool is_leader = true;
    bool receiving_jobs = true;

    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char recvline[MAXLINE + 1];
    ssize_t n;

    if (pthread_mutex_init( work_list_mutex, NULL)) {
        fprintf( stderr, "ERROR: Could not initialize mutex\n");
        free( alive_list);
        free( work_list);
        free( work_list_mutex);
        free( alive_list_mutex);
        exit( EXIT_FAILURE);
    }

    if (pthread_mutex_init( alive_list_mutex, NULL)) {
        fprintf( stderr, "ERROR: Could not initialize mutex\n");
        free( alive_list);
        free( work_list);
        pthread_mutex_destroy( work_list_mutex);
        free( work_list_mutex);
        free( alive_list_mutex);
        exit( EXIT_FAILURE);
    }

    if ((listenfd = socket( AF_INET, SOCK_STREAM, 0) == -1)) {
        fprintf( stderr, "ERROR: could not create socket, %s\n", strerror( errno));
        free( alive_list);
        free( work_list);
        pthread_mutex_destroy( work_list_mutex);
        free( work_list_mutex);
        pthread_mutex_destroy( alive_list_mutex);
        free( alive_list_mutex);
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(LEADER_PORT);

    if ((bind( listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)) {
        fprintf( stderr, "ERROR: Could not bind socket, %s\n", strerror( errno));
        free( alive_list);
        free( work_list);
        pthread_mutex_destroy( work_list_mutex);
        free( work_list_mutex);
        pthread_mutex_destroy( alive_list_mutex);
        free( alive_list_mutex);
        exit( EXIT_FAILURE);
    }

    if (listen( listenfd, LISTENQ) == -1) {
        fprintf( stderr, "ERROR: Could not listen on port, %s\n", strerror( errno));
        free( alive_list);
        free( work_list);
        pthread_mutex_destroy( work_list_mutex);
        free( work_list_mutex);
        pthread_mutex_destroy( alive_list_mutex);
        free( alive_list_mutex);
        exit( EXIT_FAILURE);
    }

    while (is_leader) {
        if ((connfd = accept( listenfd, (struct sockaddr *) NULL, NULL) == -1)) {
            fprintf(stderr, "ERROR: Could not accept connection, %s\n", strerror( errno));
            continue;
        }
        n = read( connfd, recvline, MAXLINE);
        recvline[n] = 0;

        if (!strncmp( recvline, "002\r\n", 5 * sizeof( char))) {
            is_leader = false;
        }
        else if (!strncmp( recvline, "004\r\n", 5 * sizeof( char))) {
            /*LIMPA A LISTA*/
            write( connfd, "100\r\n", 5 * sizeof( char));
            while ((read( connfd, buffer, MAXLINE)) > 0) {
                /*MUTEX*/
                insere (buffer, alive_list);
                /*MUTEX*/
            }
        }
        /*recebe um arquivo de trabalho*/
        else if (!strncmp( recvline, "005\r\n", 5 * sizeof( char))) {
            write( connfd, "100\r\n", 5 * sizeof( char));
            read( connfd, buffer, MAXLINE);

            arg->work_number = atoi(buffer);

            strncpy( buffer, "splitIn", MAXLINE * sizeof( char));
            snprintf( work_number, MAXLINE * sizeof( char), "%d", arg->work_number);
            strncat( buffer, work_number, MAXLINE * sizeof( char));
            strncat( buffer, ".txt", MAXLINE * sizeof( char));

            if ((fd = fopen( buffer, "w")) == NULL) {
                fprintf(stderr, "ERROR: Could not open file, %s\n", strerror( errno));
                write( connfd, "111\r\n", 5 * sizeof( char));
                /*LIMPA AS LISTAS*/
                free( alive_list);
                free( work_list);
                pthread_mutex_destroy( work_list_mutex);
                free( work_list_mutex);
                pthread_mutex_destroy( alive_list_mutex);
                free( alive_list_mutex);
                close( connfd);
                close( listenfd);
                exit( EXIT_FAILURE);
            }
            write( connfd, "100\r\n", 5 * sizeof( char));
            while ((read( connfd, buffer, MAXLINE)) > 0) {
                fprintf( fd, "%s", buffer);
            }
            fclose( fd);
            /*colocar mutex*/
            insere( arg->work_number, work_list);
            /*mutex*/
        }
        /*Não vai mais receber trabalho*/
        else if (!strncmp( recvline, "006\r\n", 5 * sizeof( char))) {
            receiving_jobs = false;
        }
        close( connfd);
    }

    /*LIMPA AS LISTAS*/
    free( alive_list);
    free( work_list);
    pthread_mutex_destroy( work_list_mutex);
    free( work_list_mutex);
    pthread_mutex_destroy( alive_list_mutex);
    free( alive_list_mutex);
    close( listenfd);
    exit( EXIT_SUCCESS);

}

void *
communist_leader( void *args) {
    int sockfd_im, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr_im;

    if ((sockfd_im = socket( AF_INET, SOCK_STREAM, 0) == -1)) {
        fprintf( stderr, "ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr_im, sizeof( servaddr_im));
    servaddr_im.sin_family = AF_INET;
    servaddr_im.sin_port = htons( IMMORTAL_PORT);

    /* Get immortal ip from config */
    //inet_pton( AF_INET, argv[1], &servaddr.sin_addr);

    int sockfd_wk;
    struct sockaddr_in servaddr_wk;

    if ((sockfd_wk = socket( AF_INET, SOCK_STREAM, 0) == -1)) {
        fprintf( stderr, "ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr_wk, sizeof( servaddr_wk));
    servaddr_wk.sin_family = AF_INET;
    servaddr_wk.sin_port = htons( WORKER_PORT);


    while (true) {
        if (work_list->prox != NULL) {
            /* Manda trabalho pra quem ta vivo */
        }
        else if (!receiving_jobs) {
            /* Decide se vai pedir trabalho pro imortal, ou se vai */
            /* deixar de ser lider */
        }
    }

    return NULL;
}
