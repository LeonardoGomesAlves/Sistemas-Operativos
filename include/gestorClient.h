#ifndef CLIENT_GESTOR_H
#define CLIENT_GESTOR_H
#define BSIZE 50

typedef struct msg {
    char argumentos[300];
    int tipo;
    char pid_path[30];
    char response[25];
    int n_task;
    int tempo;
    //utilizado para calcular os tempos de espera e medios
    /* struct timeval tempo_inicial;
    struct timeval tempo_final; */
} Msg;

int writeInPipe (char* input, int tipo, int tempo);

#endif