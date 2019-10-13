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

#include <time.h>

int
getIP ( char *ip, int ipsize) {
    FILE *fd;
    size_t n = ipsize * sizeof( char);
    ssize_t cn;

    fd = fopen( "ep02wk.conf", "r");
    cn = getline( &ip, &n, fd);

    if (ip[cn-1] == '\n') {
        ip[cn-1] = '\0';
    }

    fclose( fd);
    return 1;
}

char *getImmortalIP() {
    FILE *fd;
    char *ip = NULL;
    size_t n = 0;
    fd = fopen( "ep02.conf", "r");
    getline( &ip, &n, fd);
    fclose( fd);
    return ip;
}

int msleep(long msec) {
    struct timespec ts;
    int res;

    if (msec < 0) {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

void log_datetime(FILE *log) {
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf( log, "[%s] ", buffer);

}
