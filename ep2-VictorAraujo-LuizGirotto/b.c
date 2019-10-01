#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
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

       int main(int argc, char *argv[])
       {
           int s;
           struct ifconf ifconf;
           struct ifreq ifr[50];
           int i = 0;

           char ip[INET_ADDRSTRLEN];

           int sockfd, n;
           struct sockaddr_in servaddr;

           char variable_ip[3];

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

           if (!inet_ntop(AF_INET, &s_in->sin_addr, ip, sizeof(ip))) {
             perror("inet_ntop");
             return 0;
           }

           printf("%s\n", ip);
           for (i = 0; i < 256; i++) {
               sprintf(variable_ip, "%d", i);

               if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                  fprintf(stderr,"socket error :( \n");

               bzero(&servaddr, sizeof(servaddr));
               servaddr.sin_family = AF_INET;
               servaddr.sin_port = htons(21);  /* FTP padrao roda na 21 */

               if (inet_pton(AF_INET, variable_ip, &servaddr.sin_addr) <= 0)
                  fprintf(stderr,"inet_pton error for %s :(\n", argv[1]);

               if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
                  fprintf(stderr,"connect error :(\n");
           }

           return 0;
       }
