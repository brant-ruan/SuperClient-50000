// mClient.c
// 1452334 Bonan Ruan
// Mon Apr 17 20:09:42 2017

#ifndef M_PUBLIC_H
#define M_PUBLIC_H
#include "mPublic.h"
#endif

#ifndef M_PROTOCOL_H
#define M_PROTOCOL_H
#include "mProtocol.h"
#endif

/* simulator of client */
int ClientSimulate(u_int devid, struct ConfigFile* configFile)
{
	// for ts_count.xls
	u_int xls_vScrNum = 0;
	u_int xls_termNum = 0;
	// deal with socket
	int cSocket;
	if(CSocket(&cSocket, configFile->serverIP, configFile->serverPort) == ERROR){
		LogStr(devid, "Connected failed\n", strlen("Connected failed\n"));
		perror("CSocket");
		return ERROR;
	}
	LogStr(devid, "Connected OK.\n", strlen("Connected OK.\n"));
	// -> recv from server
	// -> parse (remain to log)
	// -> generate response data (remain to log)
	// -> send data
	struct SC_Identify sc_iden; // for identification
	char headerBuf[HEADER_LEN];
	int flag = 0; // used in <if>
	while(1){
		// recv one header
		flag = RecvN(&cSocket, headerBuf, HEADER_LEN); // receive one header
		if(flag != HEADER_LEN){
			if(flag == SERVER_CLOSED) // server closes connection or error occurs
				break;
		}
		// parse the header (and generate data to send)
		struct Header *recvHeader;
		recvHeader = (struct Header *)headerBuf;
		LogData(devid, RDWR_RD, headerBuf, HEADER_LEN);
		HeaderN2H(recvHeader); // convert network order to host order for header

		struct SendBuf sendBuf;
		SendBufInit(&sendBuf);
		switch(recvHeader->pLType){
		case IDENTIFY_L:
			flag = RecvN(&cSocket, (char *)(&sc_iden), recvHeader->dataLen);
			if(flag != recvHeader->dataLen){
				if(flag == SERVER_CLOSED) // server closes connection
					goto Label_OK;
			}
			LogData(devid, RDWR_RD, (char *)(&sc_iden), recvHeader->dataLen);
			IdenN2H(&sc_iden); // network order to host order
	//		printf("svrTime: %u\n", sc_iden.svrTime);
			int isOldVersion = NO;
			// version is less than required
			if((isOldVersion = IsOldVersion(sc_iden.mainVersion, sc_iden.subVersion1, sc_iden.subVersion2)) == YES){
				if(GenMinVer(&sendBuf) == ERROR){
					perror("GenMinVer");
					SendBufFree(&sendBuf);
					goto Label_ERROR;
				}
				LogStr(devid, "未达到最低版本要求\n", strlen("未达到最低版本要求\n"));
			}
			else if(OutOfDate(devid, &(sc_iden.svrTime)) == YES){ // certificate out of date
				LogStr(devid, "数字证书过期\n", strlen("数字证书过期\n"));
				goto Label_OK;
			}
			else if(Identify(&sc_iden) == NO){ // identify failed
				LogStr(devid, "认证非法\n", strlen("认证非法\n"));
				goto Label_OK;
			}
			else{ // generate identification
				if((GenIdentify(&sendBuf, &sc_iden, devid) == ERROR)){
					perror("GenIdentify");
					SendBufFree(&sendBuf);
					goto Label_ERROR;
				}
			}
			char reTime[32] = {0};
			sprintf(reTime, "收到连接间隔=%d/采样间隔=%d\n", sc_iden.retryTime, sc_iden.reTransTime);
			LogStr(devid, reTime, strlen(reTime));
			LogState(devid, RDWR_RD, "认证信息", 0);
			if(isOldVersion == YES){
				LogState(devid, RDWR_WR, "最低版本信息", sendBuf.len);
			}
			else{
				LogState(devid, RDWR_WR, "认证信息", sendBuf.len);
			}
			break;
		case SYSINFO_L:
			LogState(devid, RDWR_RD, "系统信息", 0);
			if(GenSysInfo(&sendBuf) == ERROR){
				perror("GenSysInfo");
				SendBufFree(&sendBuf);
				goto Label_ERROR;
			}
			LogState(devid, RDWR_WR, "系统信息", sendBuf.len);
			break;
		case CFGINFO_L:
			LogState(devid, RDWR_RD, "配置信息", 0);
			if(GenCfgInfo(&sendBuf) == ERROR){
				perror("GenCfgInfo");
				SendBufFree(&sendBuf);
				goto Label_ERROR;
			}
			LogState(devid, RDWR_WR, "配置信息", sendBuf.len);
			break;
		case PROCINFO_L:
			LogState(devid, RDWR_RD, "进程信息", 0);
			if(GenProcInfo(&sendBuf) == ERROR){
				perror("GenProcInfo");
				SendBufFree(&sendBuf);
				goto Label_ERROR;
			}
			LogState(devid, RDWR_WR, "进程信息", sendBuf.len);
			break;
		case ETHINFO_L:
			if(recvHeader->ID == ETH_0_ID){
				LogState(devid, RDWR_RD, "Ethernet0口信息", 0);
			}
			else{
				LogState(devid, RDWR_RD, "Ethernet1口信息", 0);
			}
			if(GenEthInfo(&sendBuf, recvHeader) == ERROR){
				perror("GenEthInfo");
				SendBufFree(&sendBuf);
				goto Label_ERROR;
			}
			if(recvHeader->ID == ETH_0_ID){
				LogState(devid, RDWR_WR, "Ethernet0口信息", sendBuf.len);
			}
			else{
				LogState(devid, RDWR_WR, "Ethernet1口信息", sendBuf.len);
			}
			break;
		case USBINFO_L:
			LogState(devid, RDWR_RD, "USB口信息", 0);
			if(GenUSBInfo(&sendBuf) == ERROR){
				perror("GenUSBInfo");
				SendBufFree(&sendBuf);
				goto Label_ERROR;
			}
			LogState(devid, RDWR_WR, "USB口信息", sendBuf.len);
			break;
		case USBFILEINFO_L:
			LogState(devid, RDWR_RD, "U盘文件列表信息", 0);
			if(GenUSBFileInfo(&sendBuf) == ERROR){
				perror("GenUSBFileInfo");
				SendBufFree(&sendBuf);
				goto Label_ERROR;
			}
			LogState(devid, RDWR_WR, "U盘文件列表信息", sendBuf.len);
			break;
		case PRINTINFO_L:
			LogState(devid, RDWR_RD, "打印口信息", 0);
			if(GenPrintInfo(&sendBuf) == ERROR){
				perror("GenPrintFileInfo");
				SendBufFree(&sendBuf);
				goto Label_ERROR;
			}
			LogState(devid, RDWR_WR, "打印口信息", sendBuf.len);
			break;
		case PRINTQUEUEINFO_L:
			LogState(devid, RDWR_RD, "打印队列信息", 0);
			if(GenPrintQueueInfo(&sendBuf) == ERROR){
				perror("GenPrintQueueFileInfo");
				SendBufFree(&sendBuf);
				goto Label_ERROR;
			}
			LogState(devid, RDWR_WR, "打印队列信息", sendBuf.len);
			break;
		case TERMINFO_L:
			LogState(devid, RDWR_RD, "终端服务信息", 0);
			if(GenTermInfo(&sendBuf, configFile, &xls_termNum) == ERROR){
				perror("GenTermFileInfo");
				SendBufFree(&sendBuf);
				goto Label_ERROR;
			}
			LogState(devid, RDWR_WR, "终端服务信息", sendBuf.len);
			break;
		case SUBTERMINFO_L_1: // Dumb Terminal
		case SUBTERMINFO_L_2: // IP terminal
			if(recvHeader->pLType == SUBTERMINFO_L_1)
				LogState(devid, RDWR_RD, "哑终端+对应虚屏配置信息", 0);
			else
				LogState(devid, RDWR_RD, "IP终端+对应虚屏配置信息", 0);
			if(GenSubTermInfo(&sendBuf, recvHeader, configFile) == ERROR){
				perror("GenSubTermInfo");
				SendBufFree(&sendBuf);
				goto Label_ERROR;
			}
			xls_vScrNum += ((struct CS_SubTermInfo *)sendBuf.buf)->vScrNum;
			if(recvHeader->pLType == SUBTERMINFO_L_1)
				LogState(devid, RDWR_WR, "哑终端+对应虚屏配置信息", sendBuf.len);
			else
				LogState(devid, RDWR_WR, "IP终端+对应虚屏配置信息", sendBuf.len);
			break;
		case END_L:
			LogState(devid, RDWR_RD, "本次接收完成", 0);
			if(GenEnd(&sendBuf) == ERROR){
				perror("GenEnd");
				SendBufFree(&sendBuf);
				goto Label_ERROR;
			}
			LogState(devid, RDWR_WR, "本次接收完成", sendBuf.len);
			break;
		default:
			fprintf(stderr, "Undefined Protocol pLType detected\n");
			goto Label_ERROR;
			break;
		}
		// send data
		flag = SendN(&cSocket, &sendBuf);
		if(flag != sendBuf.len){
			if(flag == SERVER_CLOSED){
				SendBufFree(&sendBuf);
				break;
			}
		}
		LogData(devid, RDWR_WR, sendBuf.buf, sendBuf.len);
		// free sendBuf
		SendBufFree(&sendBuf);
	}

Label_OK:
	LogStr(devid, "读取数据完成，被对端关闭\n", strlen("读取数据完成，被对端关闭\n"));
	time_t t = time(0);
	struct tm *curTim = localtime(&t);
	char xls_buf[32] = {0};
	sprintf(xls_buf, "%02d:%02d:%02d\t%u\t1\t%u\t%u\n", curTim->tm_hour, \
			curTim->tm_min, curTim->tm_sec, devid, xls_termNum, xls_vScrNum);
	int xls_fd = open("./ts_count.xls", O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
	if(xls_fd == -1){
		perror("xls open");
	}
	else{
		if(write(xls_fd, xls_buf, strlen(xls_buf)) == -1){
			perror("write");
		}
		close(xls_fd);
	}
	remove(CPU_RAM_FILE);
	close(cSocket);
	return sc_iden.reTransTime; // attention. We return the retrans time to main by the way

Label_ERROR:
	close(cSocket);
	return ERROR;
}
