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
#include <stdlib.h>


//Deixa no estado necessário para o execvp
int separa_argumentos (char** commands, char* buf){ 
    
    char * str = NULL;
    int i = 0;
	while ((str = strsep(&buf," ") )!= NULL){
		//commands[i] = malloc(sizeof(char) * strlen(buf[i]) + 1);
		commands[i++] = str;
	}
	commands[i] = NULL;
	return i;
}