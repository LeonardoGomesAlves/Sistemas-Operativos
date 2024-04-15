#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include "clientSingle.h"
#include "pipe.h"
#include "utilidades.h"
#include "orchestrator.h"


// Módulo para criação do servidor

int main (int argc, char* argv[]) {

    printf("Waiting for inputs....");

    //criar o pipe do server
    if (mkfifo(SERVER, 0666) == -1) {
        perror("mkfifo");
        return 1;   
    }

    Msg buf;
    ssize_t bytes_read;
    int fd_server;
    fd_server = open(SERVER, O_RDONLY);
    if (fd_server == -1) {
        perror("open");
        return 1;
    }


        while ((bytes_read = read(fd_server, &buf, sizeof(Msg))) > 0) {
            //SEG FAULT QUANDO MEXO NESTE BUFFER
            /* if (execvp(buf.argumentos[0], buf.argumentos) == -1) {
                    perror("execvp");
                    return 1;
            } */
        }

        /* bytes_read = read(fd_server, &buf, sizeof(Msg));
        if (bytes_read == -1) {
            perror("read");
            return 1;
        } else if (bytes_read == 0) {
            continue;
        }

        if (bytes_read > 0) {
            
        } */

    close(fd_server);


    return 0;
}