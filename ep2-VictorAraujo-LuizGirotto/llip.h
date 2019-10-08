#ifndef __LLIP_H__
#define __LLIP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

struct llip {
      char ip[INET_ADDRSTRLEN];
      struct llip *prox;
};

typedef struct llip celula_ip;

void insere_llip( char *ip, celula_ip *p);

void busca_e_remove_llip( char *ip, celula_ip *le);

void limpa_llip( celula_ip *le);

#endif
