#ifndef __LLIP_H__
#define __LLIP_H__

struct llip {
      char ip[INET_ADDRSTRLEN];
      struct llip *prox;
};

typedef struct llip celula_ip;

void insere (char ip, celula_ip *p);

void busca_e_remove (char ip, celula_ip *le);

#endif
