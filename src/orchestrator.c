#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include "gestorClient.h"
#include "pipe.h"
#include "utilidades.h"
#include "orchestrator.h"
#include <stdlib.h>
#include "queue.h"

// Módulo para criação do servidor

int main (int argc, char* argv[]) {

    //EXEMPLO DE UTILIZAÇÃO: ./orchestrator ../<nome_da_pasta_deseja>/ <numero de paralel tasks> <FCFS ou SJF>
    if (argc != 4) {
        perror("argc");
        return 1;
    }

    if (strcmp("FCFS", argv[3]) != 0 && strcmp("SJF", argv[3]) != 0) {
        perror("FCFS ou SJF");
        return 1;
    }

    int algoritmo;

    if (strcmp("FCFS", argv[3]) == 0) {
        algoritmo = 0;
    } else {
        algoritmo = 1;
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
    ssize_t bytes_read;
    int fd_server;
    fd_server = open(SERVER, O_RDONLY);

    if (fd_server == -1) {  
        perror("open");
        return 1;
    }

    //criação do ficheiro para guardar info no servidor
    char* server_info = malloc(strlen("../tmp/SERVER_INFO") + 1);
    sprintf(server_info, "../tmp/SERVER_INFO");


    //criação do ficheiro das queries completadas
    int server_output_info = open(server_info, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (server_output_info == -1) {
        perror("open");
        return 1;
    }

    int len1 = snprintf(NULL, 0, "\nCompleted\n");
    char* completed = malloc(len1 + 1);
    if (completed != NULL) {
        sprintf(completed, "\nCompleted\n");
        write(server_output_info, completed, len1);
        free(completed);
    }

    //criação do ficheiro das queries em execução
    char* in_execution = malloc(strlen("../tmp/IN_EXECUTION") + 1);
    sprintf(in_execution, "../tmp/IN_EXECUTION");

    int server_in_execution = open(in_execution, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (server_in_execution == -1) {
        perror("open");
        return 1;
    }

    int len3 = snprintf(NULL, 0, "Executing\n");
    char* completed_2 = malloc(len3 + 1);
    if (completed_2 != NULL) {
        sprintf(completed_2, "Executing\n");
        write(server_in_execution, completed_2, len3);
        free(completed_2);
    }

    int n_tasks = 1;

    Queue* fila = malloc(sizeof(Queue));
    newQueue(fila);
    
    int count = 0;

    int available = open("../tmp/available", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    close(available);

    while(1) {
        while ((bytes_read = read(fd_server, &toRead, sizeof(Msg))) > 0) {
            //SE O REQUEST FOR UM STATUS
            if (toRead.tipo == 2) {

                handleClientStatus(toRead, in_execution, server_info, fila);

            } else if (toRead.tipo == 3){
                return 1;
            } else {
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

                if (algoritmo == 0) {
                    enQueue(fila, toRead);
                } else {
                    enQueueSJF(fila, toRead);
                }                                       
            }
        }

        int available2 = open("../tmp/available", O_RDWR);


        char buf[10];
        count = read(available2, &buf, sizeof(buf));
        
        //disponivel
        if (count == 0) {
            if (fila->head != NULL) {
                count++;
                char* towrite = malloc(strlen("n")+1);
                strcpy(towrite, "n");
                write(available2, towrite, strlen(towrite) + 1);
        
                
                Msg toExecute = fila->head->data;

                int filho = fork();
                if (filho == -1) {
                    perror("fork");
                    return 1;
                }

                if (filho == 0) {
                    close(available2);

                    exec_task(toExecute, in_execution, argv[1]);                    
                    
                    _exit(0);
                } else {
                    deQueue(fila);
                }
            }
        }
        close(available2);          
    }       
        close(fd_server);
        close(server_output_info);
        free(server_info);


    return 0;

}