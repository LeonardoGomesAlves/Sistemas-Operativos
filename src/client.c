#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include "gestorClient.h"

#define SINGLE 0
#define MULTIPLE 1
#define STATUS 2

//Módulo para criação do client

int main (int argc, char* argv[]) {

    if(argc == 1) {
        char* input_inv = malloc(100);
        sprintf(input_inv, "Input inválido.\n./client status\n./client execute 100 (-u/-p) ''prog-a arg-1 (...) arg-n''\n");
        write(1,input_inv, sizeof(input_inv));
        free(input_inv);
        return 0;
    }

    //execução de um comando
    if (strcmp(argv[1], "execute") == 0) {

        char* input = malloc(strlen(argv[4]));
        strcpy(input, argv[4]);
    
        //um unico processo
        if (strcmp(argv[3], "-u") == 0) {    
            writeInPipe(input,SINGLE);
        }
        //varios processos
        else if (strcmp(argv[3], "-p") == 0) {
            writeInPipe(input,MULTIPLE);
        }

        else {
            char* input_inv = malloc(100);
            sprintf(input_inv, "Input inválido.\n./client status\n./client execute 100 (-u/-p) ''prog-a arg-1 (...) arg-n''\n");
            write(1,input_inv, sizeof(input_inv));
            free(input_inv);
            return 0;
        }       
    
    
    }

    //argc == 2, porque o status é utilizado sem argumentos
    else if (strcmp(argv[1], "status") == 0 && argc == 2) {
        char* input = malloc(strlen(argv[1]));
        strcpy(input, argv[1]);
        writeInPipe(input,STATUS);
        printf("debug cliente\n");
    
    }


    else {
        char* input_inv = malloc(100);
        sprintf(input_inv, "Input inválido.\n./client status\n./client execute 100 (-u/-p) ''prog-a arg-1 (...) arg-n''\n");
        write(1,input_inv, sizeof(input_inv));
        free(input_inv);
        return 0;
    }

    return 0;    
}