#ifndef QUEUE_H
#define QUEUE_H
#include "clientSingle.h"

typedef struct queue {
    Msg data;
    struct queue* next;
    int tamanho;
} Queue;

Queue* newQueue ();

void enQueue (Queue* fila, Msg toInsert);

void deQueue (Queue** fila);

void printQueue (Queue* fila);

#endif