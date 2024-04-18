#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

Queue* newQueue () {
    Queue* aux = malloc(sizeof(Queue));
    aux->next = NULL;
    aux->tamanho = 0;
    return aux;
}

void enQueue (Queue* fila, Msg toInsert) {
    if (fila == NULL) {
        Queue* aux = newQueue();
        fila = aux;
    }
    if (fila->tamanho == 0) {
        fila->data = toInsert;
        fila->tamanho++;
    }
    else {
        Queue* copia = fila;
        while (copia->next != NULL) {
            copia = copia->next;
        }
        Queue* nova = malloc(sizeof(Queue));
        nova->data = toInsert;
        nova->next = NULL;
        copia->next = nova;
        fila->tamanho++;
    } 
    
}

void deQueue (Queue** fila) {
    if ((*fila)->tamanho == 1) {
        (*fila)->tamanho--;
        free(*fila);
        *fila = NULL;
    }

    else if (*fila != NULL) {
        Queue* aux = (*fila)->next;
        free(*fila);
        if ((*fila)->next != NULL) {
            *fila = aux;
            (*fila)->tamanho--;
        }   
    }

}

void printQueue (Queue* fila) {
    Queue* save = fila;
    while(fila != NULL) {
        printf("%s\n", fila->data.argumentos);
        fila = fila->next;
    }
    fila = save;
}