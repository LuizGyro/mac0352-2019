#include "immortal.h"

#define JURBS 10

void
immortal( int file_number, char **out_files) {

    celula_n *work_left_list = malloc( sizeof( celula_n));
    celula_n *work_done_list = malloc( sizeof( celula_n));
    celula_n *current_work_list = malloc( sizeof( celula_n));

    pthread_mutex_t *alive_list_mutex = malloc( sizeof( pthread_mutex_t));
    pthread_mutex_t *leader_ip_mutex = malloc( sizeof( pthread_mutex_t));


    celula_ip *alive_list = malloc( sizeof( celula_ip));

    bool work_left = true;

    int work_done = 0;

    char out[MAXLINE + 1];
    char buffer[MAXLINE + 1];
    char leader_ip[INET_ADDRSTRLEN];

    pthread_t *thread = malloc( sizeof( pthread_t));
    im_thread_args *args = malloc( sizeof( im_thread_args));

    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char recvline[MAXLINE + 1];
    ssize_t n;

    if (pthread_mutex_init( alive_list_mutex, NULL)) {
        fprintf( stderr, "ERROR: Could not initialize mutex\n");
        free( work_left_list);
        free( work_done_list);
        free( current_work_list);
        free( alive_list);
        pthread_mutex_destroy( alive_list_mutex);
        free( alive_list_mutex);
        exit( EXIT_FAILURE);
    }

    if (pthread_mutex_init( leader_ip_mutex, NULL)) {
        fprintf( stderr, "ERROR: Could not initialize mutex\n");
        free( work_left_list);
        free( work_done_list);
        free( current_work_list);
        free( alive_list);
        pthread_mutex_destroy( alive_list_mutex);
        free( alive_list_mutex);
        exit( EXIT_FAILURE);
    }


    args->alive_list = alive_list;
    args->alive_list_mutex = alive_list_mutex;
    args->leader_ip_mutex = leader_ip_mutex;
    args->leader_ip = leader_ip;

    pthread_create( thread, NULL, heartbeat, args);

    if ((listenfd = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf( stderr, "ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(IMMORTAL_PORT);

    if ((bind( listenfd, (struct sockaddr *) &servaddr, sizeof( servaddr)) == -1)) {
        fprintf( stderr, "ERROR: Could not bind socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    if (listen( listenfd, LISTENQ) == -1) {
        fprintf( stderr, "ERROR: Could not listen on port, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    for (int i = 0; i < file_number; i++) {
        insere_lln( i, work_left_list);
    }

    while (work_left) {
        if ((connfd = accept( listenfd, (struct sockaddr *) NULL, NULL)) == -1) {
            fprintf(stderr, "ERROR: Could not accept connection, %s\n", strerror( errno));
            continue;
        }
        n = read( connfd, recvline, MAXLINE);
        recvline[n] = 0;

        // Recebe trabalho de um worker
        if (!strncmp( recvline, "212\r\n", 5 * sizeof( char))) {
            write( connfd, "000\r\n", 5 * sizeof( char));
            n = read( connfd, buffer, MAXLINE);
            buffer[n] = 0;
            int work_number = atoi( buffer);
            makeFileNameOut( work_number, out);
            FILE *fd = fopen( out, "w");
            write( connfd, "000\r\n", 5 * sizeof( char));
            while ((read( connfd, buffer, MAXLINE)) > 0) {
                fprintf( fd, "%s", buffer);
            }
            fclose( fd);
            busca_e_remove_lln( work_number, current_work_list);
            insere_lln( work_number, work_done_list);
            out_files[work_number] = malloc( sizeof( out));
            strcpy(out_files[work_number], out);
            work_done++;
            if (work_done == file_number) {
                work_left = false;
            }
        }
        //New machine
        else if (!strncmp( recvline, "202\r\n", 5 * sizeof( char))) {
            write( connfd, "000\r\n", 5 * sizeof( char));
            n = read( connfd, buffer, MAXLINE);
            buffer[n] = 0;
            pthread_mutex_lock( alive_list_mutex);
            insere_llip( buffer, alive_list);
            pthread_mutex_unlock( alive_list_mutex);
        }
        //Election request
        else if (!strncmp( recvline, "105\r\n", 5 * sizeof( char))) {
            printf("Imma pick a leader\n");
        }
        //Jobs request
        else if (!strncmp( recvline, "106\r\n", 5 * sizeof( char))) {
            for (int i = 0; i < JURBS && work_left_list->prox != NULL; i++) {
                write( connfd, "005\r\n", 5 * sizeof( char));
                n = read( connfd, buffer, MAXLINE);
                buffer[n] = 0;
                if (!strncmp( buffer, "100\r\n", 5 * sizeof( char))) {
                    snprintf( buffer, MAXLINE, "%d", work_left_list->prox->workn);
                    write( connfd, buffer, sizeof( buffer));
                    if (!strncmp( buffer, "100\r\n", 5 * sizeof( char))) {
                        makeFileNameIn( work_left_list->prox->workn, buffer);
                        sendFile( buffer, connfd);
                        n = read( connfd, buffer, MAXLINE);
                        buffer[n] = 0;
                        if (!strncmp( buffer, "100\r\n", 5 * sizeof( char))) {
                            insere_lln( work_left_list->prox->workn, current_work_list);
                            busca_e_remove_lln( work_left_list->prox->workn, work_left_list);
                        }
                    }
                }
            }
            //WILL GO BOOM
            if (work_left_list->prox == NULL) {
                celula_n *p = work_left_list;
                work_left_list = current_work_list;
                current_work_list = p;
            }
            write( connfd, "006\r\n", 5 * sizeof( char));
            printf("Imma give this man some jurbs\n");
        }
        close( connfd);
    }

    //SEPUKKU ALL THE THINGS
    close( listenfd);
    pthread_mutex_destroy( alive_list_mutex);
    free( work_left_list);
    free( work_done_list);
    free( current_work_list);
    free( alive_list);
    return;
}

void *
heartbeat( void *args) {
    im_thread_args *arg = (im_thread_args *) args;
    
    char buffer[MAXLINE];

    int num_alive = 0;

    int sockfd_wk;
    struct sockaddr_in servaddr_wk;
    if ((sockfd_wk = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf( stderr, "ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }
    bzero( &servaddr_wk, sizeof( servaddr_wk));
    servaddr_wk.sin_family = AF_INET;
    servaddr_wk.sin_port = htons( WORKER_PORT);

    int sockfd_leader;
    struct sockaddr_in servaddr_leader;
    if ((sockfd_leader = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf( stderr, "ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }
    bzero( &servaddr_leader, sizeof( servaddr_leader));
    servaddr_leader.sin_family = AF_INET;
    servaddr_leader.sin_port = htons( WORKER_PORT);


    while (true) {
        sleep(TIME_SLEEP);

        pthread_mutex_lock( arg->alive_list_mutex);
        
        for (celula_ip *p = arg->alive_list->prox; p != NULL; p = p->prox) {
            if (inet_pton(AF_INET, p->ip, &servaddr_wk.sin_addr) != 1) {
                perror( "inet_pton");
                exit( EXIT_FAILURE);
            }
            int retries = 0;
            // Fazer timeout deste socket ser mais curto do que o normal =
            if (connect( sockfd_wk, (struct sockaddr *) &servaddr_wk, sizeof( servaddr_wk)) < 0) {
                if (retries < 3) {
                    retries++;
                    sleep(1);
                }
                else {
                    fprintf( stderr, "ERROR: Failed to connect to worker, removing from list. %s\n", strerror( errno));
                    busca_e_remove_llip( p->ip, arg->alive_list);
                }
            }
            else {
                write( sockfd_wk, "003\r\n", 5 * sizeof( char));
                int n = read( sockfd_leader, buffer, MAXLINE);
                buffer[n] = 0;
                if (strncmp( buffer, "203\r\n", 5 * sizeof( char))) {
                    num_alive++;
                }
                close( sockfd_wk);
            }

        }

        pthread_mutex_unlock( arg->alive_list_mutex);

        if (num_alive == 0) {
            continue;
        } 

        pthread_mutex_lock( arg->leader_ip_mutex);
        if (inet_pton(AF_INET, arg->leader_ip, &servaddr_leader.sin_addr) == 0 || connect( sockfd_leader, (struct sockaddr *) &servaddr_leader, sizeof( servaddr_leader)) < 0) {
            // Eleicao emergencial
            int new_leader = rand() % num_alive;
            int i = 0;
            celula_ip *p;
            for (p = arg->alive_list->prox; p != NULL; p = p->prox) {
                if (i == new_leader)
                    break;
                i++;
            }
            strncpy( arg->leader_ip, p->ip, INET_ADDRSTRLEN);

            // Avisa para todos quem eh o novo lider
            pthread_mutex_lock( arg->alive_list_mutex);
        
            for (celula_ip *p = arg->alive_list->prox; p != NULL; p = p->prox) {
                if (inet_pton(AF_INET, p->ip, &servaddr_wk.sin_addr) != 1) {
                    perror( "inet_pton");
                    exit( EXIT_FAILURE);
                }
                // Fazer timeout deste socket ser mais curto do que o normal
                if (connect( sockfd_wk, (struct sockaddr *) &servaddr_wk, sizeof( servaddr_wk)) < 0) {
                    fprintf( stderr, "ERROR: Failed to connect do worker, somehow.\n");
                }
                else {
                    if (strncmp(p->ip, arg->leader_ip, INET_ADDRSTRLEN) == 0) {
                        write( sockfd_wk, "001\r\n", 5 * sizeof( char));
                    }
                    write( sockfd_wk, "007\r\n", 5 * sizeof( char));
                    int n = read( sockfd_leader, buffer, MAXLINE);
                    buffer[n] = 0;
                    if (strncmp( buffer, "200\r\n", 5 * sizeof( char))) {
                        write( sockfd_wk, arg->leader_ip, sizeof( arg->leader_ip));
                    }
                    close( sockfd_wk);
                }
            }

            pthread_mutex_unlock( arg->alive_list_mutex);
        }

        connect( sockfd_leader, (struct sockaddr *) &servaddr_leader, sizeof( servaddr_leader));
        write( sockfd_leader, "004\r\n", 5 * sizeof( char));
        int n = read( sockfd_leader, buffer, MAXLINE);
        buffer[n] = 0;
        if (strncmp( buffer, "100\r\n", 5 * sizeof( char))) {
            for (celula_ip *p = arg->alive_list->prox; p != NULL; p = p->prox) {
                write( sockfd_leader, p->ip, INET_ADDRSTRLEN * sizeof( char));
            }
        }
        close( sockfd_leader);
        pthread_mutex_unlock( arg->leader_ip_mutex);
    }
    return NULL;
}