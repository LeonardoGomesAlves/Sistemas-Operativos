#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "writeInput.h"
#include "pipe.h"

// Módulo para criação do servidor

int main (int argc, char* argv[]) {

    printf("Waiting for inputs....");

    createFifo("fifo");

    char buf[BSIZE];
    ssize_t bytes_read;
    int fd;
    fd = open("../tmp/fifo", O_RDONLY);

    while (1) {
        while ((bytes_read = read(fd, buf, BSIZE)) > 0) {
        write(1, buf, bytes_read);
        }
    }

    

    //close(fd);






    return 0;
}