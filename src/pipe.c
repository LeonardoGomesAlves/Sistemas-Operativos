#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

char* createFifo (int pid) {

    char fifo_name[30];
    sprintf(fifo_name, "../tmp/CLIENT_%d", pid);
    
    if (mkfifo(fifo_name, 0666) == -1) {
        perror("mkfifo");
        return NULL;
    }
    

    char* toRet = malloc(strlen(fifo_name) + 1);
    strcpy(toRet, fifo_name);
    return toRet;
}