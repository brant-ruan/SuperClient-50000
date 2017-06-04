// mSocket.c
// 1452334 Bonan Ruan
// Tue Apr 18 09:56:51 2017

#ifndef M_PUBLIC_H
#define M_PUBLIC_H
#include "mPublic.h"
#endif

#ifndef M_PUBLIC_H
#define M_PUBLIC_H
#include "mProtocol.h"
#endif

/* Configure client socket */
Status CSocket(int *fd, char *ip, u_short port)
{
	// client socket
	*fd = socket(AF_INET, SOCK_STREAM, 0);
	if(*fd == -1){
		perror("socket");
		return ERROR;
	}
	// configure server IP:port
	struct sockaddr_in svrAddr;
	bzero(&svrAddr, sizeof(svrAddr));
	svrAddr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, (void *)&(svrAddr.sin_addr));
	svrAddr.sin_port = htons(port);
	// connect
	while(connect(*fd, (struct sockaddr *)&svrAddr, sizeof(svrAddr)) < 0){
		perror("connect");
		sleep(1);
	}

	return OK;
}

/* receive num bytes
 * return actually received length
 * if server closes connection or error occurs,
 * 	return SERVER_CLOSED
 *
 * */
int RecvN(int *fd, char *buf, int num)
{
	int haveRecvd = 0;
	int onceRecvd = 0;
	while(haveRecvd < num){
		if((onceRecvd = read(*fd, buf + haveRecvd, num - haveRecvd)) <= 0){
			return SERVER_CLOSED;
		}
		haveRecvd += onceRecvd;
	}

	return num;
}

/* send num bytes then return num
 * return actually sent length
 * if server closes connection or error occurs,
 * 	return SERVER_CLOSED
 *
 * */
int SendN(int *fd, struct SendBuf *sendBuf)
{
	int haveSend = 0;
	int onceSend = 0;
	while(haveSend < sendBuf->len){
		if((onceSend = write(*fd, sendBuf->buf + haveSend, sendBuf->len - haveSend)) <= 0){
			return SERVER_CLOSED;
		}
		haveSend += onceSend;
	}

	return sendBuf->len;
}

void SendBufInit(struct SendBuf *sendBuf)
{
	sendBuf->len = 0;
	sendBuf->buf = NULL;

	return;
}

Status SendBufAlloc(struct SendBuf *sendBuf, int length)
{
	if((sendBuf->buf = (char *)malloc(length * 1)) == NULL)
		return ERROR;

	sendBuf->len = length;

	return OK;
}

Status SendBufFree(struct SendBuf *sendBuf)
{
	if(sendBuf->buf != NULL){
		free(sendBuf->buf);
		sendBuf->buf = NULL;
	}
	sendBuf->len = 0;

	return OK;
}
