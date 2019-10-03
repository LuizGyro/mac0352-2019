#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

struct ll {
      char ip[INET_ADDRSTRLEN];
      struct ll *prox;
};

typedef struct ll celula;

void insere (char *ip, celula *p) {
   celula *nova;
   nova = malloc (sizeof (celula));
   strcpy(nova->ip, ip);
   nova->prox = p->prox;
   p->prox = nova;
}

void busca_e_remove (char *ip, celula *le) {
   celula *p, *q;
   p = le;
   q = le->prox;
   while (q != NULL && strcmp(q->ip, ip) != 0) {
      p = q;
      q = q->prox;
   }
   if (q != NULL) {
      p->prox = q->prox;
      free (q);
   }
}
