#include "immortal.h"

#define JURBS 2

void
immortal( int file_number, char **out_files) {

    celula_n *work_left_list = malloc( sizeof( celula_n));
    celula_n *work_done_list = malloc( sizeof( celula_n));
    celula_n *current_work_list = malloc( sizeof( celula_n));

    work_left_list->prox = NULL;
    work_done_list->prox = NULL;
    current_work_list->prox = NULL;

    pthread_mutex_t *alive_list_mutex = malloc( sizeof( pthread_mutex_t));
    pthread_mutex_t *leader_ip_mutex = malloc( sizeof( pthread_mutex_t));
    pthread_mutex_t *work_lists_mutex = malloc( sizeof( pthread_mutex_t));

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
        fprintf( stderr, "IM-ERROR: Could not initialize mutex\n");
        free( work_left_list);
        free( work_done_list);
        free( current_work_list);
        free( alive_list);
        pthread_mutex_destroy( alive_list_mutex);
        free( alive_list_mutex);
        printf("Imma die, mutex\n");
        exit( EXIT_FAILURE);
    }

    if (pthread_mutex_init( leader_ip_mutex, NULL)) {
        fprintf( stderr, "IM-ERROR: Could not initialize mutex\n");
        free( work_left_list);
        free( work_done_list);
        free( current_work_list);
        free( alive_list);
        pthread_mutex_destroy( alive_list_mutex);
        free( alive_list_mutex);
        printf("Imma die, mutex\n");
        exit( EXIT_FAILURE);
    }

    if (pthread_mutex_init( work_lists_mutex, NULL)) {
        fprintf( stderr, "IM-ERROR: Could not initialize mutex\n");
        free( work_left_list);
        free( work_done_list);
        free( current_work_list);
        free( alive_list);
        pthread_mutex_destroy( alive_list_mutex);
        free( alive_list_mutex);
        printf("Imma die, mutex\n");
        exit( EXIT_FAILURE);
    }


    args->alive_list = alive_list;
    args->work_left_list = work_left_list;
    args->current_work_list = current_work_list;
    args->alive_list_mutex = alive_list_mutex;
    args->leader_ip_mutex = leader_ip_mutex;
    args->work_lists_mutex = work_lists_mutex;
    args->leader_ip = leader_ip;

    pthread_create( thread, NULL, heartbeat, args);

    if ((listenfd = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf( stderr, "IM-ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(IMMORTAL_PORT);

    if (bind( listenfd, (struct sockaddr *) &servaddr, sizeof( servaddr)) == -1) {
        fprintf( stderr, "IM-ERROR: Could not bind socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    if (listen( listenfd, LISTENQ) == -1) {
        fprintf( stderr, "IM-ERROR: Could not listen on port, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    for (int i = 0; i < file_number; i++) {
        insere_lln( i, work_left_list);
    }

    while (work_left) {
        if ((connfd = accept( listenfd, (struct sockaddr *) NULL, NULL)) == -1) {
            fprintf(stderr, "IM-ERROR: Could not accept connection, %s\n", strerror( errno));
            continue;
        }
        n = read( connfd, recvline, MAXLINE);
        recvline[n] = 0;
        printf("[IM] Recebi: %s", recvline);

        // Recebe trabalho de um worker
        if (!strncmp( recvline, "212\r\n", 5 * sizeof( char))) {
            pthread_mutex_lock( work_lists_mutex);
            write( connfd, "000\r\n", 5 * sizeof( char));
            n = read( connfd, buffer, MAXLINE);
            buffer[n] = 0;
            int work_number = atoi( buffer);
            makeFileNameOut( work_number, out, "IM");
            FILE *fd = fopen( out, "w");
            write( connfd, "000\r\n", 5 * sizeof( char));
            while ((read( connfd, buffer, MAXLINE)) > 0) {
                if (!strncmp( buffer, "EOF\r\n", 5 * sizeof( char))) {
                    break;
                }
                fprintf( fd, "%s", buffer);
                printf("[IM] recebi algo do WK %s", buffer);
                write(connfd, "200\r\n", 5 * sizeof( char));
                msleep(200);
            }
            fclose( fd);
            write( connfd, "000\r\n", 5 * sizeof( char));
            busca_e_remove_lln( work_number, current_work_list);
            insere_lln( work_number, work_done_list);
            out_files[work_number] = malloc( strlen( out) * sizeof( char));
            strcpy(out_files[work_number], out);
            work_done++;
            if (work_done == file_number) {
                work_left = false;
            }
            pthread_mutex_unlock( work_lists_mutex);
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
            printf("[IM] Fazendo eleicao requerida\n");
        }
        //Jobs request
        else if (!strncmp( recvline, "106\r\n", 5 * sizeof( char))) {
            pthread_mutex_lock( work_lists_mutex);
            for (int i = 0; i < JURBS && work_left_list->prox != NULL; i++) {
                printf("[IM] Enviando trabalho numero %d para lider\n", work_left_list->prox->workn);
                write( connfd, "005\r\n", 5 * sizeof( char));
                n = read( connfd, buffer, MAXLINE);
                buffer[n] = 0;
                if (!strncmp( buffer, "100\r\n", 5 * sizeof( char))) {
                    snprintf( buffer, MAXLINE, "%d", work_left_list->prox->workn);
                    write( connfd, buffer, strlen(buffer) * sizeof( char));
                    n = read( connfd, buffer, MAXLINE);
                    buffer[n] = 0;

                    if (!strncmp( buffer, "100\r\n", 5 * sizeof( char))) {
                        makeFileNameIn( work_left_list->prox->workn, buffer, "IM");
                        FILE *fd;
                        char big_buffer[1000];
                        fd = fopen( buffer, "r");
                        while (fgets( big_buffer, 1000, fd) != NULL) {
                            write( connfd, big_buffer, 1000 * sizeof( char));
                            n = read( connfd, buffer, MAXLINE);
                            buffer[n] = 0;
                        }
                        write( connfd, "EOF\r\n", 5 * sizeof( char));
                        fclose( fd);
                        n = read( connfd, buffer, MAXLINE);
                        buffer[n] = 0;
                        if (!strncmp( buffer, "100\r\n", 5 * sizeof( char))) {
                            insere_lln( work_left_list->prox->workn, current_work_list);
                            busca_e_remove_lln( work_left_list->prox->workn, work_left_list);
                        }
                    }
                }
            }
            sleep(2);
            /*
            //WILL GO BOOM
            if (work_left_list->prox == NULL) {
                celula_n *p = work_left_list;
                work_left_list = current_work_list;
                current_work_list = p;
            }
            */
            write( connfd, "006\r\n", 5 * sizeof( char));
            printf("[IM] Nao enviarei mais trabalhos para o lider.\n");
            pthread_mutex_unlock( work_lists_mutex);
        }
        printf("[IM] Fechei a ultima conexao. Continuarei: %d\n", work_left);
        close( connfd);
    }
    //SEPUKKU ALL THE THINGS
    printf("[IM] Preparando-se para acabar\n");
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

    int num_alive;

    int sockfd_wk;
    struct sockaddr_in servaddr_wk;
    bzero( &servaddr_wk, sizeof( servaddr_wk));
    servaddr_wk.sin_family = AF_INET;
    servaddr_wk.sin_port = htons( WORKER_PORT);

    int sockfd_leader;
    struct sockaddr_in servaddr_leader;
    if ((sockfd_leader = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf( stderr, "IM-ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }
    bzero( &servaddr_leader, sizeof( servaddr_leader));
    servaddr_leader.sin_family = AF_INET;
    servaddr_leader.sin_port = htons( LEADER_PORT);


    while (true) {
        sleep(TIME_SLEEP);

        pthread_mutex_lock( arg->alive_list_mutex);

        /* Checa se geral ta vivo */
        num_alive = 0;
        for (celula_ip *p = arg->alive_list->prox; p != NULL; p = p->prox) {
            if ((sockfd_wk = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
                fprintf( stderr, "IM-ERROR: could not create socket, %s\n", strerror( errno));
                exit( EXIT_FAILURE);
            }
            if (inet_pton(AF_INET, p->ip, &servaddr_wk.sin_addr) != 1) {
                perror( "inet_pton");
                exit( EXIT_FAILURE);
            }
            int retries = 0;
            // Fazer timeout deste socket ser mais curto do que o normal
            while (connect( sockfd_wk, (struct sockaddr *) &servaddr_wk, sizeof( servaddr_wk)) < 0) {
                if (retries < 3) {
                    retries++;
                    sleep(1);
                }
                else {
                    fprintf( stderr, "IM-ERROR: Failed to connect to worker, removing from list. %s\n", strerror( errno));
                    busca_e_remove_llip( p->ip, arg->alive_list);
                    /* Possivel que o trabalhador morra sem mandar seu
                    trabalho. Logo, fazemos por seguranca */
                    pthread_mutex_lock( arg->work_lists_mutex);
                    printf("[IM] Someone went boom, and we're not risking it\n");
                    celula_n *q;
                    for (q = arg->current_work_list->prox; q != NULL; q = q->prox) {
                        insere_lln( q->workn, arg->work_left_list);
                        busca_e_remove_lln( q->workn, arg->current_work_list);
                    }
                    pthread_mutex_unlock( arg->work_lists_mutex);
                    break;
                }
            }
            if (retries < 3) {
                write( sockfd_wk, "003\r\n", 5 * sizeof( char));
                int n = read( sockfd_wk, buffer, MAXLINE);
                buffer[n] = 0;
                if (!strncmp( buffer, "203\r\n", 5 * sizeof( char))) {
                    num_alive++;
                }
            }
            close( sockfd_wk);
        }

        pthread_mutex_unlock( arg->alive_list_mutex);

        if (num_alive == 0) {
            continue;
        }

        /* Checa se precisa de eleicao */
        pthread_mutex_lock( arg->leader_ip_mutex);
        bool requires_election = false;
        if (inet_pton(AF_INET, arg->leader_ip, &servaddr_leader.sin_addr) != 1) {
            requires_election = true;
        }
        else {
            if ((sockfd_leader = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
                fprintf( stderr, "IM-ERROR: could not create socket, %s\n", strerror( errno));
                exit( EXIT_FAILURE);
            }
            int retries = 0;
            // Fazer timeout deste socket ser mais curto do que o normal
            while ((connect( sockfd_leader, (struct sockaddr *) &servaddr_leader, sizeof( servaddr_leader))) == -1) {
                printf("IM-ERROR: Failed to connect to existing leader: %s\n", strerror(errno));
                if (retries < 5) {
                    retries++;
                    sleep(1);
                }
                else {
                    requires_election = true;
                    break;
                }
                close( sockfd_leader);
                if ((sockfd_leader = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
                    fprintf( stderr, "IM-ERROR: could not create socket, %s\n", strerror( errno));
                    exit( EXIT_FAILURE);
                }
            }
            if (!requires_election) {
                /* Sending heartbeat is not required, but we should close socket here */
                write( sockfd_leader, "003\r\n", 5 * sizeof( char));
                int n = read( sockfd_leader, buffer, MAXLINE);
                buffer[n] = 0;
                printf("[IM] lider me falou: %s", buffer);
                close( sockfd_leader);
                if ((sockfd_leader = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
                    fprintf( stderr, "IM-ERROR: could not create socket, %s\n", strerror( errno));
                    exit( EXIT_FAILURE);
                }
            }
        }

        if (requires_election) {
            // Eleicao emergencial
            int new_leader = rand() % num_alive;
            int i = 0;
            celula_ip *p;

            /* Possivel que o lider antigo morreu sem mandar todos
            seus trabalhos. Logo, fazemos por seguranca */
            pthread_mutex_lock( arg->work_lists_mutex);
            printf("[IM] Someone went boom, and we're not risking it\n");
            celula_n *q;
            for (q = arg->current_work_list->prox; q != NULL; q = q->prox) {
                insere_lln( q->workn, arg->work_left_list);
                busca_e_remove_lln( q->workn, arg->current_work_list);
            }
            pthread_mutex_unlock( arg->work_lists_mutex);

            for (p = arg->alive_list->prox; p != NULL; p = p->prox) {
                if (i == new_leader)
                    break;
                i++;
            }
            strncpy( arg->leader_ip, p->ip, INET_ADDRSTRLEN);
            printf("NOVO LIDE SUPREMO: %s\n", arg->leader_ip);
            // Avisa para todos quem eh o novo lider
            pthread_mutex_lock( arg->alive_list_mutex);

            for (celula_ip *p = arg->alive_list->prox; p != NULL; p = p->prox) {
                if ((sockfd_wk = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
                    fprintf( stderr, "IM-ERROR: could not create socket, %s\n", strerror( errno));
                    exit( EXIT_FAILURE);
                }
                if (inet_pton(AF_INET, p->ip, &servaddr_wk.sin_addr) != 1) {
                    perror( "inet_pton");
                    exit( EXIT_FAILURE);
                }
                // Fazer timeout deste socket ser mais curto do que o normal
                if (connect( sockfd_wk, (struct sockaddr *) &servaddr_wk, sizeof( servaddr_wk)) < 0) {
                    fprintf( stderr, "IM-ERROR: Failed to connect to worker, somehow. %s\n", strerror(errno));
                }
                else {
                    if (strncmp(p->ip, arg->leader_ip, INET_ADDRSTRLEN) == 0) {
                        printf("Imma pick you %s | %s\n", arg->leader_ip, p->ip);
                        write( sockfd_wk, "001\r\n", 5 * sizeof( char));
                    }
                    write( sockfd_wk, "007\r\n", 5 * sizeof( char));
                    int n = read( sockfd_wk, buffer, MAXLINE);
                    buffer[n] = 0;
                    if (!strncmp( buffer, "200\r\n", 5 * sizeof( char))) {
                        write( sockfd_wk, arg->leader_ip, INET_ADDRSTRLEN * sizeof( char));
                    }
                }
                close( sockfd_wk);
            }
            pthread_mutex_unlock( arg->alive_list_mutex);
        }
        else {
            printf("[IM] No election required\n");
        }

        if (connect( sockfd_leader, (struct sockaddr *) &servaddr_leader, sizeof( servaddr_leader)) == -1) {
            fprintf( stderr, "ERRO: Could not connect IM to LD: %s\n", strerror( errno));
        }
        else {
            write( sockfd_leader, "004\r\n", 5 * sizeof( char));
            int n = read( sockfd_leader, buffer, MAXLINE);
            buffer[n] = 0;
            if (!strncmp( buffer, "100\r\n", 5 * sizeof( char))) {
                printf("[IM] Sending alive list\n");
                for (celula_ip *p = arg->alive_list->prox; p != NULL; p = p->prox) {
                    write( sockfd_leader, p->ip, INET_ADDRSTRLEN * sizeof( char));
                }
            }
        }
        close( sockfd_leader);
        pthread_mutex_unlock( arg->leader_ip_mutex);
    }
    return NULL;
}
