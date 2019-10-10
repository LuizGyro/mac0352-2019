#include "immortal.h"

#define JURBS 10

void
immortal( int file_number, char **out_files) {

    celula_n *work_left_list = malloc( sizeof( celula_n));
    celula_n *work_done_list = malloc( sizeof( celula_n));
    celula_n *current_work_list = malloc( sizeof( celula_n));

    pthread_mutex_t *alive_list_mutex = malloc( sizeof( pthread_mutex_t));

    celula_ip *alive_list = malloc( sizeof( celula_ip));

    bool work_left = true;
    bool no_leader = true;

    int work_done = 0;

    char out[MAXLINE + 1];
    char buffer[MAXLINE + 1];

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

        /* Recebe trabalho de um worker */
        if (!strncmp( recvline, "212\r\n", 5 * sizeof( char))) {
            write( connfd, "000\r\n", 5 * sizeof( char));
            n = read( connfd, recvline, MAXLINE);
            recvline[n] = 0;
            int work_number = atoi( recvline);
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
                    else if (!strncmp( buffer, "111\r\n", 5 * sizeof( char))) {
                        no_leader = true;
                    }
                }
            }
            /*WILL GO BOOM*/
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

    /*SEPUKKU ALL THE THINGS*/
    close( listenfd);
    pthread_mutex_destroy( alive_list_mutex);
    free( work_left_list);
    free( work_done_list);
    free( current_work_list);
    free( alive_list);
    return;
}
