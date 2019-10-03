#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#define LISTENQ 1
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
           int i = 0, j, j_beg, k;

           char ip[INET_ADDRSTRLEN];

           int sockfd, n;
           struct sockaddr_in servaddr;
           char variable_ip[3];

           int listenfd, connfd;

           /* Encontra IP Proprio */
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
           for (j = INET_ADDRSTRLEN - 1; j > 0; j--)
            if (isdigit(ip[j])) {
                j_beg = j;
                break;
            }

           if (argc > 1) {
               /* Faz o "multicast" */
               for (i = 100; i < 256; i++) {
                   sprintf(variable_ip, "%d", i);
                   printf("VIP: %s\n", variable_ip);
                   for (j = j_beg, k = 2; j > j_beg - 3; j--, k--) {
                       if (isdigit(variable_ip[k]))
                        ip[j] = variable_ip[k];
                       else
                        ip[j] = '\0';
                   }
                   ip[j_beg+1] = '\0';

                   if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                      fprintf(stderr,"socket error :( \n");

                   bzero(&servaddr, sizeof(servaddr));
                   servaddr.sin_family = AF_INET;
                   servaddr.sin_port = htons(6000);  /* Escolhemos rodar na porta 6000 */

                   printf("IP: %s\n", ip);
                   if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0)
                      fprintf(stderr,"inet_pton error for %s :(\n", ip);

                   if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
                      fprintf(stderr,"connect error :(\n");
               }

           }
           else {
               /* Recebe o multicast */
               if ((listenfd = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
                   perror( "socket :(\n");
                   exit( 2);
               }

               bzero( &servaddr, sizeof(servaddr));
               servaddr.sin_family      = AF_INET;
               servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
               servaddr.sin_port        = htons(6000);
               if (bind( listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
                   perror( "bind :(\n");
                   exit( 3);
               }

               if (listen( listenfd, LISTENQ) == -1) {
                   perror( "listen :(\n");
                   exit( 4);
               }

               for (;;) {
                   if ((connfd = accept( listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
                      perror( "accept :(\n");
                      exit( 5);
                   }
               }


           }
           return 0;
       }
