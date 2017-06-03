// mMain.c
// 1452334 Bonan Ruan
// Mon Apr 17 18:53:53 2017


#ifndef M_PUBLIC_H
#define M_PUBLIC_H
#include "mPublic.h"
#endif

#define ALLOW_EXIST_CHILD_NUM	200

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
	int existChild = 0;
	pid_t pid;
	for(i = 0; i < argDev.devidNum; i++){
		if((pid = fork()) == -1){
			perror("fork");
			i--;
			sleep(2);
			continue;
//			goto Label_ERROR;
		}
		if(pid == 0)
			break;
		existChild++;
		if(existChild == ALLOW_EXIST_CHILD_NUM){
			waitpid(-1, NULL, 0); // must wait for one child exit

		}
		while(waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0){
			existChild--; // wait as many as possible
		}
	}
	if(pid > 0){ // parent iterates
		while(waitpid(-1, NULL, 0) > 0) // wait children exit
			;
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
