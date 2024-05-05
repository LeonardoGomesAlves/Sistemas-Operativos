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

//cria uma nova queue
void newQueue (Queue* aux) {
    aux->head = NULL;
    aux->tail = NULL;
}

//dá enqueue utilizando FCFS
void enQueueFCFS (Queue* fila, Msg toInsert) {
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

//dá enqueue utilizando SJF
void enQueueSJF (Queue* fila, Msg toInsert) {
    Node* newNode = malloc(sizeof(Node));
    if (!newNode) {
        perror("malloc");
        return;
    }
    newNode->data = toInsert;
    newNode->next = NULL;

    Node *current = fila->head;
    Node *prev = NULL;

    if (fila->head == NULL || newNode->data.tempo < fila->head->data.tempo) {
        newNode->next = fila->head;
        fila->head = newNode;

        if (fila->tail == NULL) {
            fila->tail = newNode;
        }
    } else {
        while (current != NULL && current->data.tempo <= newNode->data.tempo) {
            prev = current;
            current = current->next;
        }

        prev->next = newNode;
        newNode->next = current;

        if (current == NULL) {
            fila->tail = newNode;
        }
    }

    fila->tamanho++;
}

//dá dequeue
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

//executa um comando do pipeline
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


void executePipeline (Msg toExecute, char* server_output_info){
    Msg buf = toExecute;

    char copy[300];
    strcpy(copy, toExecute.argumentos);
    
    struct timeval inicio_time, fim_time;
    gettimeofday(&inicio_time, NULL); //primeiro tempo

    //FICHEIRO DE OUTPUT
    char* output_ref = malloc(strlen(server_output_info) + 15);
    sprintf(output_ref, "%sTASK%d_OUTPUT", server_output_info, buf.n_task);
    
    int output_fd = open(output_ref, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(output_fd == -1){
        perror("open");
        return;
    }

    dup2(output_fd, 2);
    dup2(output_fd, 1);

    char** commands = malloc(300);
    int n_commands = separa_argumentos(commands, buf.argumentos,buf.tipo);
    int pipes[n_commands-1][2];
    int pids[n_commands];


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
                dup2(output_fd,STDOUT_FILENO);
			}

			for (int j = 0; j < n_commands - 1; j++) {
				close(pipes[j][0]);
				close(pipes[j][1]);
			}

			//verificação de 
			for (int j = 0; j < n_commands; j++) {
				if (i == j) {
					if (exec_command(commands[i]) == -1) {
                        perror("execvp");
                        return;
                    }
				}
			}

            close(output_fd);

		}
	}

	for (int i = 0; i < n_commands - 1; i++) {
		close(pipes[i][0]);
		close(pipes[i][1]);
	}


	for (int i = 0; i < n_commands; i++) {
		 waitpid(pids[i], NULL, 0);
	}
    

    gettimeofday(&fim_time, NULL);
    

    int server_output_inf = open("../tmp/SERVER_INFO", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (server_output_inf == -1) {
        perror("open");
        return;
    }

    long tempo_execucao = ((fim_time.tv_sec * 1000000 + fim_time.tv_usec) - (inicio_time.tv_sec * 1000000 + inicio_time.tv_usec)) / 1000;

    int len = snprintf(NULL, 0, "%d %s %ld ms\n", buf.n_task, copy, tempo_execucao);
    char *toWrite_output = malloc(len + 1);
    if (toWrite_output != NULL) {
        sprintf(toWrite_output, "%d %s %ld ms\n", buf.n_task, copy, tempo_execucao);
        write(server_output_inf, toWrite_output, len);
    }

    free(output_ref);
    close(server_output_inf);

	return;

}


