#ifndef QUEUE_H
#define QUEUE_H
#include "gestorClient.h"

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

void handleQueue (Msg toExecute, char* server_output_info);

void handleMultiple (Msg toExecute, char* server_output_info);

int handleClientStatus(Msg toRead, char* in_execution, char* server_info, Queue* fila);

int exec_task (Msg toExecute, char* in_execution, char* output_folder);

#endif