#include "lln.h"

void insere_lln( int n, celula_n *p) {
    celula_n *nova;
    nova = malloc( sizeof( celula_n));
    nova->workn = n;
    nova->prox = p->prox;
    p->prox = nova;
}

void busca_e_remove_lln( int n, celula_n *le) {
    celula_n *p, *q;
    p = le;
    q = le->prox;
    while (q != NULL && q->workn != n) {
        p = q;
        q = q->prox;
    }
    if (q != NULL) {
        p->prox = q->prox;
        free ( q);
    }
}

void limpa_lln( celula_n *le) {
    celula_n *p, *q;
    if (le->prox == NULL) {
        return;
    }
    p = le->prox;
    q = p->prox;
    while (q != NULL) {
        free( p);
        p = q;
        q = q->prox;
    }
    free( p);
    le->prox = NULL;
}
