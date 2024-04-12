#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "writeInput.h"


int createFifo (char* nome) {
    
    char* path = malloc(strlen(nome) + strlen("../tmp/") + 2);
    strcpy(path, "../tmp/");
    strcat(path, nome);
    printf("%s", path);
    
    if (mkfifo(path, 0666) == -1) {
        perror("mkfifo");
        return 1;   
    }
    
    return 0;
}