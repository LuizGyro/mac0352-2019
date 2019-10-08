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

#include <sys/stat.h>

int
getIP ( char *ip, int ipsize) {
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

char *getImmortalIP() {
    FILE *conf;
    char *ip;
    struct stat *file_mdata = malloc( sizeof( struct stat));

    conf = fopen( "ep02.conf", "r");
    stat( conf, file_mdata);
    ip = malloc( file_mdata->st_size);
    fread( ip, 1, file_mdata->st_size, conf);
    return ip;

}
