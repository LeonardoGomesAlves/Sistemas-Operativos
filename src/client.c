#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "gestorClient.h"

#define SINGLE 0
#define MULTIPLE 1
#define STATUS 2
#define KILL 3

//Módulo para criação do client

void print_erro() {
    char* input_inv = malloc(100);
    sprintf(input_inv, "Input inválido.\n./client status\n./client kill\n./client execute 100 (-u/-p) ''prog-a arg-1 (...) arg-n''\n");
    write(1,input_inv, strlen(input_inv));
    free(input_inv);
}

int main (int argc, char* argv[]) {

    if(argc == 1) {
        print_erro();
        return 1;
    }

    //argc == 2, porque o status é utilizado sem argumentos
    if (strcmp(argv[1], "status") == 0 && argc == 2) {
        char* input = malloc(strlen(argv[1]));
        strcpy(input, argv[1]);
        writeInPipe(input,STATUS,0);
        free(input);
        return 1;
    } 

    //responsavel por terminar o programa
    if (strcmp(argv[1], "kill") == 0 && argc == 2) {
        char* input = malloc(strlen(argv[1]));
        strcpy(input, argv[1]);
        writeInPipe(input,KILL,0);
        free(input);
        return 1;
    }

    //se o argc == 2 apos o anterior, entao é invalido
    if (argc == 2) {
        print_erro();
        return 1;
    }

    for (int i = 0; i < strlen(argv[2]); i++) {
        if (!isdigit(argv[2][i])) {
            print_erro();
            return 1;
        }
    }

    //execução de um comando
    if (strcmp(argv[1], "execute") == 0) {

        char* input = malloc(strlen(argv[4]));
        strcpy(input, argv[4]);
    
        //um unico processo
        if (strcmp(argv[3], "-u") == 0) {    
            writeInPipe(input,SINGLE,atoi(argv[2]));
        }
        //varios processos
        else if (strcmp(argv[3], "-p") == 0) {
            writeInPipe(input,MULTIPLE,atoi(argv[2]));
        }

        else {
            print_erro();
            free(input); 
            return 1;
        }      
        free(input); 
    }

    else {
        print_erro();
        return 1;
    }

    return 0;    
}