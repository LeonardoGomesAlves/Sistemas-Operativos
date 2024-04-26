#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include "clientSingle.h"
#include "pipe.h"
#include "utilidades.h"
#include "orchestrator.h"
#include <stdlib.h>
#include "queue.h"


// Módulo para criação do servidor

int main (int argc, char* argv[]) {

    //EXEMPLO DE UTILIZAÇÃO: ./orchestrator ../<nome_da_pasta_deseja>/

    if (argc != 2) {
        perror("argc");
        return 1;
    }

    //criar o pipe do server
    if (mkfifo(SERVER, 0666) == -1) {
        perror("mkfifo");
        return 1;   
    }

    //criar repositório do argv, caso não exista
    mkdir(argv[1], 0700);

    //abrir pipe do server
    Msg toRead;
    Msg buf;
    ssize_t bytes_read;
    int fd_server;
    int status;
    fd_server = open(SERVER, O_RDONLY);

    if (fd_server == -1) {  
        perror("open");
        return 1;
    }

    //criação do ficheiro para guardar info no servidor
    char* server_info = malloc(strlen("../tmp/SERVER_INFO") + 1);
    sprintf(server_info, "../tmp/SERVER_INFO");

    int server_output_info = open(server_info, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (server_output_info == -1) {
        perror("open");
        return 1;
    }


    int n_tasks = 1;

    Queue* fila = malloc(sizeof(Queue));
    newQueue(fila);

    int pipefd[2];
    if (pipe(pipefd) == -1){
        perror("pipe");
        return 1;
    }

    int pid_geral = fork();

    if (pid_geral == -1) {
        perror("fork");
        return 1;
    }

    //filho
    if (pid_geral == 0) {
        close(pipefd[1]);
        while (1) {          

            Msg toExecute;
            ssize_t bytes_read = read(pipefd[0], &toExecute, sizeof(Msg));
            //enQueue(fila, toExecute);

            //filho
            if (bytes_read > 0) {
                handleQueue(toExecute, argv[1]);
            }

        }
        close(pipefd[0]);
    }
    else {
        close(pipefd[0]);
        while(1) {
            while ((bytes_read = read(fd_server, &toRead, sizeof(Msg))) > 0) {
                
                //ENVIA MENSAGEM PARA O CLIENT
                toRead.n_task = n_tasks++;
                sprintf(toRead.response, "TASK %d Received\n", toRead.n_task);
                
                int fd_client = open(toRead.pid_path, O_WRONLY);
                if (fd_client == -1) {
                    perror("open");
                    return 1;
                }

                write(fd_client, &toRead, sizeof(toRead));
                close(fd_client);

                enQueue(fila, toRead);
                write(pipefd[1], &fila->head->data, sizeof(Msg));
                //printf("%d\n", fila->tamanho);
                deQueue(fila);
            }
        }        
        close(pipefd[1]);
        wait(NULL);
    }

        close(fd_server);
        close(server_output_info);
        free(server_info);


    return 0;

}