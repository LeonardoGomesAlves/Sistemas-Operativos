#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "clientSingle.h"
#include "orchestrator.h"
#include "pipe.h"
#include "utilidades.h"



int writeInPipe_Single (char* input) {

    Msg buf [BSIZE];
    int fd_server, fd_client;

    //ABRIR PIPE DO SERVER
    fd_server = open(SERVER, O_WRONLY);
    if (fd_server == -1) {
        perror("open");
        return -1;
    }

    
    //CRIAR PIPE DO CLIENT
    int pid = getpid();

    char* fd_client_path = createFifo(pid);
    /* printf("%s", fd_client_path);
    fd_client = open(fd_client_path, O_RDONLY);
    if (fd_client == -1) {
        perror("open");
        return -1;
    } */

    
    //INSERIR NA STRUCT MSG PARA ENVIAR PARA O PIPE 
    char** commands = malloc(300); //diz no enunciado que os argumentos não passam os 300 bytes
    int nargs = separa_argumentos(commands, input);
    Msg toPipe;

    int i;
    toPipe.argumentos = malloc(sizeof(char**) * nargs-1);

    for (i = 0; commands[i] != NULL; i++) {
        toPipe.argumentos[i] = commands[i];
    }
    toPipe.argumentos[i] = NULL;

    //toPipe.comando = commands[0];
    toPipe.pid_path = fd_client_path;

    //ENVIAR PARA O PIPE
    ssize_t bytes_written = write(fd_server, &toPipe, sizeof(Msg));
    close(fd_server);
    printf("Bytes escritos -> %d\n", bytes_written);

    //LÊ O OUTPUT QUE SERÁ ENVIADO PELO SERVER PARA O CLIENT

    Msg buffer;
    size_t bytes_read;
    /* while ((bytes_read = read(fd_client, &buffer, sizeof(Msg))) > 0); */




    /* for (int i = 0; input[i] != NULL; i++) {
        bytes_read = write(fd_server, input[i], strlen(input[i]));
        if (bytes_read == -1) {
            perror("write");
            close(fd_server);
            return -1;
        }
    } */

    

    return 0;
}
