// mArg.c
// 1452334 Bonan Ruan
// Mon Apr 17 19:08:49 2017

#ifndef M_PUBLIC_H
#define M_PUBLIC_H
#include "mPublic.h"
#endif

void Usage(char *argv0)
{
	printf("%s devid number\n", argv0);
}

Status ArgParse(int argc, char** argv, struct ArgDev* argDev)
{
	if(argc <= 2){
		Usage(argv[0]);
		return ERROR;
	}
	if(argc > 3){
		// for extension in the future
		return ERROR;
	}
	argDev->devidNum = atoi(argv[2]);
	if(ArgAlloc(argDev) == ERROR){
		perror("ArgAlloc");
		ArgFree(argDev);
		return ERROR;
	}

	int i;
	for(i = 0; i < argDev->devidNum; i++){
		argDev->devidArray[i] = (u_int)(atoi(argv[1]) + i);
	}

	return OK;
}

Status ArgAlloc(struct ArgDev* argDev)
{
	int i;
	argDev->devidArray = (u_int *)malloc(argDev->devidNum * sizeof(u_int));
	if(argDev->devidArray == NULL){
		perror("malloc");
		return ERROR;
	}

	return OK;
}

Status ArgFree(struct ArgDev* argDev)
{
	if(argDev->devidArray){
		free(argDev->devidArray);
		argDev->devidArray = NULL;
	}
	argDev->devidNum = 0;

	return OK;
}


void ArgDevInit(struct ArgDev* argDev)
{
	argDev->devidNum = 0;
	argDev->devidArray = NULL;
}
