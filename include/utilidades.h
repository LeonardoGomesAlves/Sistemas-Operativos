#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include "pipe.h"
#include "clientSingle.h"


int separa_argumentos (char** commands,char* buf);
#endif