void executeSingle (Msg toExecute, char* server_output_info) {
    struct timeval inicio_time, fim_time;
    gettimeofday(&inicio_time, NULL);
    
    int status;


    Msg buf = toExecute;

    char copy[300];
    strcpy(copy, buf.argumentos);


    //FICHEIRO DE ERROS
    char* output = malloc(strlen(server_output_info) + 15);
    sprintf(output, "%sTASK%d_OUTPUT", server_output_info, buf.n_task);
    
    int output_descritor = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(output_descritor == -1){
        perror("open");
        return;
    }

    int save_errors = dup(2);
    dup2(output_descritor, 2);

    char** commands = malloc(300);
    separa_argumentos(commands, buf.argumentos,buf.tipo);
    char* aux = malloc(strlen(server_output_info) + 15);

    int filho_pid = fork();
    
    if (filho_pid == -1) {
        perror("fork_filho");
        return;

    } else if (filho_pid == 0) {

        dup2(output_descritor, 1);

        //EXECUÇÃO DO COMANDO DO USER
        if(execvp(commands[0],commands) == -1) 
        {
            perror("execvp");
            return;
        }

        close(output_descritor);
        _exit(0);
    }

    else {
        waitpid(filho_pid, &status, 0);
    }

    
    gettimeofday(&fim_time, NULL);
    

    int server_output_inf = open("../tmp/SERVER_INFO", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (server_output_inf == -1) {
        perror("open");
        return;
    }

    long tempo_execucao = ((fim_time.tv_sec * 1000000 + fim_time.tv_usec) - (inicio_time.tv_sec * 1000000 + inicio_time.tv_usec)) / 1000;

    
    int len = snprintf(NULL, 0, "%d %s %ld ms\n", buf.n_task, copy, tempo_execucao);
    char *toWrite_output = malloc(len + 1);
    if (toWrite_output != NULL) {
        sprintf(toWrite_output, "%d %s %ld ms\n", buf.n_task, copy, tempo_execucao);
        write(server_output_inf, toWrite_output, len);
    }

    close(server_output_inf);
    close(output_descritor);
    dup2(save_errors, 2);
    //n_tasks++;

    free(commands);
    free(aux);
    free(toWrite_output); 
}

int handleClientStatus(Msg toRead, int paralel_tasks, char* server_info, Queue* fila) {
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

    //EM EXECUÇÃO
    char* executing_info = malloc(strlen("Executing\n") + 1);
    sprintf(executing_info, "Executing\n");
    write(status_info, executing_info, strlen(executing_info) + 1);
    free(executing_info);

    for (int i = 0; i < paralel_tasks; i++) {
        char* iterator = malloc(strlen("../tmp/IN_EXECUTION") + 5);
        sprintf(iterator, "../tmp/IN_EXECUTION%d", i);

        int descp_current_paralel = open(iterator, O_RDONLY);
        if (descp_current_paralel == -1) {
            perror("open");
            return 1;
        }
        char buf[350];
        ssize_t bytes_read;
        if ((bytes_read = read(descp_current_paralel, &buf, sizeof(buf))) > 0) {
            write(status_info, buf, bytes_read);
        }
        free(iterator);
        close(descp_current_paralel);
    }

    //SCHEDULED
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

int exec_task (Msg toExecute, int in_execution_id, char* output_folder, char* process) {
    //EXECUTAR O CÓDIGO
    char* in_execution = malloc(strlen("../tmp/IN_EXECUTION") + 5);
    sprintf(in_execution, "../tmp/IN_EXECUTION%d", in_execution_id);

    //utilizado para calcular os tempos de execução e de espera dos processos
    /* int tt = open("../tmp/TEMPO_TOTAL", O_WRONLY | O_APPEND);
    struct timeval fim;
    gettimeofday(&fim, NULL);
    double total_time = (double)((fim.tv_sec - toExecute.tempo_inicial.tv_sec) + (fim.tv_usec - toExecute.tempo_inicial.tv_usec) / 1000000.0);
    char* teste = malloc(strlen("%.6f\n") + 4);
    sprintf(teste, "%.6f\n", total_time);
    write(tt, teste, strlen(teste));
    close(tt); */

    int server_execution = open(in_execution, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (server_execution == -1) {
        perror("open");
        return 1;
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
        executeSingle(toExecute, output_folder);
    }
    else{
        executePipeline(toExecute,output_folder);
    }
      
    close(server_execution);

    int truncate = open(in_execution, O_TRUNC);
    close(truncate);
    
    int available3 = open(process, O_WRONLY | O_TRUNC);


    close(available3);

    return 0;
}