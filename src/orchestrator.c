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
#include <ctype.h>

// Módulo para criação do servidor

void print_erro() {
    char* input_inv = malloc(100);
    sprintf(input_inv, "Input inválido.\n./orchestrator ../<nome_da_pasta_deseja>/ <numero de paralel tasks> <FCFS ou SJF>\n");
    write(1,input_inv, strlen(input_inv));
    free(input_inv);
}

int main (int argc, char* argv[]) {

    //EXEMPLO DE UTILIZAÇÃO: ./orchestrator ../<nome_da_pasta_deseja>/ <numero de paralel tasks> <FCFS ou SJF>
    if (argc != 4) {
        print_erro();
        return 1;
    }

    if (strcmp("FCFS", argv[3]) != 0 && strcmp("SJF", argv[3]) != 0) {
        print_erro();
        return 1;
    }

    for (int i = 0; i < strlen(argv[2]); i++) {
        if (!isdigit(argv[2][i])) {
            print_erro();
            return 1;
        }
    }

    //falta a verificação do argv[2]
    int paralel_tasks = atoi(argv[2]);
    if (paralel_tasks <= 0) {
        print_erro();
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

    for (int i = 0; i < paralel_tasks; i++) {
        char* in_execution = malloc(strlen("../tmp/IN_EXECUTION") + 4);
        sprintf(in_execution, "../tmp/IN_EXECUTION%d", i);

        int server_in_execution = open(in_execution, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (server_in_execution == -1) {
            perror("open");
            return 1;
        }

        close(server_in_execution);
    }

    int n_tasks = 1;

    Queue* fila = malloc(sizeof(Queue));
    newQueue(fila);
    
    for (int i = 0; i < paralel_tasks; i++) {
        char* to_open = malloc(strlen("../tmp/available") + 4);
        sprintf(to_open, "../tmp/available%d", i);
        int available = open(to_open, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        close(available);
    }


    while(1) {
        while ((bytes_read = read(fd_server, &toRead, sizeof(Msg))) > 0) {
            //SE O REQUEST FOR UM STATUS
            if (toRead.tipo == 2) {

                handleClientStatus(toRead, paralel_tasks, server_info, fila);

            } else if (toRead.tipo == 3){ //mata o server
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

        int to_read_desc;
        int reader;
        char* to_read;

        int i;
        for (i = 0; i < paralel_tasks; i++) {
            to_read = malloc(strlen("../tmp/available") + 4);
            sprintf(to_read, "../tmp/available%d", i);
            //printf("%s\n", to_read);
            to_read_desc = open(to_read, O_RDWR);
            if (to_read_desc == -1){
                perror("open");
                return 1;
            }
            char buf_temp[10];
            reader = read(to_read_desc, &buf_temp, sizeof(buf_temp));
            if (reader == 0) break;
            else close(to_read_desc);
        }

        
        if (reader == 0 && fila->head != NULL) {
            char* towrite = malloc(strlen("n")+1);
            strcpy(towrite, "n");
            write(to_read_desc, towrite, strlen(towrite) + 1);
            free(towrite);
            close(to_read_desc);
            
            Msg toExecute = fila->head->data;

            int filho = fork();
            if (filho == -1) {
                perror("fork");
                return 1;
            }

            if (filho == 0) {
                exec_task(toExecute, i, argv[1], to_read);
                free(to_read);                   
                
                _exit(0);
            } else {
                deQueue(fila);
            }
        }
        else {
            close(to_read_desc);
            free(to_read);
        }
        //close(available2);          
    }       
        close(fd_server);
        close(server_output_info);
        free(server_info);


    return 0;

}