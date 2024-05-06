#ifndef CLIENT_GESTOR_H
#define CLIENT_GESTOR_H
#define BSIZE 50

#include <sys/time.h>

typedef struct msg {
    char argumentos[300];
    int tipo;
    char pid_path[30];
    char response[25];
    int n_task;
    int tempo;
    struct timeval inicio;
} Msg;

int writeInPipe (char* input, int tipo, int tempo);

#endif