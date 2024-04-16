#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include "clientSingle.h"
#include "pipe.h"
#include "utilidades.h"
#include "orchestrator.h"
#include <stdlib.h>


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

    mkdir(argv[1], 0700);

    Msg buf;
    ssize_t bytes_read;
    int fd_server;
    int status;
    fd_server = open(SERVER, O_RDONLY);

    if (fd_server == -1) {  
        perror("open");
        return 1;
    }

    int n_tasks = 1;

    while(1) {
        while ((bytes_read = read(fd_server, &buf, sizeof(Msg))) > 0) 
        {
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

            int filho_pid = fork();
            
            if (filho_pid == -1) {
                perror("fork_filho");
                return 1;

            } else if (filho_pid == 0) {
                //CRIAR O FICHEIRO DE OUTPUT
                int stdout = dup(1);
                char* aux = malloc(strlen(argv[1]) + 15);
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
            
            close(erros_out);
            dup2(save_errors, 2);
            n_tasks++;

        }
    }        

        close(fd_server);


    return 0;

}