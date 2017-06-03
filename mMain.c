// mMain.c
// 1452334 Bonan Ruan
// Mon Apr 17 18:53:53 2017


#ifndef M_PUBLIC_H
#define M_PUBLIC_H
#include "mPublic.h"
#endif

int main(int argc, char** argv)
{
	struct ArgDev argDev;
	ArgDevInit(&argDev);

	// parse arguments
	if(ArgParse(argc, argv, &argDev) == ERROR){
		goto Label_ERROR;
	}

	// parse ts.conf
	struct ConfigFile configFile;
	if(ConfigParse(&configFile) == ERROR){
		goto Label_ERROR;
	}
	
	// fork sub-processes as terminals
	int i;
	pid_t pid;
	for(i = 0; i < argDev.devidNum; i++){
		if((pid = fork()) == -1){
			perror("fork");
			goto Label_ERROR;
		}
		if(pid == 0)
			break;
	}
	if(pid > 0){ // parent iterates
		waitpid(-1, NULL, 0); // wait children exit
	}
	if(pid == 0){ // gogogo~
		while(1){
			int flagOrTime = 0; // used to hold sc_iden.reTransTime for re-transport
			if((flagOrTime = ClientSimulate(argDev.devidArray[i], &configFile)) == ERROR){
				perror("ClientSimulate");
				goto Label_ERROR;
			}
			if(configFile.quitAfterSucc != 0)
				break;
			sleep(flagOrTime); // after flagOrTime then connect server again
		}
	}

Label_OK:
	ArgFree(&argDev);
	return OK;

Label_ERROR:
	ArgFree(&argDev);
	return ERROR;
}
