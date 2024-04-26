#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include "queue.h"
#include "utilidades.h"

void newQueue (Queue* aux) {
    aux->head = NULL;
    aux->tail = NULL;
}

void enQueue (Queue* fila, Msg toInsert) {
    Node* newNode = malloc(sizeof(Node));
    if (!newNode) {
        perror("malloc");
        return;
    }
    newNode->data = toInsert;
    newNode->next = NULL;

    if (fila->tail != NULL) {
        fila->tail->next = newNode;
    }
    fila->tail = newNode;

    //fila vazia
    if (fila->head == NULL) {
        fila->head = newNode;
    }

    fila->tamanho++;
    
}

void deQueue (Queue* fila) {
    //fila vazia
    if (fila->head != NULL) {
        fila->head = fila->head->next;

        //se a fila enventualmente ficar vazia, a tail tmb fica vazia
        if (fila->head == NULL) {
            fila->tail = NULL;
        }
        fila->tamanho--;
    }  
}

void printQueue (Queue* fila) {
    Node* atual = fila->head;
    while(atual != NULL) {
        printf("%s\n", atual->data.argumentos);
        atual = atual->next;
    }
}

void handleQueue (Msg toExecute, char* server_output_info) {
    int status;

    Msg buf = toExecute;
    
    struct timeval* inicio_time = malloc(sizeof(struct timeval*));
    struct timezone* zona = NULL;
    int verifytime = gettimeofday(inicio_time, zona);
    if (verifytime == -1) {
        perror("gettimeofday");
        return;
    }

    //FICHEIRO DE ERROS
    char* erros = malloc(strlen(server_output_info) + 15);
    sprintf(erros, "%sTASK%d_ERRORS", server_output_info, buf.n_task);
    
    int erros_out = open(erros, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(erros_out == -1){
        perror("open");
        return;
    }

    int save_errors = dup(2);
    dup2(erros_out, 2);

    char** commands = malloc(300);
    separa_argumentos(commands, buf.argumentos);
    char* aux = malloc(strlen(server_output_info) + 15);

    int filho_pid = fork();
    
    if (filho_pid == -1) {
        perror("fork_filho");
        return;

    } else if (filho_pid == 0) {
        //CRIAR O FICHEIRO DE OUTPUT
        int stdout = dup(1);
        sprintf(aux, "%sTASK%d_OUTPUT", server_output_info, buf.n_task);

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
            return;
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
        return;
    }

    int server_output_inf = open("../tmp/SERVER_INFO", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (server_output_inf == -1) {
        perror("open");
        return;
    }


    suseconds_t tempomicrosegundos = fim_time->tv_usec - inicio_time->tv_usec;
    int len = snprintf(NULL, 0, "\nTask %d\nTime elapsed: %ld\n", buf.n_task, tempomicrosegundos);
    char *toWrite_output = malloc(len + 1);
    if (toWrite_output != NULL) {
        sprintf(toWrite_output, "\nTask %d\nTime elapsed: %ld\n", buf.n_task, tempomicrosegundos);
        write(server_output_inf, toWrite_output, len);
    }

    close(erros_out);
    dup2(save_errors, 2);
    //n_tasks++;

    free(erros);
    free(inicio_time);
    free(commands);
    free(aux);
    free(fim_time);
    free(toWrite_output); 
}