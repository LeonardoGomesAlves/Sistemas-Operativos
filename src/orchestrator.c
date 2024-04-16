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


// Módulo para criação do servidor

int main (int argc, char* argv[]) {

    printf("Waiting for inputs....");

    //criar o pipe do server
    if (mkfifo(SERVER, 0666) == -1) {
        perror("mkfifo");
        return 1;   
    }

    Msg buf;
    ssize_t bytes_read;
    int fd_server;
    int status;
    fd_server = open(SERVER, O_RDONLY);

    if (fd_server == -1) {  
        perror("open");
        return 1;
    }

    while(1) {
        while ((bytes_read = read(fd_server, &buf, sizeof(Msg))) > 0) 
        {
            char** commands = malloc(300);
            separa_argumentos(commands, buf.argumentos);

            int filho_pid = fork();
            
            if (filho_pid == -1) {
                perror("fork_filho");
                return 1;
            } else if (filho_pid == 0) {
                int stdout = dup(1);
                char* aux = "output";
                char* output = strdup(buf.pid_path);
                strcat(output,aux);
                int file_out = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(file_out == -1){
                    perror("open");
                    _exit(-1);
                }
                dup2(file_out,1);
                if(execvp(commands[0],commands) == -1) 
                {
                    perror("execvp");
                    return 1;
                }
                close(file_out);
                _exit(0);
            }
            else {
                wait(&status);
            }

           printf("teste\n");
        }
    }        

        close(fd_server);


    return 0;

}