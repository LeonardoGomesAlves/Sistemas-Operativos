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
    
    //CONTROLO DE ERROS DE INPUT
    
    if (argc != 4) {
        print_erro();
        return 1;
    }

    //tipo de escalonamento
    if (strcmp("FCFS", argv[3]) != 0 && strcmp("SJF", argv[3]) != 0) {
        print_erro();
        return 1;
    }

    //valor do paralel tasks
    for (int i = 0; i < strlen(argv[2]); i++) {
        if (!isdigit(argv[2][i])) {
            print_erro();
            return 1;
        }
    }

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
        
    //abrir pipe do server, que recebe mensagens do cliente
    Msg toRead;
    ssize_t bytes_read;
    int fd_server;
    fd_server = open(SERVER, O_RDONLY);

    if (fd_server == -1) {  
        perror("open");
        return 1;
    }

    //utilizado para calcular os tempos de execução e de espera dos processos
    /* int ss = open("../tmp/TEMPO_TOTAL", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    close(ss); */

    //criação do ficheiro para guardar as queries que já executou
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

    //cria um ficheiro que serve para avisar qual é a tarefa a ser executada, conforme as paralel_tasks que o utilizador insere no servidor
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

    //criação da fila
    Queue* fila = malloc(sizeof(Queue));
    newQueue(fila);
    
    //ficheiro que serve para manter o numero maximo de paralel_tasks conforme o que tem que executar
    for (int i = 0; i < paralel_tasks; i++) {
        char* to_open = malloc(strlen("../tmp/available") + 4);
        sprintf(to_open, "../tmp/available%d", i);
        int available = open(to_open, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        free(to_open);
        close(available);
    }

    //utilizado para calcular os tempos de execução e de espera dos processos
    /* struct timeval inicio_time, fim_time;

    int stat = 1;
    int total = 300; */

    while(1) {
        while ((bytes_read = read(fd_server, &toRead, sizeof(Msg))) > 0) {
           
           //utilizado para calcular os tempos de execução e de espera dos processos
            /* if (stat == 1) {
                gettimeofday(&inicio_time, NULL);
                stat = 0;
            }
            
            struct timeval tempo;
            gettimeofday(&tempo, NULL);
            toRead.tempo_inicial = tempo; */
                
            //SE O REQUEST FOR UM CLIENT STATUS
            if (toRead.tipo == 2) {

                handleClientStatus(toRead, paralel_tasks, server_info, fila);

            } else if (toRead.tipo == 3){ //mata o server, ./client kill
                
                free(fila);
                close(fd_server);
                close(server_output_info);
                free(server_info);
                
                return 1;
            } else {

                //ENVIA MENSAGEM PARA O CLIENTE QUE A TASK FOI RECEBIDA
                toRead.n_task = n_tasks++;
                sprintf(toRead.response, "TASK %d Received\n", toRead.n_task);
                
                int fd_client = open(toRead.pid_path, O_WRONLY);
                if (fd_client == -1) {
                    perror("open");
                    return 1;
                }

                write(fd_client, &toRead, sizeof(toRead));
                close(fd_client);

                //dá enqueue dependendo do algoritmo escolhido
                if (algoritmo == 0) {
                    enQueueFCFS(fila, toRead);
                } else {
                    enQueueSJF(fila, toRead);
                }                                       
            }
        }

        //utilizado para calcular os tempos de execução e de espera dos processos
        /* if (total == 0) {
            gettimeofday(&fim_time, NULL);
            double total_time = (double)((fim_time.tv_sec - inicio_time.tv_sec) + (fim_time.tv_usec - inicio_time.tv_usec) / 1000000.0);
            printf("tempo: %.6f\n", total_time);
            total--;
        } */

        int to_read_desc;
        int reader;
        char* to_read;

        //faz a verificação se estao a executar o numero maximo de tarefas
        //se não estiverem, dá break do ciclo e executa, utilizamos escrita ficheiros para manter o numero max de tasks em paralelo
        int i;
        for (i = 0; i < paralel_tasks; i++) {
            to_read = malloc(strlen("../tmp/available") + 4);
            sprintf(to_read, "../tmp/available%d", i);

            to_read_desc = open(to_read, O_RDWR);
            if (to_read_desc == -1){
                perror("open");
                return 1;
            }
            char buf_temp[10];
            reader = read(to_read_desc, &buf_temp, sizeof(buf_temp));
            //se nao ler nada desse ficheiro, sai do loop e vai executar a primeira tarefa da queue caso possa
            if (reader == 0) break;
            else close(to_read_desc);
        }

        //entra na parte de executar
        if (reader == 0 && fila->head != NULL) {
            //escreve no ficheiro que tinha lido anteriormente, que serve para dizer que está a ser executada mais uma tarefa
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
            //o processo filho executa e o processo pai prossegue para continuar a ler mensagens vindas do cliente
            if (filho == 0) {
                exec_task(toExecute, i, argv[1], to_read);
                free(to_read);                   
                _exit(0);
            } else {
                //utilizado para calcular os tempos de execução e de espera dos processos
                /* total--; */
                deQueue(fila);
            }
        }
        else {
            close(to_read_desc);
            free(to_read);
        }
    }       
        free(fila);
        close(fd_server);
        close(server_output_info);
        free(server_info);


    return 0;

}