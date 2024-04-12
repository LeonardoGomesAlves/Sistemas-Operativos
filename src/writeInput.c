#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include "writeInput.h"



int writeInPipe_Single (char** input) {

    char buf [BSIZE];
    int bytes_read, fd;

    fd = open("../tmp/fifo", O_WRONLY);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    for (int i = 0; input[i] != NULL; i++) {
        bytes_read = write(fd, input[i], strlen(input[i]));
        if (bytes_read == -1) {
            perror("write");
            close(fd);
            return -1;
        }
    }

    close(fd);
    return 0;

}