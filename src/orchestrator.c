#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "writeInput.h"

// Módulo para criação do servidor

int main (int argc, char* argv[]) {

    if (mkfifo("../tmp/fifo", 0666) == -1) {
        perror("mkfifo");
        return 1;   
    }


    char buf[BSIZE];
    ssize_t bytes_read;
    int fd;
    fd = open("../tmp/fifo", O_RDONLY);

    while ((bytes_read = read(fd, buf, BSIZE)) > 0) {
        write(1, buf, bytes_read);
    }

    close(fd);






    return 0;
}