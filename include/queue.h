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

void enQueueFCFS (Queue* fila, Msg toInsert);

void enQueueSJF (Queue* fila, Msg toInsert);

void deQueue (Queue* fila);

void printQueue (Queue* fila);

void executeSingle (Msg toExecute, char* server_output_info);

void executePipeline (Msg toExecute, char* server_output_info);

int handleClientStatus(Msg toRead, int paralel_tasks, char* server_info, Queue* fila);

int exec_task (Msg toExecute, int in_execution_id, char* output_folder, char* process);

#endif