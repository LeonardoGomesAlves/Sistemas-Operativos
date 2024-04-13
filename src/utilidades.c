#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include "pipe.h"
#include "writeInput.h"


//Deixa no estado necessário para o execvp
void separa_argumentos (char* commands[BSIZE],char buf[BSIZE]){ 
    
    char * str = NULL;
    int i = 0;
	while ((str = strsep(&buf," ") )!= NULL){
		commands[i++] = str;
	}
	commands[i] = NULL;
}