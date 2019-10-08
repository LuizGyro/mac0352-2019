#ifndef __LLN_H__
#define __LLN_H__

struct lln {
      char ip[INET_ADDRSTRLEN];
      struct lln *prox;
};

typedef struct lln celula_n;

void insere (int n, celula_n *p);

void busca_e_remove (int n, celula_n *le);

void limpa (celula_n *le);

#endif
