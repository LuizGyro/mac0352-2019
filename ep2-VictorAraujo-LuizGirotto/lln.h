#ifndef __LLN_H__
#define __LLN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>


struct lln {
      int workn;
      struct lln *prox;
};

typedef struct lln celula_n;

void insere_lln( int n, celula_n *p);

void busca_e_remove_lln( int n, celula_n *le);

void limpa_lln( celula_n *le);

#endif
