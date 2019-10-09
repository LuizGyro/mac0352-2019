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


#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

int
leader() {
    FILE *fd;
    char buffer[MAXLINE];

    int work_number;

    celula_ip *alive_list = malloc( sizeof( celula_ip));
    celula_n *work_list = malloc( sizeof( celula_n));
    pthread_mutex_t *work_list_mutex = malloc( sizeof( pthread_mutex_t));
    pthread_mutex_t *alive_list_mutex = malloc( sizeof( pthread_mutex_t));

    pthread_t thread;

    bool is_leader = true;
    bool receiving_jobs = true;

    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char recvline[MAXLINE + 1];
    ssize_t n;

    leader_args *args = malloc( sizeof( leader_args));

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

    args->alive_list = alive_list;
    args->alive_list_mutex = alive_list_mutex;
    args->work_list = work_list;
    args->work_list_mutex = work_list_mutex;
    args->receiving_jobs = &receiving_jobs;

    pthread_create( thread, NULL, communist_leader, args);
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
            limpa_llip( alive_list);
            write( connfd, "100\r\n", 5 * sizeof( char));
            while ((read( connfd, buffer, MAXLINE)) > 0) {
                pthread_mutex_lock( alive_list_mutex);
                insere_llip( buffer, alive_list);
                pthread_mutex_unlock( alive_list_mutex);
            }
        }
        /*recebe um arquivo de trabalho*/
        else if (!strncmp( recvline, "005\r\n", 5 * sizeof( char))) {
            write( connfd, "100\r\n", 5 * sizeof( char));
            read( connfd, buffer, MAXLINE);

            work_number = atoi(buffer);

            makeFileNameIn( work_number, buffer);

            if ((fd = fopen( buffer, "w")) == NULL) {
                /*LEMBRAR QUE O LIDER MORRE (na hora de fazer o imortal)*/
                fprintf(stderr, "ERROR: Could not open file, %s\n", strerror( errno));
                write( connfd, "111\r\n", 5 * sizeof( char));
                limpa_llip( alive_list);
                limpa_lln( work_list);
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
            pthread_mutex_lock( work_list_mutex);
            insere_lln( work_number, work_list);
            pthread_mutex_unlock( work_list_mutex);
        }
        /*Não vai mais receber trabalho*/
        else if (!strncmp( recvline, "006\r\n", 5 * sizeof( char))) {
            receiving_jobs = false;
        }
        close( connfd);
    }

    limpa_llip( alive_list);
    limpa_lln( work_list);
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
    int sockfd_im;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr_im;
    bool alive = true;

    leader_args *arg = (leader_args *) args;

    if ((sockfd_im = socket( AF_INET, SOCK_STREAM, 0) == -1)) {
        fprintf( stderr, "ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr_im, sizeof( servaddr_im));
    servaddr_im.sin_family = AF_INET;
    servaddr_im.sin_port = htons( IMMORTAL_PORT);

    if (inet_pton( AF_INET, getImmortalIP(), &servaddr_im.sin_addr) != 1){
        fprintf(stderr, "ERROR: Some problem with inet_pton\n");
        exit( EXIT_FAILURE);
    }

    int sockfd_wk;
    struct sockaddr_in servaddr_wk;

    if ((sockfd_wk = socket( AF_INET, SOCK_STREAM, 0) == -1)) {
        fprintf( stderr, "ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr_wk, sizeof( servaddr_wk));
    servaddr_wk.sin_family = AF_INET;
    servaddr_wk.sin_port = htons( WORKER_PORT);


    while (alive) {
        pthread_mutex_lock( arg->work_list_mutex);
        if (arg->work_list->prox != NULL) {
            /* Manda trabalho um pra quem ta vivo */

            /* Acha alguém que quer trabalhar*/
            celula_ip *p;
            pthread_mutex_lock( arg->alive_list_mutex);
            p = arg->alive_list->prox;
            while (p != NULL) {
                if (inet_pton(AF_INET, p->ip, &servaddr_wk.sin_addr) != 1) {
                    perror( "inet_pton");
                    exit( EXIT_FAILURE);
                }
                /* Fazer timeout deste socket ser mais curto do que o normal */
                if (connect( sockfd_wk, (struct sockaddr *) &servaddr_wk, sizeof( servaddr_wk)) < 0) {
                    fprintf( stderr, "Failed to connect do worker.\n");
                }

                else {
                    write(sockfd_wk, "102\r\n", 5 * sizeof( char));
                    read( sockfd_wk, buffer, MAXLINE);
                    if (!strncmp( buffer, "200\r\n", 5 * sizeof( char))) {
                        snprintf( buffer, MAXLINE, "%d", arg->work_list->prox->workn);
                        write( sockfd_wk, buffer, sizeof( buffer));
                        read( sockfd_wk, buffer, MAXLINE);
                        if (!strncmp( buffer, "200\r\n", 5 * sizeof( char))) {
                            makeFileNameIn( arg->work_list->prox->workn, buffer);
                            sendFile( buffer, sockfd_wk);
                            busca_e_remove_lln( arg->work_list->prox->workn, arg->work_list);
                            close( sockfd_wk);
                            break;
                        }
                    }
                    close( sockfd_wk);
                }
                p = p->prox;
            }
            pthread_mutex_unlock( arg->alive_list_mutex);
        }
        else if (*(arg->receiving_jobs) == false) {
            if (nextLeader()) {
                /* Request new leader election */
                if (connect( sockfd_im, (struct sockaddr *) &servaddr_im, sizeof( servaddr_im)) < 0) {
                    fprintf( stderr, "Failed to connect do worker.\n");
                }
                write( sockfd_im, "105\r\n", 5 * sizeof( char));
                alive = false;
                close( sockfd_im);
            }
            else {
                /* Remain as leader, request new workload */
                if (connect( sockfd_im, (struct sockaddr *) &servaddr_im, sizeof( servaddr_im)) < 0) {
                    fprintf( stderr, "Failed to connect do worker.\n");
                }
                write( sockfd_im, "106\r\n", 5 * sizeof( char));
                read( sockfd_im, buffer, MAXLINE);
                if (!strncmp( buffer, "200\r\n", 5 * sizeof( char))) {
                    *(arg->receiving_jobs) = true;
                }
                else {
                    alive = false;
                }
            }
        }
        pthread_mutex_unlock( arg->work_list_mutex);
    }

    return NULL;
}

bool
nextLeader() {
    if (rand() % 2)
        return true;
    return false;
}
