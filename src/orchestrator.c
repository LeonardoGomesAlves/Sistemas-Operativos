#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include "writeInput.h"
#include "pipe.h"
#include "utilidades.h"


// Módulo para criação do servidor

int main (int argc, char* argv[]) {

    printf("Waiting for inputs....");

    createFifo("fifo");

    char buf[BSIZE];
    ssize_t bytes_read;
    int fd;
    fd = open("../tmp/fifo", O_RDONLY);

        while ((bytes_read = read(fd, buf, BSIZE)) > 0) {
                buf[bytes_read] = '\0';
        }
    
        char* commands[BSIZE];
        separa_argumentos(commands,buf);

       if(execvp(commands[0],commands) == -1){
           perror("execvp");
           return -1;
       }
    
        

    close(fd);


    return 0;
}