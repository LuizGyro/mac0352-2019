#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

struct lln {
      int workn;
      struct lln *prox;
};

typedef struct lln celula_n;

void insere (int n, celula_n *p) {
   celula_n *nova;
   nova = malloc (sizeof (celula_n));
   nova->workn = n;
   nova->prox = p->prox;
   p->prox = nova;
}

void busca_e_remove (int n, celula_n *le) {
   celula_n *p, *q;
   p = le;
   q = le->prox;
   while (q != NULL && q->workn != n) {
      p = q;
      q = q->prox;
   }
   if (q != NULL) {
      p->prox = q->prox;
      free (q);
   }
}
