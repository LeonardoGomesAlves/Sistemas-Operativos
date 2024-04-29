#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "gestorClient.h"
#include "orchestrator.h"
#include "pipe.h"
#include "utilidades.h"
#include "queue.h"



int writeInPipe (char* input,int tipo) {

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

    toPipe.tipo = tipo;

    strcpy(toPipe.argumentos, input);

    strcpy(toPipe.pid_path, fd_client_path);

    //ENVIAR PARA O PIPE
    write(fd_server, &toPipe, sizeof(toPipe));
    close(fd_server);

    //LÊ O OUTPUT QUE SERÁ ENVIADO PELO SERVER PARA O CLIENT
    //O FD_CLIENT TEM QUE SER ABERTO AQUI!!
    fd_client = open(fd_client_path, O_RDONLY);
    if (fd_client == -1) {
        perror("open");
        return -1;
    } 

    size_t bytes_read;
    //APENAS DIZ QUE RECEBEU A QUERY
    if (tipo != 2) {
        Msg buffer;
        while ((bytes_read = read(fd_client, &buffer, sizeof(Msg))) > 0) {
            write(1, buffer.response, strlen(buffer.response));
        }  
    } else { //TRATA DE IMPRIMIR NA CONSOLA O ./client status
        char buffer[4096];
        while((bytes_read = read(fd_client, &buffer, sizeof(buffer))) > 0);
        int fd_status = open("../tmp/status", O_RDONLY);
        if (fd_status == -1) {
            perror("open");
            return 1;
        }
        char status_buf[4096];
        ssize_t bytes_status;
        while((bytes_status = read(fd_status, status_buf, sizeof(status_buf))) > 0) {
            write(1, status_buf, bytes_status);
        }

        int in_execution = open("../tmp/IN_EXECUTION", O_WRONLY | O_TRUNC);
        if (in_execution == -1) {
            perror("open");
            return 1;
        }

        int len = snprintf(NULL, 0, "Executing\n");
        char* to_execute_output = malloc(len + 1);
        if (to_execute_output != NULL) {
            sprintf(to_execute_output, "Executing\n");
            write(in_execution, to_execute_output, len);
            free(to_execute_output);
        }

        close(in_execution);
        close(fd_status);
    }


    close(fd_client);

    return 0;
}
      

    

  
