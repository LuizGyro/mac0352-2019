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


#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

int
leader() {
    FILE *fd;
    char buffer[MAXLINE];
    char work_number[MAXLINE];

    bool is_leader = true;
    bool receiving_jobs = false;

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
    servaddr.sin_port        = htons(LEADER_PORT);

    if ((bind( listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)) {
        fprintf( stderr, "ERROR: Could not bind socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    if (listen( listenfd, LISTENQ) == -1) {
        fprintf( stderr, "ERROR: Could not listen on port, %s\n", strerror( errno));
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
            write( connfd, "100\r\n", 5 * sizeof( char));
            /*recebe quem ta vivo*/
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
                close( connfd);
                close( listenfd);
                exit( EXIT_FAILURE);
            }
            write( connfd, "100\r\n", 5 * sizeof( char));
            while ((read( connfd, buffer, MAXLINE)) > 0) {
                fprintf( fd, "%s", buffer);
            }
            fclose( fd);

        }
        /*Não vai mais receber trabalho*/
        else if (!strncmp( recvline, "006\r\n", 5 * sizeof( char))) {
            receiving_jobs = false;
        }
        close( connfd);
    }

    close( listenfd);
    exit( EXIT_SUCCESS);

}
