#ifndef QUEUE_H
#define QUEUE_H
#include "clientSingle.h"

typedef struct node {
    Msg data;
    struct node *next;
} Node;

typedef struct queue {
    Node *head;
    Node *tail;
    int tamanho;
} Queue;

void newQueue (Queue* aux);

void enQueue (Queue* fila, Msg toInsert);

void deQueue (Queue* fila);

void printQueue (Queue* fila);

#endif