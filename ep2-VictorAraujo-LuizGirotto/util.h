#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>

#define IMMORTAL_PORT 40000
#define LEADER_PORT 50000
#define WORKER_PORT 60000

#define DEBUG 0


/* Esta função devolve o endereço IP local da maquina na qual o
** programa esta rodando, em formato IPv4 */
int getIP(char *ip, int ipsize);

/* Esta função devolve o endereço IP do computador imortal, definido
** no arquivo de configuracao, em formato IPv4 */
char *getImmortalIP();

/* Sleeps for msec miliseconds */
int msleep(long msec);

/* Prints date and time */
void log_datetime(FILE *log);

#endif
