#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include "clientSingle.h"



//Módulo para criação do client

int main (int argc, char* argv[]) {

    if(argc == 1) {
        printf("Input inválido.\n./client status\n./client execute 100 (-u/-p) ''prog-a arg-1 (...) arg-n''\n");
        return 0;
    }

    //execução de um comando
    if (strcmp(argv[1], "execute") == 0) {

        char* input = malloc(strlen(argv[4]));
        strcpy(input, argv[4]);
    
        //um unico processo
        if (strcmp(argv[3], "-u") == 0) {           
            writeInPipe_Single(input);
        }
        //varios processos
        else if (strcmp(argv[3], "-p") == 0) {
            
            printf("debug programas\n");

        }

        else {
            printf("Input inválido.\n./client status\n./client execute 100 (-u/-p) ''prog-a arg-1 (...) arg-n''\n");

        }       
    
    
    }

    //argc == 2, porque o status é utilizado sem argumentos
    else if (strcmp(argv[1], "status") == 0 && argc == 2) {
        printf("debug cliente\n");
    
    
    
    
    }


    else printf("Input inválido.\n./client status\n./client execute 100 (-u/-p) ''prog-a arg-1 (...) arg-n''\n");

    return 0;    
}