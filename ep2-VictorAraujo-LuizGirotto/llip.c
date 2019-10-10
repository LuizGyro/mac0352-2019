#include "llip.h"

void insere_llip( char *ip, celula_ip *p) {
   celula_ip *nova;
   nova = malloc (sizeof (celula_ip));
   strcpy(nova->ip, ip);
   nova->prox = p->prox;
   p->prox = nova;
}

void busca_e_remove_llip( char *ip, celula_ip *le) {
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

void limpa_llip( celula_ip *le) {
    celula_ip *p, *q;
    if (le->prox == NULL) {
        return;
    }
    p = le->prox;
    q = p->prox;
    while (q != NULL) {
        free(p);
        p = q;
        q = q->prox;
    }
    free(p);
    le->prox = NULL;
}
