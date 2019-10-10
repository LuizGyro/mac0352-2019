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

#define PORT 20000
#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

int main(int argc, char const *argv[]) {
    /*Só roubei do EP01*/
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    ssize_t n;
    char recvline[MAXLINE + 1];

    if ((listenfd = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf( stderr, "WK-ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr, sizeof( servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(PORT);

    if (bind( listenfd, (struct sockaddr *) &servaddr, sizeof( servaddr)) == -1) {
        fprintf( stderr, "WK-ERROR: Could not bind socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    if (listen( listenfd, LISTENQ) == -1) {
        fprintf( stderr, "WK-ERROR: Could not listen on port, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }
    while (true) {
        printf("Waiting for connection.\n");
        if ((connfd = accept( listenfd, (struct sockaddr *) NULL, NULL)) == -1) {
            fprintf( stderr, "WK-ERROR: Could not accept connection, %s\n", strerror( errno));
            continue;
        }
        while ((n = read( connfd, recvline, MAXLINE)) > 0) {
            recvline[n] = 0;
            printf("Recebi: %s", recvline);
            if (!strncmp( recvline, "GD1\r\n", 5 * sizeof( char))) {
                printf("1 falou\n");
            }
            if (!strncmp( recvline, "GD2\r\n", 5 * sizeof( char))) {
                printf("2 falou\n");
            }
        }
        close( connfd);
    }
    close( listenfd);
    return 0;
}
