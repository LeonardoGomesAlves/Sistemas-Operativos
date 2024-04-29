#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include "queue.h"
#include "utilidades.h"
#include <sys/types.h>
#include <sys/wait.h>

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


int exec_command(char* arg){

	//Estamos a assumir numero maximo de argumentos
	char *exec_args[10];

	char *string;	
	int exec_ret = 0;
	int i=0;

	char* command = strdup(arg);

	string=strtok(command," ");
	
	while(string!=NULL){
		exec_args[i]=string;
		string=strtok(NULL," ");
		i++;
	}

	exec_args[i]=NULL;
	
	exec_ret=execvp(exec_args[0],exec_args);
	
	return exec_ret;
}


void handleMultiple (Msg toExecute, char* server_output_info){
    Msg buf = toExecute;

    char copy[300];
    strcpy(copy, toExecute.argumentos);
    
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

    dup2(erros_out, 2);
    close(erros_out);

    char** commands = malloc(300);
    int n_commands = separa_argumentos(commands, buf.argumentos,buf.tipo);
    char* aux = malloc(strlen(server_output_info) + 15);
    int pipes[n_commands-1][2];
    int pids[n_commands];

    int stdout = dup(1);
    sprintf(aux, "%sTASK%d_OUTPUT", server_output_info, buf.n_task);
    int file_out = open(aux, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(file_out == -1){
        perror("open");
        _exit(-1);
    }


    for (int i = 0; i < n_commands - 1; i++) {
		if(pipe(pipes[i]) == -1) {
			perror("pipe");
			return;
		}
	}

	for (int i = 0; i < n_commands; i++) {
		if ((pids[i] = fork()) == -1) {
			perror("fork");
			return;
		}
	
		//filho
		if (pids[i] == 0) {
			if (i == 0) {
				dup2(pipes[0][1], STDOUT_FILENO);
			}

			for (int j = 1; j < n_commands - 1; j++) {
				if (i == j) {
					dup2(pipes[j-1][0], STDIN_FILENO);
					dup2(pipes[j][1], STDOUT_FILENO);
				}
			}

			if (i == n_commands-1) {
				dup2(pipes[i-1][0], STDIN_FILENO);
                dup2(file_out,STDOUT_FILENO);
			}

			for (int j = 0; j < n_commands - 1; j++) {
				close(pipes[j][0]);
				close(pipes[j][1]);
			}

			//verificação de 
			for (int j = 0; j < n_commands; j++) {
				if (i == j) {
					exec_command(commands[i]);
				}
			}

            close(file_out);
            close(erros_out);

		}
	}

	for (int i = 0; i < n_commands - 1; i++) {
		close(pipes[i][0]);
		close(pipes[i][1]);
	}


	for (int i = 0; i < n_commands; i++) {
		 waitpid(pids[i], NULL, 0);
		//printf("%d\n", k);
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


    //O TEMPO DISTO ESTÁ ABSURDAMENTE BAIXO, ficou em microsegundos mas parece milisegundos comparando com o single execution
    suseconds_t tempomicrosegundos = fim_time->tv_usec - inicio_time->tv_usec;
    //printf("%ld %ld\n", fim_time->tv_usec, inicio_time->tv_usec);
    /* if (tempomicrosegundos < 0) {
        tempomicrosegundos += 1000000;
    } */
    int len = snprintf(NULL, 0, "%d %s %ld ms\n", buf.n_task, copy, tempomicrosegundos);
    char *toWrite_output = malloc(len + 1);
    if (toWrite_output != NULL) {
        sprintf(toWrite_output, "%d %s %ld ms\n", buf.n_task, copy, tempomicrosegundos);
        write(server_output_inf, toWrite_output, len);
    }

    close(file_out);
    close(server_output_inf);

	return;


}


void handleQueue (Msg toExecute, char* server_output_info) {
    int status;

    Msg buf = toExecute;

    char copy[300];
    strcpy(copy, buf.argumentos);

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
    separa_argumentos(commands, buf.argumentos,buf.tipo);
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
        close(erros_out);
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
    if (tempomicrosegundos < 0) {
        tempomicrosegundos += 1000000;
    }
    tempomicrosegundos = tempomicrosegundos / 1000;
    int len = snprintf(NULL, 0, "%d %s %ld ms\n", buf.n_task, copy, tempomicrosegundos);
    char *toWrite_output = malloc(len + 1);
    if (toWrite_output != NULL) {
        sprintf(toWrite_output, "%d %s %ld ms\n", buf.n_task, copy, tempomicrosegundos);
        write(server_output_inf, toWrite_output, len);
    }

    close(server_output_inf);
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

int handleClientStatus(Msg toRead, char* in_execution, char* server_info, Queue* fila) {
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


    char buffer_sc[12];
    sprintf(buffer_sc, "\nScheduled\n");
    write(status_info, buffer_sc, strlen(buffer_sc));
    Node* save = fila->head;

    while (save != NULL) {
        int len = snprintf(NULL, 0, "%d %s\n", save->data.n_task, save->data.argumentos);
        char* toWrite = malloc(len + 1);
        if (toWrite != NULL) {
            sprintf(toWrite, "%d %s\n", save->data.n_task, save->data.argumentos);
            write(status_info, toWrite, len);
            free(toWrite);
        }
        save = save->next;
    }

    //COMPLETADO

    ssize_t reading_complete;
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

    return 0;
}

int exec_task (Msg toExecute, char* in_execution, char* output_folder) {
    //EXECUTAR O CÓDIGO
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

    //PARA O STATUS - ESCREVE QUE ESTÁ A EXECUTAR

    int len2 = snprintf(NULL, 0, "%d %s\n", toExecute.n_task, toExecute.argumentos);
    char* to_execute_output_s = malloc(len2 + 1);
    if (to_execute_output_s != NULL) {
        sprintf(to_execute_output_s, "%d %s\n", toExecute.n_task, toExecute.argumentos);
        write(server_execution, to_execute_output_s, len2);
        free(to_execute_output_s);
    }

    if(toExecute.tipo == 0){
        handleQueue(toExecute, output_folder);
    }
    else{
        handleMultiple(toExecute,output_folder);
    }
    close(server_execution);

    
    int available3 = open("../tmp/available", O_WRONLY | O_TRUNC);


    close(available3);

    return 0;
}