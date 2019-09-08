#include "util.h"
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>

#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <linux/netdevice.h>
#include <stropts.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

int
getFiles( char *path, char *buffer, int max_size) {
    DIR *current;
    strcpy( buffer, "\0");
    current = opendir( path);
    if (current == NULL) return 1;
    for (struct dirent *curr_file = readdir( current); curr_file != NULL; curr_file = readdir( current))
        if (strcmp( curr_file->d_name, ".") && strcmp( curr_file->d_name, "..")) {
            strncat( buffer, curr_file->d_name, max_size);
            strncat( buffer, "\r\n", max_size);
        }
    closedir( current);
    return 0;
}

int
openPort( int minPort, int maxPort, int *socketId) {
    struct sockaddr_in servaddr;
    int result = -1;
    int i;
    if ((*socketId = socket( AF_INET, SOCK_STREAM, 0)) == -1) return 1;

    bzero( &servaddr, sizeof( servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl( INADDR_ANY);

    for (i = minPort; i < maxPort; i++) {
        servaddr.sin_port = htons(i);
        result = bind( *socketId, (struct sockaddr *)&servaddr, sizeof( servaddr));
        if (result == 0) break;
    }

    if (result == -1) {
        close( *socketId);
        return 1;
    }
    result = listen( *socketId, 1);
    if (result == -1) {
        close( *socketId);
        return 1;
    }
    return servaddr.sin_port;
}

int
getName( char *input, char *output, int max_size) {
    int i = 5;
    while (input[i] >= 32 && input[i] <= 126) {
        if (i >= max_size - 1) {
            return 1;
        }
        output[i - 5] = input[i];
        i++;
    }
    if (i == 5) return 1;
    output[i - 5] = '\0';
    return 0;
}

int
getHostIP ( char *ip, int ipsize) {
    int s;
    struct ifconf ifconf;
    struct ifreq ifr[50];
    int i = 0;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
      perror("socket");
      return 0;
    }

    ifconf.ifc_buf = (char *) ifr;
    ifconf.ifc_len = sizeof ifr;

    if (ioctl(s, SIOCGIFCONF, &ifconf) == -1) {
      perror("ioctl");
      return 0;
    }

    while (strncmp(ifr[i].ifr_name, "wlp1s0", 6)){
        i++;
    }
    struct sockaddr_in *s_in = (struct sockaddr_in *) &ifr[i].ifr_addr;

    if (!inet_ntop(AF_INET, &s_in->sin_addr, ip, ipsize)) {
      perror("inet_ntop");
      return 0;
    }

    return 1;
}
