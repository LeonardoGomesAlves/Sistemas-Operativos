#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

void newQueue (Queue* aux) {
    aux->head = NULL;
    aux->tail = NULL;
}

void enQueue (Queue* fila, Msg toInsert) {
    Node* newNode = malloc(sizeof(Node));
    if (!newNode) {
        perror("malloc");
        return;
    }
    newNode->data = toInsert;
    newNode->next = NULL;

    if (fila->tail != NULL) {
        fila->tail->next = newNode;
    }
    fila->tail = newNode;

    //fila vazia
    if (fila->head == NULL) {
        fila->head = newNode;
    }

    fila->tamanho++;
    
}

void deQueue (Queue* fila) {
    //fila vazia
    if (fila->head != NULL) {
        fila->head = fila->head->next;

        //se a fila enventualmente ficar vazia, a tail tmb fica vazia
        if (fila->head == NULL) {
            fila->tail = NULL;
        }
        fila->tamanho--;
    }  
}

void printQueue (Queue* fila) {
    Node* atual = fila->head;
    while(atual != NULL) {
        printf("%s\n", atual->data.argumentos);
        atual = atual->next;
    }
}