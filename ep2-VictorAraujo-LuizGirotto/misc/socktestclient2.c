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

int main(int argc, char const *argv[]) {
    /* Prepara socket pra comunicacao inicial com o serv */
    int sockfd_im;
    struct sockaddr_in servaddr_im;

    if ((sockfd_im = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf( stderr, "WK-ERROR: could not create socket, %s\n", strerror( errno));
        exit( EXIT_FAILURE);
    }

    bzero( &servaddr_im, sizeof( servaddr_im));
    servaddr_im.sin_family = AF_INET;
    servaddr_im.sin_port = htons( PORT);

    if (inet_pton( AF_INET, "127.0.0.1", &servaddr_im.sin_addr) != 1){
        fprintf( stderr, "WK-ERROR: Some problem with inet_pton\n");
        exit( EXIT_FAILURE);
    }

    while (connect( sockfd_im, (struct sockaddr *) &servaddr_im, sizeof( servaddr_im)) < 0) {
        fprintf( stderr, "WK-%s\n", strerror( errno));
    }

    /* Comunica-se com o serv */
    while (true) {
        printf("Sending.\n");
        write( sockfd_im, "GD2\r\n", 5 * sizeof( char));
        sleep (3);
    }

    close( sockfd_im);
    return 0;
}
