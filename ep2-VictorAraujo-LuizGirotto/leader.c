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
    pthread_mutex_t *alive_list_mutex = malloc( sizeof( pthread_mutex_t));

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
            limpa( alive_list);
            write( connfd, "100\r\n", 5 * sizeof( char));
            while ((read( connfd, buffer, MAXLINE)) > 0) {
                pthread_mutex_lock( alive_list_mutex);
                insere (buffer, alive_list);
                pthread_mutex_unlock( alive_list_mutex);
            }
        }
        /*recebe um arquivo de trabalho*/
        else if (!strncmp( recvline, "005\r\n", 5 * sizeof( char))) {
            write( connfd, "100\r\n", 5 * sizeof( char));
            read( connfd, buffer, MAXLINE);

            arg->work_number = atoi(buffer);

            makeFileNameIn( arg->work_number, buffer);

            if ((fd = fopen( buffer, "w")) == NULL) {
                fprintf(stderr, "ERROR: Could not open file, %s\n", strerror( errno));
                write( connfd, "111\r\n", 5 * sizeof( char));
                limpa( alive_list);
                limpa( work_list);
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

    limpa( alive_list);
    limpa( work_list);
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
    char buffer[MAXLINE];
    struct sockaddr_in servaddr_im;

    leader_args *arg = (leader_args *) args;

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
        if (arg->work_list->prox != NULL) {
            /* Manda trabalho um pra quem ta vivo */

            /* Acha alguém que quer trabalhar*/
            celula_ip *p;
            p = arg->alive_list->prox;
            while (p != NULL) {
                if (!inet_pton(AF_INET, p->ip, &servaddr_wk.sin_addr)) {
                  perror( "inet_pton");
                  exit( EXIT_FAILURE);
                }
                /* Fazer timeout deste socket ser mais curto do que o normal */
                if (connect( sockfd_wk, (struct sockaddr *) &servaddr_wk, sizeof( servaddr_wk)) < 0) {
                    fprintf( stderr,"Failed to connect do worker.\n");
                }

                else {
                    write(sockfd_wk, "102\r\n", 5 * sizeof( char));
                    read( sockfd_wk, buffer, MAXLINE);
                    if (!strncmp( buffer, "200\r\n", 5 * sizeof( char))) {
                        snprintf( buffer, MAXLINE, "%d", arg->work_list->prox->wnum);
                        write( sockfd_wk, buffer, sizeof( buffer));
                        read( sockfd_wk, buffer, MAXLINE);
                        if (!strncmp( buffer, "200\r\n", 5 * sizeof( char))) {
                            makeFileNameIn( arg->work_list->prox->wnum, buffer);
                            sendFile( buffer, sockfd_wk);
                            busca_e_remove( arg->work_list->prox->wnum, arg->work_list);
                            break;
                        }
                    }
                }
                p = p->prox;
            }
        }
        else if (*(arg->receiving_jobs) == false) {
            /* Decide se vai pedir trabalho pro imortal, ou se vai */
            /* deixar de ser lider */
        }
    }

    return NULL;
}
