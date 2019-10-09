#include "immortal.h"

void
immortal( int file_number) {

    celula_n *work_left_list = malloc( sizeof( celula_n));
    celula_n *work_done_list = malloc( sizeof( celula_n));


    bool work_left = true;

    char out[MAXLINE + 1];
    char buffer[MAXLINE + 1];

    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char recvline[MAXLINE + 1];
    ssize_t n;

    if ((listenfd = socket( AF_INET, SOCK_STREAM, 0) == -1)) {
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
        if ((connfd = accept( listenfd, (struct sockaddr *) NULL, NULL) == -1)) {
            fprintf(stderr, "ERROR: Could not accept connection, %s\n", strerror( errno));
            continue;
        }
        n = read( connfd, recvline, MAXLINE);
        recvline[n] = 0;

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
            busca_e_remove_lln( work_number, work_left_list);
            insere_lln( work_number, work_done_list);
        }
    }

    free( work_left_list);
    return;
}
