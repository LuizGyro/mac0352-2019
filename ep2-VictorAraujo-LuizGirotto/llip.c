#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

struct llip {
      char ip[INET_ADDRSTRLEN];
      struct llip *prox;
};

typedef struct llip celula_ip;

void insere (char *ip, celula_ip *p) {
   celula_ip *nova;
   nova = malloc (sizeof (celula_ip));
   strcpy(nova->ip, ip);
   nova->prox = p->prox;
   p->prox = nova;
}

void busca_e_remove (char *ip, celula_ip *le) {
   celula_ip *p, *q;
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
