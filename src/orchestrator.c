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

#define AVAILABLE "A"

#define UNAVAILABLE "U"


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

    int len1 = snprintf(NULL, 0, "\nCompleted\n");
    char* completed = malloc(len1 + 1);
    if (completed != NULL) {
        sprintf(completed, "\nCompleted\n");
        write(server_output_info, completed, len1);
        free(completed);
    }

    //criação do ficheiro 
    char* in_execution = malloc(strlen("../tmp/IN_EXECUTION") + 1);
    sprintf(in_execution, "../tmp/IN_EXECUTION");

    //open(SERVER, O_WRONLY);


    int n_tasks = 1;

    Queue* fila = malloc(sizeof(Queue));
    newQueue(fila);

    int pipefd[2];
    if (pipe(pipefd) == -1){
        perror("pipe");
        return 1;
    }

    if(mkfifo("../tmp/fifo_queue", 0666) == -1) {
        perror("mkfifo");
        return 1;
    }

    /* int fifos;
    if((fifos = open("../tmp/fifo_queue", O_RDWR)) == -1){
        perror("open");
        return 1;
    } */


    /* int pipe_available[2];
    if (pipe(pipe_available) == -1){
        perror("pipe");
        return 1;
    }
    fcntl(pipe_available[0], F_SETFL, O_NONBLOCK);
    fcntl(pipe_available[1], F_SETFL, O_NONBLOCK); */


    int pid_geral = fork();

    if (pid_geral == -1) {
        perror("fork");
        return 1;
    }

    //filho
    if (pid_geral == 0) {
        close(pipefd[1]);
        close(fd_server);
        //write(pipe_available[1], AVAILABLE, strlen(AVAILABLE) + 1);
        while (1) {      
            //ESTA PARTE TRATA DE ESCREVER NO FICHEIRO PARA O CLIENT STATUS    

            int server_execution = open(in_execution, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (server_execution == -1) {
                perror("open");
                return 1;
            }

        
            int len = snprintf(NULL, 0, "Executing\n");
            char* to_execute_output = malloc(len + 1);
            if (to_execute_output != NULL) {
                sprintf(to_execute_output, "Executing\n");
                write(server_execution, to_execute_output, len);
                free(to_execute_output);
            }
            Msg toExecute;
            ssize_t bytes_read = read(pipefd[0], &toExecute, sizeof(Msg));

            //filho
            if (bytes_read > 0) {
                //PARA O STATUS - ESCREVE QUE ESTÁ A EXECUTAR
                int len = snprintf(NULL, 0, "%d %s\n", toExecute.n_task, toExecute.argumentos);
                char* to_execute_output = malloc(len + 1);
                if (to_execute_output != NULL) {
                    sprintf(to_execute_output, "%d %s\n", toExecute.n_task, toExecute.argumentos);
                    write(server_execution, to_execute_output, len);
                    free(to_execute_output);
                }
                if(toExecute.tipo == 0){
                    handleQueue(toExecute, argv[1]);
                }
                else{
                    handleMultiple(toExecute,argv[1]);
                }
            }
            close(server_execution);
        }
        //close(pipefd[0]);
        //close(pipe_available[1]);
    }else {
       int count = 0;

       int available = open("../tmp/available", O_WRONLY | O_CREAT | O_TRUNC, 0644);
       close(available);
        //close(pipefd[0]);
        //usar o open(fd_server, O_WRONLY);
        while(1) {
            while ((bytes_read = read(fd_server, &toRead, sizeof(Msg))) > 0) {
                //SE O REQUEST FOR UM STATUS
                if (toRead.tipo == 2) {
                    char buffer[4096];
                    char buffer_complete[4096];

                    int status_info = open("../tmp/status", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (status_info == -1) {
                        perror("open");
                        return 1;
                    }
                    

                    int fd_client = open(toRead.pid_path, O_WRONLY);
                    if (fd_client == -1) {
                        perror("open");
                        return 1;
                    }

                    ssize_t reading;

                    //EM EXECUÇÃO
                    int server_execution = open(in_execution, O_RDONLY);
                    if (server_execution == -1) {
                        perror("open");
                        return 1;
                    }

                    while ((reading = read(server_execution, buffer, 4096)) > 0) {
                        write(status_info, buffer, reading);
                    }
                    close(server_execution);

                    //SCHEDULED
                    //printf("%d\n", fila->tamanho);
                    int server_scheduled = open("../tmp/scheduled", O_RDONLY);
                    if (server_scheduled == -1) {
                        perror("open");
                        return 1;
                    }
                    ssize_t reading_scheduled;
                    char buffer_scheduled[4096];

                    while((reading_scheduled = read(server_scheduled, buffer_scheduled, 4096)) > 0) {
                        write(status_info, buffer_scheduled, reading_scheduled);
                    }
                    close(server_scheduled);

                    ssize_t reading_complete;
                    //COMPLETADO

                    int server_complete = open(server_info, O_RDONLY);
                    if (server_complete == -1) {
                        perror("open");
                        return 1;
                    }
                    
                    while((reading_complete = read(server_complete, buffer_complete, 4096)) > 0) {
                        write(status_info, buffer_complete, reading_complete);
                    }

                    close(server_complete);
                    close(fd_client);
                    close(status_info);

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

                    enQueue(fila, toRead);                        
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
                        //EXECUTAR O CÓDIGO
                        printf("teste\n");
                        int server_execution = open(in_execution, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (server_execution == -1) {
                            perror("open");
                            return 1;
                        }

                        int len = snprintf(NULL, 0, "Executing\n");
                        char* to_execute_output = malloc(len + 1);
                        if (to_execute_output != NULL) {
                            sprintf(to_execute_output, "Executing\n");
                            write(server_execution, to_execute_output, len);
                            free(to_execute_output);
                        }
                        //ssize_t bytes_read = read(pipefd[0], &toExecute, sizeof(Msg));

                        //filho
                        //PARA O STATUS - ESCREVE QUE ESTÁ A EXECUTAR

                        int len2 = snprintf(NULL, 0, "%d %s\n", toExecute.n_task, toExecute.argumentos);
                        char* to_execute_output_s = malloc(len2 + 1);
                        if (to_execute_output_s != NULL) {
                            sprintf(to_execute_output_s, "%d %s\n", toExecute.n_task, toExecute.argumentos);
                            write(server_execution, to_execute_output_s, len2);
                            free(to_execute_output_s);
                        }

                        if(toExecute.tipo == 0){
                            handleQueue(toExecute, argv[1]);
                        }
                        else{
                            handleMultiple(toExecute,argv[1]);
                        }
                        close(server_execution);

                        close(available2);
                        
                        int available3 = open("../tmp/available", O_WRONLY | O_TRUNC);


                        close(available3);

                        
                        
                        _exit(0);
                        }
                        else {
                            deQueue(fila);
                            //close(available2);
                        }
                }
            }
            close(available2);          
                
                
                
                /* else {
                    int scheduled_info = open("../tmp/scheduled", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (scheduled_info == -1) {
                        perror("open");
                        return 1;
                    }
                    

                        char buffer_sc[12];
                        sprintf(buffer_sc, "\nScheduled\n");
                        write(scheduled_info, buffer_sc, strlen(buffer_sc));
                        //if (fila->head != NULL) printf("%s\n",fila->head->data.argumentos);
                        Node* save = fila->head;
                        while (save != NULL) {
                            //printf("%d\n", save->data.n_task);
                            int len = snprintf(NULL, 0, "%d %s\n", save->data.n_task, save->data.argumentos);
                            char* toWrite = malloc(len + 1);
                            if (toWrite != NULL) {
                                sprintf(toWrite, "%d %s\n", save->data.n_task, save->data.argumentos);
                                write(scheduled_info, toWrite, len);
                                free(toWrite);
                            }
                            save = save->next;
                        }

                    write(pipefd[1], &fila->head->data, sizeof(Msg));
                    deQueue(fila);
                    //close(scheduled_info);
                //usleep(1000);
                } */
        }
    }        
        //close(fifos);
        close(pipefd[1]);
        //wait(NULL);
    

        close(fd_server);
        close(server_output_info);
        free(server_info);


    return 0;

}