#ifndef CLIENT_SINGLE
#define CLIENT_SINGLE
#define BSIZE 50

typedef struct msg {
    char argumentos[300];
    char pid_path[30];
    char response[25];
    int n_task;
} Msg;

int writeInPipe_Single (char* input);

#endif