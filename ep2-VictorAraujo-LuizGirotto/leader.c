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

#define LOGFILE "logLD.txt"
FILE *log_ld;

int
leader() {
    printf("[LD] Lider criado\n");
    char buffer[MAXLINE];

    celula_ip *alive_list = malloc( sizeof( celula_ip));
    celula_n *work_list = malloc( sizeof( celula_n));
    work_list->prox = NULL;
    alive_list->prox = NULL;

    pthread_mutex_t *work_list_mutex = malloc( sizeof( pthread_mutex_t));
    pthread_mutex_t *alive_list_mutex = malloc( sizeof( pthread_mutex_t));

    pthread_t *thread = malloc( sizeof( pthread_t));

    bool is_leader = true;

    int listenfd_ld, connfd_ld;
    struct sockaddr_in servaddr_ld;
    char recvline[MAXLINE + 1];
    ssize_t n;

    leader_args *args = malloc( sizeof( leader_args));

    if (pthread_mutex_init( work_list_mutex, NULL)) {
        fprintf( stderr, "LD-ERROR: Could not initialize mutex\n");
        free( alive_list);
        free( work_list);
        free( work_list_mutex);
        free( alive_list_mutex);
        exit( EXIT_FAILURE);
    }

    if (pthread_mutex_init( alive_list_mutex, NULL)) {
        fprintf( stderr, "LD-ERROR: Could not initialize mutex\n");
        free( alive_list);
        free( work_list);
        pthread_mutex_destroy( work_list_mutex);
        free( work_list_mutex);
        free( alive_list_mutex);
        exit( EXIT_FAILURE);
    }

    if ((listenfd_ld = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf( stderr, "LD-ERROR: could not create socket, %s\n", strerror( errno));
        free( alive_list);
        free( work_list);
        pthread_mutex_destroy( work_list_mutex);
        free( work_list_mutex);
        pthread_mutex_destroy( alive_list_mutex);
        free( alive_list_mutex);
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr_ld, sizeof(servaddr_ld));
    servaddr_ld.sin_family      = AF_INET;
    servaddr_ld.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_ld.sin_port        = htons(LEADER_PORT);

    if ((bind( listenfd_ld, (struct sockaddr *)&servaddr_ld, sizeof(servaddr_ld)) == -1)) {
        fprintf( stderr, "LD-ERROR: Could not bind socket, %s\n", strerror( errno));
        free( alive_list);
        free( work_list);
        pthread_mutex_destroy( work_list_mutex);
        free( work_list_mutex);
        pthread_mutex_destroy( alive_list_mutex);
        free( alive_list_mutex);
        exit( EXIT_FAILURE);
    }

    if (listen( listenfd_ld, LISTENQ) == -1) {
        fprintf( stderr, "LD-ERROR: Could not listen on port, %s\n", strerror( errno));
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
    args->is_leader = &is_leader;

    pthread_create( thread, NULL, communist_leader, args);

    log_ld = fopen( LOGFILE, "w");

    if (DEBUG) {
        log_datetime( log_ld);
        fprintf( log_ld, "Inicio de meu tempo de lider\n");
    }

    while (is_leader) {
        if ((connfd_ld = accept( listenfd_ld, (struct sockaddr *) NULL, NULL)) == -1) {
            fprintf(stderr, "LD-ERROR: Could not accept connection, %s\n", strerror( errno));
            continue;
        }
        n = read( connfd_ld, recvline, MAXLINE);
        recvline[n] = 0;
        printf("[LD] recebeu: %s", recvline);

        if (!strncmp( recvline, "002\r\n", 5 * sizeof( char))) {
            write( connfd_ld, "100\r\n", 5 * sizeof( char));
            sleep(1);
            is_leader = false;
        }
        /* Not permanent!! Leader does not respond to heartbeat */
        else if (!strncmp( recvline, "003\r\n", 5 * sizeof( char))) {
            write( connfd_ld, "103\r\n", 5 * sizeof( char));
        }
        else if (!strncmp( recvline, "004\r\n", 5 * sizeof( char))) {
            limpa_llip( alive_list);
            write( connfd_ld, "100\r\n", 5 * sizeof( char));
            while ((read( connfd_ld, buffer, MAXLINE)) > 0) {
                pthread_mutex_lock( alive_list_mutex);
                insere_llip( buffer, alive_list);
                pthread_mutex_unlock( alive_list_mutex);
            }
        }
        close( connfd_ld);
    }

    limpa_llip( alive_list);
    limpa_lln( work_list);
    free( alive_list);
    free( work_list);
    pthread_mutex_destroy( work_list_mutex);
    free( work_list_mutex);
    pthread_mutex_destroy( alive_list_mutex);
    free( alive_list_mutex);
    close( listenfd_ld);
    free( args);

    fclose(log_ld);

    if (DEBUG) {
        log_datetime( log_ld);
        fprintf( log_ld, "Termino do meu tempo de lider\n");
    }

    exit( EXIT_SUCCESS);
}

void *
communist_leader( void *args) {
    int sockfd_im;
    char buffer1[MAXLINE];
    char buffer2[MAXLINE];
    struct sockaddr_in servaddr_im;
    bool alive = true;

    int work_number;
    //FILE *fd;

    leader_args *arg = (leader_args *) args;

    if ((sockfd_im = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf( stderr, "LD-ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr_im, sizeof( servaddr_im));
    servaddr_im.sin_family = AF_INET;
    servaddr_im.sin_port = htons( IMMORTAL_PORT);

    if (inet_pton( AF_INET, getImmortalIP(), &servaddr_im.sin_addr) != 1){
        fprintf(stderr, "LD-ERROR: Some problem with inet_pton\n");
        exit( EXIT_FAILURE);
    }

    int sockfd_wk;
    struct sockaddr_in servaddr_wk;

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
                if ((sockfd_wk = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
                   fprintf( stderr, "LD-ERROR: could not create socket, %s\n", strerror( errno));
                   exit( EXIT_FAILURE);
                }

                if (inet_pton(AF_INET, p->ip, &servaddr_wk.sin_addr) != 1) {
                    perror( "inet_pton");
                    exit( EXIT_FAILURE);
                }
                /* Fazer timeout deste socket ser mais curto do que o normal */
                if (connect( sockfd_wk, (struct sockaddr *) &servaddr_wk, sizeof( servaddr_wk)) < 0) {
                    fprintf( stderr, "LD-Failed to connect to worker. %s\n", strerror( errno));
                    printf("IP de tentativa falha de conexao: %s\n", p->ip);
                }

                else {
                    ssize_t n;
                    write(sockfd_wk, "102\r\n", 5 * sizeof( char));
                    read( sockfd_wk, buffer1, MAXLINE);
                    if (!strncmp( buffer1, "200\r\n", 5 * sizeof( char))) {
                        snprintf( buffer1, MAXLINE, "%d", arg->work_list->prox->workn);
                        write( sockfd_wk, buffer1, strlen( buffer1) * sizeof( char));
                        n = read( sockfd_wk, buffer1, MAXLINE);
                        buffer1[n] = 0;
                        if (!strncmp( buffer1, "200\r\n", 5 * sizeof( char))) {
                            printf("[LD] Enviando trabalho %d\n", arg->work_list->prox->workn);
                            makeFileNameIn( arg->work_list->prox->workn, buffer1, "LD");

                            FILE *fd;
                            char big_buffer[1000];
                            fd = fopen( buffer1, "r");
                            while (fgets( big_buffer, 1000, fd) != NULL) {
                                printf("[LD] vou mandar pro WK %s", big_buffer);
                                write( sockfd_wk, big_buffer, 1000 * sizeof( char));
                                n = read( sockfd_wk, buffer1, MAXLINE);
                                buffer1[n] = 0;
                                msleep(200);
                            }
                            write( sockfd_wk, "EOF\r\n", 5 * sizeof( char));
                            fclose( fd);

                            if (DEBUG) {
                                log_datetime( log_ld);
                                fprintf( log_ld, "Enviei o trabalho %d para um worker (%s)\n", arg->work_list->prox->workn, p->ip);
                            }

                            busca_e_remove_lln( arg->work_list->prox->workn, arg->work_list);
                            close( sockfd_wk);
                            break;
                        }
                    }
                }
                close( sockfd_wk);
                p = p->prox;
            }
            pthread_mutex_unlock( arg->alive_list_mutex);
        }

        else {
            if ((sockfd_im = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
                fprintf( stderr, "LD-ERROR: could not create socket, %s\n", strerror( errno));
                exit( EXIT_FAILURE);
            }

            if (nextLeader()) {
                // Request new leader election
                printf("[LD] I REQUEST A NEW LEADER. NOOOOOOOOOOOOOOOW!\n\n\n\n\n");
                if (connect( sockfd_im, (struct sockaddr *) &servaddr_im, sizeof( servaddr_im)) < 0) {
                    fprintf( stderr, "LD-Failed to connect to immortal.\n");
                }
                write( sockfd_im, "105\r\n", 5 * sizeof( char));
                close( sockfd_im);
                alive = false;
                *(arg->is_leader) = false;
            }
            else {
                // Remain as leader, request new workload
                if (connect( sockfd_im, (struct sockaddr *) &servaddr_im, sizeof( servaddr_im)) < 0) {
                    fprintf( stderr, "LD-Failed to connect to immortal. %s\n", strerror(errno));
                    close( sockfd_im);
                    pthread_mutex_unlock( arg->work_list_mutex);
                    sleep(1);
                    continue;
                }
                ssize_t n;
                printf("[LD] Pedindo novos trabalhos para o imortal\n");
                write( sockfd_im, "106\r\n", 5 * sizeof( char));
                if((n = read( sockfd_im, buffer2, MAXLINE)) < 0) {
                    printf("[LD] Pedido de trabalho sem tamanho.\n");
                    continue;
                }
                buffer2[n] = 0;
                while (!strncmp( buffer2, "005\r\n", 5 * sizeof( char))) {
                    write( sockfd_im, "100\r\n", 5 * sizeof( char));
                    n = read( sockfd_im, buffer2, MAXLINE);
                    buffer2[n] = 0;

                    work_number = atoi(buffer2);
                    makeFileNameIn( work_number, buffer2, "LD");

                    FILE *fd = fopen( buffer2, "w");
                    write( sockfd_im, "100\r\n", 5 * sizeof( char));

                    while (true) {
                        n = read( sockfd_im, buffer2, MAXLINE);
                        if (!strncmp( buffer2, "EOF\r\n", 5 * sizeof( char))) {
                            break;
                        }
                        fprintf( fd, "%s", buffer2);
                        write( sockfd_im, "100\r\n", 5 * sizeof( char));
                    }
                    fclose( fd);
                    printf("[LD] Recebi o trabalho %d!\n", work_number);
                    if (DEBUG) {
                        log_datetime( log_ld);
                        fprintf( log_ld, "Recebi o trabalho %d do immortal\n", work_number);
                    }
                    insere_lln( work_number, arg->work_list);
                    write( sockfd_im, "100\r\n", 5 * sizeof( char));
                    read( sockfd_im, buffer2, MAXLINE);
                }
                printf("[LD] Terminei de receber trabalhos\n");
                close( sockfd_im);
            }
        }
        pthread_mutex_unlock( arg->work_list_mutex);
    }

    pthread_exit(NULL);
    return NULL;
}

bool
nextLeader() {
    srand(time(NULL));
    int i = rand();
    i = i % 10;
    printf("A NOVA ONDA DO IMPERADOR, TALVEZ %d\n\n\n\n\n\n", i);
    if (i < 2) {
        return true;
    }
    return false;
}
