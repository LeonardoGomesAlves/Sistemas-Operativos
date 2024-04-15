#ifndef CLIENT_SINGLE
#define CLIENT_SINGLE
#define BSIZE 50

typedef struct msg {
    char* comando;
    char** argumentos;
    char* pid_path;
} Msg;

int writeInPipe_Single (char* input);

#endif