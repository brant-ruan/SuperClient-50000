// mPublic.h
// 1452334 Bonan Ruan
// Mon Apr 17 18:54:44 2017

/* -------------------------------------- INCLUDE */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/wait.h>

/* -------------------------------------- DEFINE */
#define	ERROR		-1
#define	OK			0

#define YES			1
#define NO			0

#define ENABLE		1
#define DISABLE		0

#define	DEVID_LEN	9
#define IP_LEN		16

#define DEBUG_NUM	6
#define DEBUG_ENV	0
#define DEBUG_ERR	1
#define DEBUG_SPACK	2
#define DEBUG_RPACK	3
#define DEBUG_SDATA	4
#define DEBUG_RDATA	5

#define SERVER_CLOSED	-2

/* macro for log-module */
#define RDWR_RD		0 // read data
#define RDWR_WR		1 // write data
#define CPU_RAM_FILE	"./CPU_RAM.dat"

/* -------------------------------------- TYPEDEF */
typedef int Status;
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;

/* -------------------------------------- CONST */


/* -------------------------------------- STRUCT */
struct ConfigFile{
	char serverIP[IP_LEN + 1];
	u_short serverPort;
	int termNumMin;
	int termNumMax;
	int vScrNumMin;
	int vScrNumMax;
	char delLog;
	char quitAfterSucc;
	char debugCfg[DEBUG_NUM];
	char debugShow;
};

struct ArgDev{
	int devidNum;
	// overflow may occur if the devid is too large...
	u_int *devidArray;
};

struct SendBuf{
	int len;
	char *buf;
};
