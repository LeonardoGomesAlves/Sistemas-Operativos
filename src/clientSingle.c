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
       
    //INSERIR NA STRUCT MSG PARA ENVIAR PARA O PIPE 
    Msg toPipe;

    strcpy(toPipe.argumentos, input);

    strcpy(toPipe.pid_path, fd_client_path);

    //ENVIAR PARA O PIPE
    write(fd_server, &toPipe, sizeof(toPipe));
    close(fd_server);

    //LÊ O OUTPUT QUE SERÁ ENVIADO PELO SERVER PARA O CLIENT
    //O FD_CLIENT TEM QUE SER ABERTO AQUI!!
    /* fd_client = open(fd_client_path, O_RDONLY);
    if (fd_client == -1) {
        perror("open");
        return -1;
    } 

    Msg buffer;
    size_t bytes_read;
    while ((bytes_read = read(fd_client, &buffer, sizeof(Msg))) > 0) {
        write(1, buffer.response, strlen(buffer.response));
    }  
    close(fd_client); */



    return 0;
}
      

    

  
