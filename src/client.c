#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

//Módulo para criação do client

int main (int argc, char* argv[]) {

    if(argc == 1) {
        printf("./client status\n./client execute 100 -u prog-a arg-1 (...) arg-n\n");
        return 0;
    }
    
    if (strcmp(argv[1], "execute") == 0) {
        printf("debug\n");
    
    
    
    
    }

    //argc == 2 porque o status é utilizado sem argumentos
    else if (strcmp(argv[1], "status") == 0 && argc == 2) {
        printf("debug cliente\n");
    
    
    
    
    }

    else printf("Input inválido.\n./client status\n./client execute 100 -u prog-a arg-1 (...) arg-n\n");

    return 0;    
}