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

    int fifos;
    if((fifos = open("../tmp/fifo_queue", O_RDWR)) == -1){
        perror("open");
        return 1;
    }


    /* int pipe_available[2];
    if (pipe(pipe_available) == -1){
        perror("pipe");
        return 1;
    }
    fcntl(pipe_available[0], F_SETFL, O_NONBLOCK);
    fcntl(pipe_available[1], F_SETFL, O_NONBLOCK); */


    /* int pid_geral = fork();

    if (pid_geral == -1) {
        perror("fork");
        return 1;
    } */

    //filho
    /* if (pid_geral == 0) {
        //close(pipe_available[0]);
        //close(pipefd[1]);
        //write(pipe_available[1], AVAILABLE, strlen(AVAILABLE) + 1);
        while (1) {          

            Msg toExecute;
            //ssize_t bytes_read = read(pipefd[0], &toExecute, sizeof(Msg));
            ssize_t bytes_read = read(fifos, &toExecute, sizeof(Msg));

            //filho
            if (bytes_read > 0) {
                //write(pipe_available[1], UNAVAILABLE, strlen(UNAVAILABLE) + 1);
                if(toExecute.tipo == 0){
                    handleQueue(toExecute, argv[1]);
                }
                else{
                    handleMultiple(toExecute,argv[1]);
                }
                //write(pipe_available[1], AVAILABLE, strlen(AVAILABLE) + 1);
            }

        }
        close(fifos);
        //close(pipefd[0]);
        //close(pipe_available[1]);
    } */
     
        //close(pipe_available[1]);
        //close(pipefd[0]);
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

                usleep(1000); //engana o read a continuar a ler para nao fazer sequencialmente

                //write(fifos, &toRead, sizeof(toRead));
                //write(pipefd[1], &fila->head->data, sizeof(Msg));
                //deQueue(fila);
                }

                if (fila->head != NULL) {
                    if(fila->head->data.tipo == 0){
                        handleQueue(fila->head->data, argv[1]);
                    }
                    else{
                        handleMultiple(fila->head->data,argv[1]);
                    } 
                    deQueue(fila);
                }
                
                /* char estado[2];
                //read(pipe_available[0], estado, strlen(AVAILABLE) + 1);
                if (strcmp(estado, AVAILABLE) == 0) {
                    write(pipefd[1], &fila->head->data, sizeof(Msg));
                    //printf("%d\n", fila->tamanho);
                    deQueue(fila);
                } */
  
        }        
        close(fifos);
        //close(pipefd[1]);
        //wait(NULL);
    

        close(fd_server);
        close(server_output_info);
        free(server_info);


    return 0;

}