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

    Queue* fila = newQueue();

    while(1) {
        while ((bytes_read = read(fd_server, &buf, sizeof(Msg))) > 0) {
            Msg data = buf;
            Msg data2 = buf;
            enQueue(fila, data);
            //enQueue(fila, data2);
            deQueue(&fila);
            //enQueue(fila, data2);
            printQueue(fila);




        }
        /* {
            struct timeval* inicio_time = malloc(sizeof(struct timeval*));
            struct timezone* zona = NULL;
            int verifytime = gettimeofday(inicio_time, zona);
            if (verifytime == -1) {
                perror("gettimeofday");
                return 1;
            }

            //FICHEIRO DE ERROS
            char* erros = malloc(strlen(argv[1]) + 15);
            sprintf(erros, "%sTASK%d_ERRORS", argv[1], n_tasks);
            
            int erros_out = open(erros, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(erros_out == -1){
                perror("open");
                return 1;
            }

            int save_errors = dup(2);
            dup2(erros_out, 2);



            //ENVIA MENSAGEM PARA O CLIENT
            sprintf(buf.response, "TASK %d Received\n", n_tasks);
            int fd_client = open(buf.pid_path, O_WRONLY);
            if (fd_client == -1) {
                perror("open");
                return 1;
            }

            write(fd_client, &buf, sizeof(buf));
            close(fd_client);

            char** commands = malloc(300);
            separa_argumentos(commands, buf.argumentos);
            char* aux = malloc(strlen(argv[1]) + 15);

            int filho_pid = fork();
            
            if (filho_pid == -1) {
                perror("fork_filho");
                return 1;

            } else if (filho_pid == 0) {
                //CRIAR O FICHEIRO DE OUTPUT
                int stdout = dup(1);
                sprintf(aux, "%sTASK%d_OUTPUT", argv[1], n_tasks);
        
                int file_out = open(aux, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(file_out == -1){
                    perror("open");
                    _exit(-1);
                }
                
                dup2(file_out, 1);

                //EXECUÇÃO DO COMANDO DO USER
                if(execvp(commands[0],commands) == -1) 
                {
                    perror("execvp");
                    return 1;
                }

                close(file_out);
                dup2(stdout, 1);
                _exit(0);
            }

            else {
                waitpid(filho_pid, &status, 0);
            }

            //tempo final e escrita no ficheiro
            struct timeval* fim_time = malloc(sizeof(struct timeval*));
            //struct timezone* zona2 = NULL;
            
            int verifytime2 = gettimeofday(fim_time, zona);
            if (verifytime2 == -1) {
                perror("gettimeofday");
                return 1;
            }

            suseconds_t tempomicrosegundos = fim_time->tv_usec - inicio_time->tv_usec;
            int len = snprintf(NULL, 0, "Task %d\nTime elapsed: %ld\n", n_tasks, tempomicrosegundos);
            char *toWrite_output = malloc(len + 1);
            if (toWrite_output != NULL) {
                sprintf(toWrite_output, "Task %d\nTime elapsed: %ld\n\n", n_tasks, tempomicrosegundos);
                write(server_output_info, toWrite_output, len);
            }

            close(erros_out);
            dup2(save_errors, 2);
            n_tasks++;

            free(erros);
            free(inicio_time);
            free(commands);
            free(aux);
            free(fim_time);
            free(toWrite_output);
        } */
    }        

        close(fd_server);
        close(server_output_info);
        free(server_info);


    return 0;

}