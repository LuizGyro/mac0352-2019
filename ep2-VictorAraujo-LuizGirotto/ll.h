#ifndef __LL_H__
#define __LL_H__

struct ll {
      char ip[INET_ADDRSTRLEN];
      struct ll *prox;
};

typedef struct ll celula;

void insere (char ip, celula *p);

void busca_e_remove (char ip, celula *le);

#endif
