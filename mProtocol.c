// mProtocol.c
// 1452334 Bonan Ruan
// Wed Apr 19 08:27:15 2017

#ifndef M_PUBLIC_H
#define M_PUBLIC_H
#include "mPublic.h"
#endif

#ifndef M_PROTOCOL_H
#define M_PROTOCOL_H
#include "mProtocol.h"
#endif

#ifndef M_ENCRYPT_H
#define M_ENCRYPT_H
#include "mEncrypt.h"
#endif

const char plainText[] = "yzmond:id*str&to!tongji@by#Auth^";

/* set header when generating package */
void SetHeader(struct Header *header, struct SendBuf *sendBuf, u_char pLType, u_short ID)
{
	header->pHType = CS_H;
	header->pLType = pLType;
	header->len = htons(sendBuf->len);
	header->ID = htons(ID);
	header->dataLen = htons(sendBuf->len - HEADER_LEN);

	return;
}

#define CR_BUF_LEN	16
/* fetch CPU & RAM infomation from OS */
Status SetCPURAM(struct CS_Identify *res)
{
	const char GET_CPU_INFO[] = "cat /proc/cpuinfo | grep \"cpu MHz\" | head -n 1 | cut -d\' \' -f 3 > " CPU_RAM_FILE;
	const char GET_RAM_INFO[] = "cat /proc/meminfo | head -n 1 | cut  -d\":\" -f 2 >> " CPU_RAM_FILE;
	system(GET_CPU_INFO);
	system(GET_RAM_INFO);
	FILE *fp;
	fp = fopen(CPU_RAM_FILE, "r");
	if(fp == NULL){
		perror("fopen");
		return ERROR;
	}
	char buf[CR_BUF_LEN + 1] = {0};
	int tempChr = 0;
	int i = 0;
	while((tempChr = fgetc(fp)) != '\n' && tempChr != ' ' && tempChr != '\t'
		&& tempChr != EOF && i < CR_BUF_LEN)
		buf[i++] = tempChr;

	res->CPUHz = htons((u_short)(atoi(buf)));

	memset(buf, '\0', CR_BUF_LEN);
	i = 0;
	while(((tempChr = fgetc(fp)) == ' ' || tempChr == '\n' || tempChr == '\t')
		&& tempChr != EOF)
		;
	buf[i++] = tempChr;
	while((tempChr = fgetc(fp)) <= '9' && tempChr >= '0'
		&& tempChr != EOF && i < CR_BUF_LEN)
		buf[i++] = tempChr;

	res->RAM = htons((u_short)(atoi(buf) / 1024));

	fclose(fp);
	return OK;
}

/* generate a string with random characters */
void RandStr(char *array, int length)
{
	const char candidate[] = "abcdefghijklmnopqrstuvwxyz"
							 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
							 "0123456789";
	int i;
	for(i = 0; i < length; i++){
		array[i] = candidate[rand() % (strlen(candidate))];
	}

	return;
}

/* check verion of server */
Status IsOldVersion(u_short mainVer, u_char subVer1, u_char subVer2)
{
	if(mainVer < MAIN_VERSION_MIN)
		return YES;
	else if(mainVer == MAIN_VERSION_MIN
			&& subVer1 < SUB_VERSION_1_MIN)
		return YES;
	else if(mainVer == MAIN_VERSION_MIN
			&& subVer1 == SUB_VERSION_1_MIN
			&& subVer2 < SUB_VERSION_2_MIN)
		return YES;

	return NO;
}

#define SVR_YEAR	2017
#define SVR_MON		0 // 1 month
#define SVR_DAY		1
#define YEAR_BASE	1900
Status OutOfDate(u_int devid, u_int *serverTime)
{
	struct tm svrTm;
	struct tm *myTm;
	myTm = localtime((time_t *)serverTime);
	memcpy(&svrTm, localtime((time_t *)serverTime), sizeof(struct tm));
	// log server time
	char strSvrTime[40] = {0};

	sprintf(strSvrTime, "收到服务器的时间:%d-%02d-%02d %02d:%02d:%02d\n", \
			(svrTm.tm_year) + 1900, svrTm.tm_mon + 1, \
			svrTm.tm_mday, svrTm.tm_hour, \
			svrTm.tm_min, svrTm.tm_sec);
	LogStr(devid, strSvrTime, strlen(strSvrTime));

	if((svrTm.tm_year + YEAR_BASE) < SVR_YEAR){
		return YES;
	}

	return NO;
}

/* convert network order to host order */
Status HeaderN2H(struct Header *header)
{
	header->len = ntohs(header->len);
	header->dataLen = ntohs(header->dataLen);
	header->ID = ntohs(header->ID);

	return OK;
}

/* convert network order to host order */
Status IdenN2H(struct SC_Identify *sc_iden)
{
	sc_iden->mainVersion = ntohs(sc_iden->mainVersion);
	sc_iden->retryTime = ntohs(sc_iden->retryTime);
	sc_iden->reTransTime = ntohs(sc_iden->reTransTime);
	sc_iden->randomNum = ntohl(sc_iden->randomNum);
	sc_iden->svrTime = ntohl(sc_iden->svrTime);
	sc_iden->svrTime ^= (u_int)0xFFFFFFFF;
	return OK;
}

/* Identify data from server */
Status Identify(struct SC_Identify *sc_iden)
{
	int pos = sc_iden->randomNum % 4093;

	char decryptText[TOKEN_LEN + 1] = {0};

	int i;
	for(i = 0; i < TOKEN_LEN; i++){
		decryptText[i] = secret[(pos + i) % 4096] ^ sc_iden->token[i];
	}

	if(strncmp(decryptText, plainText, TOKEN_LEN) == 0)
		return YES;

	return NO;
}

/* generate min-version package */
Status GenMinVer(struct SendBuf *sendBuf)
{
	if(SendBufAlloc(sendBuf, sizeof(struct CS_MinVerReq)) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}

	struct CS_MinVerReq *req;
	req = (struct CS_MinVerReq *)(sendBuf->buf);
	SetHeader(&(req->header), sendBuf, MINVERREQ_L, DEFAULT_ID);
	req->mainVersion = htons(MAIN_VERSION_MIN);
	req->subVersion1 = SUB_VERSION_1_MIN;
	req->subVersion2 = SUB_VERSION_2_MIN;

	return OK;
}

/* generate identification & basic configuration package */
Status GenIdentify(struct SendBuf *sendBuf, struct SC_Identify *sc_iden, u_int devid)
{
	if(SendBufAlloc(sendBuf, sizeof(struct CS_Identify)) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}
	struct CS_Identify *res;
	res = (struct CS_Identify *)(sendBuf->buf);
	SetHeader(&(res->header), sendBuf, IDENTIFY_L, DEFAULT_ID);

	if(SetCPURAM(res) == ERROR){
		perror("SetCPURAM");
		// do not return because this error is not critical
	}
	res->Flash = htons((u_short)(rand() % 65533));
	res->UID = htons((u_short)(rand() % 65533));
	RandStr(res->GID, DEV_GID_LEN);
	RandStr(res->TID, DEV_ID_LEN);
	RandStr(res->VID, DEV_VER_LEN);
	res->ethNum = ETH_NUM; // casually
	res->syncNum = SYNC_NUM; // casually
	res->asnycNum = ASYNC_NUM; // casually
	res->transNum = TRANS_NUM; // casually
	res->USBNum = USB_NUM; // casually
	res->printNum = PRINT_NUM; // casually
	res->devid = htonl(devid); // is it right?
	res->devidIn = DEVID_IN_DEFAULT;
	memcpy(res->token, plainText, TOKEN_LEN);

	u_int randomNum;
	int pos;
	randomNum = (u_int)rand();
	pos = randomNum % 4093;
	int i;
	for(i = 0; i < (sendBuf->len - HEADER_LEN - sizeof(u_int)); i++){
		sendBuf->buf[HEADER_LEN + i] ^= secret[(pos + i) % 4096];
	}

	res->randomNum = htonl(randomNum);

	return OK;
}

const char STAT_FILE[] = "/proc/stat";
const char MEM_INFO_FILE[] = "/proc/meminfo";
#define CPU_TIME_BUF_LEN	128
#define MEM_INFO_BUF_LEN	128
void SkipNonNum(char *buf, int *i, int length)
{
	while((*i < length) && (buf[*i] <= '9' && buf[*i] >= '0')){
		(*i)++;
	}
	while((*i < length) && (buf[*i] > '9' || buf[*i] < '0')){ // non-number
		(*i)++;
	}
}

/* get information from /proc/stat */
Status SetCPUTime(struct CS_SysInfo *res)
{
	FILE *fp;
	if((fp = fopen(STAT_FILE, "r")) == NULL){
		perror("fopen");
		return ERROR;
	}

	char cpuTimeBuf[CPU_TIME_BUF_LEN + 1] = {0};
	fgets(cpuTimeBuf, CPU_TIME_BUF_LEN, fp);

	int i = 0;
	while(cpuTimeBuf[i] > '9' || cpuTimeBuf[i] < '0') // non-number
		i++;
	res->uCPUTime = htonl((u_int)(atoi(cpuTimeBuf + i)));

	SkipNonNum(cpuTimeBuf, &i, CPU_TIME_BUF_LEN);
	res->nCPUTime = htonl((u_int)(atoi(cpuTimeBuf + i)));

	SkipNonNum(cpuTimeBuf, &i, CPU_TIME_BUF_LEN);
	res->sCPUTime = htonl((u_int)(atoi(cpuTimeBuf + i)));

	SkipNonNum(cpuTimeBuf, &i, CPU_TIME_BUF_LEN);
	res->iCPUTime = htonl((u_int)(atoi(cpuTimeBuf + i)));

	fclose(fp);
	return OK;
}

/* get information from /proc/mem */
Status SetFreeMem(struct CS_SysInfo *res)
{
	FILE *fp;
	if((fp = fopen(MEM_INFO_FILE, "r")) == NULL){
		perror("fopen");
		return ERROR;
	}
	int i = 0;
	u_int tempUInt = 0;
	char memInfoBuf[MEM_INFO_BUF_LEN + 1] = {0};
	 // 2nd entry: MemFree
	fgets(memInfoBuf, MEM_INFO_BUF_LEN, fp);
	fgets(memInfoBuf, MEM_INFO_BUF_LEN, fp);
	SkipNonNum(memInfoBuf, &i, MEM_INFO_BUF_LEN);
	tempUInt = (u_int)(atoi(memInfoBuf + i));
	// 4th entry: Buffers
	fgets(memInfoBuf, MEM_INFO_BUF_LEN, fp);
	fgets(memInfoBuf, MEM_INFO_BUF_LEN, fp);
	i = 0;
	SkipNonNum(memInfoBuf, &i, MEM_INFO_BUF_LEN);
	tempUInt += (u_int)(atoi(memInfoBuf + i));
	// 5th entry: Cached
	fgets(memInfoBuf, MEM_INFO_BUF_LEN, fp); // 5nd entry
	i = 0;
	SkipNonNum(memInfoBuf, &i, MEM_INFO_BUF_LEN);
	tempUInt += (u_int)(atoi(memInfoBuf + i));

	res->freedMem = htonl(tempUInt);

	fclose(fp);
	return OK;
}

/* generate system information package */
Status GenSysInfo(struct SendBuf *sendBuf)
{
	if(SendBufAlloc(sendBuf, sizeof(struct CS_SysInfo)) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}

	struct CS_SysInfo *res;
	res = (struct CS_SysInfo *)(sendBuf->buf);
	SetHeader(&(res->header), sendBuf, SYSINFO_L, DEFAULT_ID);

	SetCPUTime(res);
	SetFreeMem(res);

	return OK;
}

#define CFG_INFO_LEN	8191
/* generate configuration information package */
Status GenCfgInfo(struct SendBuf *sendBuf)
{
	const char CFG_INFO_FILE[] = "./config.dat";
	char *tempBuf = (char *)malloc((CFG_INFO_LEN + 1) * sizeof(char));
	if(tempBuf == NULL){
		perror("malloc");
		return ERROR;
	}
	int length = 0;
	FILE *fp = fopen(CFG_INFO_FILE, "r");
	if(fp == NULL){
		perror("fopen");
		return ERROR;
	}
	length = fread(tempBuf, sizeof(char), CFG_INFO_LEN, fp);
	if(length <= 0){
		perror("fread");
		free(tempBuf);
		return ERROR;
	}
	tempBuf[length] = '\0';

	if(SendBufAlloc(sendBuf, (HEADER_LEN + length + 1)) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}
	struct Header *header = (struct Header *)(sendBuf->buf);
	SetHeader(header, sendBuf, CFGINFO_L, DEFAULT_ID);
	memcpy(sendBuf->buf + HEADER_LEN, tempBuf, length + 1);

	free(tempBuf);
	fclose(fp);

	return OK;
}

#define PROC_INFO_LEN	8191

/* generate process information package */
Status GenProcInfo(struct SendBuf *sendBuf)
{
	const char PROC_INFO_FILE[] = "./process.dat";

	char *tempBuf = (char *)malloc((PROC_INFO_LEN + 1) * sizeof(char));
	if(tempBuf == NULL){
		perror("malloc");
		return ERROR;
	}
	int length = 0;
	FILE *fp = fopen(PROC_INFO_FILE, "r");
	if(fp == NULL){
		perror("fopen");
		return ERROR;
	}
	length = fread(tempBuf, sizeof(char), PROC_INFO_LEN, fp);
	if(length <= 0){
		perror("fread");
		free(tempBuf);
		return ERROR;
	}
	tempBuf[length] = '\0';

	if(SendBufAlloc(sendBuf, (HEADER_LEN + length + 1)) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}
	struct Header *header = (struct Header *)(sendBuf->buf);
	SetHeader(header, sendBuf, PROCINFO_L, DEFAULT_ID);
	memcpy(sendBuf->buf + HEADER_LEN, tempBuf, length + 1);

	free(tempBuf);
	fclose(fp);

	return OK;
}

#define ETH_EXISTED		1
#define ETH_CONFIGURED	1
#define ETH_UP_DOWN		1
#define ETH_MAC			"abcdef"
#define ETH_OPT_HMB		0x0001
#define ETH_OPT_MOD		0x0002
#define ETH_OPT_AUTO	0x0004
#define ETH_OPT			ETH_OPT_HMB | ETH_OPT_MOD | ETH_OPT_AUTO
#define ETH_IP_ADDR		(127 << 24) + 1 // 127.0.0.1
#define ETH_NET_MASK	(255 << 24) + (255 << 16) + (255 << 8) + (255)

#define ETH_INFO_BUF_LEN	256
/* get information from /proc/net/dev */
Status SetRecordData(struct CS_EthInfo *res, struct Header *svrHeader, const char *ethFile)
{
	FILE *fp;
	if((fp = fopen(ethFile, "r")) == NULL){
		perror("fopen");
		return ERROR;
	}

	int i = 0;
	u_int tempUInt = 0;
	char ethInfoBuf[ETH_INFO_BUF_LEN + 1] = {0};

	fgets(ethInfoBuf, ETH_INFO_BUF_LEN, fp);
	fgets(ethInfoBuf, ETH_INFO_BUF_LEN, fp);
	fgets(ethInfoBuf, ETH_INFO_BUF_LEN, fp); // eth0
	if(svrHeader->ID == ETH_1_ID) // eth1
		fgets(ethInfoBuf, ETH_INFO_BUF_LEN, fp);

	while(ethInfoBuf[i] != ':'){
		i++;
	}
	// received
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->recvByte = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->recvPack = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->recvErr = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->recvDrop = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->recvFifo = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->recvFrame = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->recvComp = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->recvMCast = htonl((u_int)(atoi(ethInfoBuf + i)));
	// sent
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->sendByte = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->sendPack = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->sendErr = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->sendDrop = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->sendFifo = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->sendFrame = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->sendComp = htonl((u_int)(atoi(ethInfoBuf + i)));
	SkipNonNum(ethInfoBuf, &i, ETH_INFO_BUF_LEN);
	res->sendMCast = htonl((u_int)(atoi(ethInfoBuf + i)));

	fclose(fp);
	return OK;
}

/* generate ethernet information package
 * Two types of package:
 * 		ETH_0_ID
 * 		ETH_1_ID
 */
Status GenEthInfo(struct SendBuf *sendBuf, struct Header *svrHeader)
{
	const char NET_DEV_FILE[] = "/proc/net/dev";

	if(SendBufAlloc(sendBuf, sizeof(struct CS_EthInfo)) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}

	struct CS_EthInfo *res;
	res = (struct CS_EthInfo *)(sendBuf->buf);
	SetHeader(&(res->header), sendBuf, ETHINFO_L, svrHeader->ID);

	res->existed = ETH_EXISTED;
	res->configured = ETH_CONFIGURED;
	res->upDown = ETH_UP_DOWN;
	memcpy(res->MAC, ETH_MAC, MAC_LEN);
	res->options = htons(ETH_OPT);
	res->IP = htonl(ETH_IP_ADDR);
	res->mask = htonl(ETH_NET_MASK);
	res->subIfaceIP1 = htonl(ETH_IP_ADDR + 1);
	res->subIfaceMask1 = htonl(ETH_NET_MASK);
	res->subIfaceIP2 = htonl(ETH_IP_ADDR + 2);
	res->subIfaceMask2 = htonl(ETH_NET_MASK);
	res->subIfaceIP3 = htonl(ETH_IP_ADDR + 3);
	res->subIfaceMask3 = htonl(ETH_NET_MASK);
	res->subIfaceIP4 = htonl(ETH_IP_ADDR + 4);
	res->subIfaceMask4 = htonl(ETH_NET_MASK);
	res->subIfaceIP5 = htonl(ETH_IP_ADDR + 5);
	res->subIfaceMask5 = htonl(ETH_NET_MASK);

	SetRecordData(res, svrHeader, NET_DEV_FILE);

	return OK;
}

#define U_DISK_INSERTED		1

/* generate whether u-disk inserted package */
Status GenUSBInfo(struct SendBuf *sendBuf)
{
	if(SendBufAlloc(sendBuf, sizeof(struct CS_USBInfo)) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}

	struct CS_USBInfo *res;
	res = (struct CS_USBInfo *)(sendBuf->buf);
	SetHeader(&(res->header), sendBuf, USBINFO_L, DEFAULT_ID);
	res->uDiskIn = U_DISK_INSERTED;

	return OK;
}

#define USB_FILE_INFO_LEN		4095

/* generate file list on the u-disk inserted */
Status GenUSBFileInfo(struct SendBuf *sendBuf)
{
	const char USB_INFO_FILE[] = "./usbfiles.dat";

	char *tempBuf = (char *)malloc((USB_FILE_INFO_LEN + 1) * sizeof(char));
	if(tempBuf == NULL){
		perror("malloc");
		return ERROR;
	}
	int length = 0;
	FILE *fp = fopen(USB_INFO_FILE, "r");
	if(fp == NULL){
		perror("fopen");
		return ERROR;
	}
	length = fread(tempBuf, sizeof(char), USB_FILE_INFO_LEN, fp);
	if(length <= 0){
		perror("fread");
		free(tempBuf);
		return ERROR;
	}
	tempBuf[length] = '\0';

	if(SendBufAlloc(sendBuf, (HEADER_LEN + length + 1)) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}
	struct Header *header = (struct Header *)(sendBuf->buf);
	SetHeader(header, sendBuf, USBFILEINFO_L, DEFAULT_ID);
	memcpy(sendBuf->buf + HEADER_LEN, tempBuf, length + 1);

	free(tempBuf);
	fclose(fp);

	return OK;
}

#define SERVICE_ON	1

/* generate print port information package */
Status GenPrintInfo(struct SendBuf *sendBuf)
{
	if(SendBufAlloc(sendBuf, sizeof(struct CS_PrintInfo)) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}

	struct CS_PrintInfo *res;
	res = (struct CS_PrintInfo *)(sendBuf->buf);
	SetHeader(&(res->header), sendBuf, PRINTINFO_L, DEFAULT_ID);
	res->isOn = SERVICE_ON;
	res->taskNum = htons((u_short)(rand() % 26));
	RandStr(res->printerName, PRINTER_NAME_LEN);

	return OK;
}

/* generate print queue information package */
Status GenPrintQueueInfo(struct SendBuf *sendBuf)
{
	if(SendBufAlloc(sendBuf, sizeof(struct CS_PrintQueueInfo)) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}

	struct CS_PrintQueueInfo *res;
	res = (struct CS_PrintQueueInfo *)(sendBuf->buf);
	SetHeader(&(res->header), sendBuf, PRINTQUEUEINFO_L, DEFAULT_ID);
	res->info = '\0';

	return OK;
}

/* randomly select some bytes and set them to 1, else to 0 */
void SetTerminal(u_char *terminal, int length, int numOf1)
{
	memset(terminal, 0, length); // set all to 0 firstly
	int k = 0;
	while(k < numOf1){
		int pos = rand() % numOf1;
		if(terminal[pos] == 0){
			terminal[pos] = 1;
			k++;
		}
	}

	return;
}

/* generate terminal service information package */
Status GenTermInfo(struct SendBuf *sendBuf, \
		struct ConfigFile *configFile, u_int *xls_termNum)
{
	if(SendBufAlloc(sendBuf, sizeof(struct CS_TermInfo)) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}
	srand(time(0));
	struct CS_TermInfo *res;
	res = (struct CS_TermInfo *)(sendBuf->buf);
	SetHeader(&(res->header), sendBuf, TERMINFO_L, DEFAULT_ID);

	u_int total = (u_int)(rand() % (configFile->termNumMax - configFile->termNumMin) \
						+ configFile->termNumMin);
	u_int asyncTermNum = (u_int)((rand() % ASYNC_NUM) + 1);
	if(asyncTermNum > total)
		total = asyncTermNum;
	*xls_termNum = total;
//	printf("xls_termNum: %u\n", *xls_termNum);
	SetTerminal(res->vTermUsed, V_TERM_NUM, asyncTermNum);


	u_int ipTermNum = total - asyncTermNum;

	SetTerminal(res->ipTermUsed, IP_TERM_NUM, ipTermNum);

	res->termNum = htons((u_short)(rand() % (270 - total) + total));
	return OK;
}

/* generate dumb terminal/IP terminal information */
Status GenSubTermInfo(struct SendBuf *sendBuf, \
		struct Header *svrHeader, struct ConfigFile *configFile)
{
	// generate number of screen first
	u_int screenNum = (u_int)(rand() % (configFile->vScrNumMax - configFile->vScrNumMin) \
							+ configFile->vScrNumMin);
	// one terminal package + screenNum * virtual screen packages
	if(SendBufAlloc(sendBuf, \
			(sizeof(struct CS_SubTermInfo) + screenNum * sizeof(struct VScreen))) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}
	srand(time(0));
	struct CS_SubTermInfo *res;
	res = (struct CS_SubTermInfo *)(sendBuf->buf);
	SetHeader(&(res->header), sendBuf, svrHeader->pLType, svrHeader->ID);

	res->port = (u_char)(svrHeader->ID);
	if(svrHeader->pLType == SUBTERMINFO_L_1){
		res->portCfg = (u_char)(rand() % 16 + 1);
	}
	else{
		res->portCfg = (u_char)(rand() % 254 + 1);
	}
	res->actVScr = (u_char)(rand() % screenNum);
	res->vScrNum = (u_char)screenNum;
	if(svrHeader->pLType == SUBTERMINFO_L_1){ // dumb terminal
		res->termIP = htonl(0);
		strcpy(res->termType, "串口终端");
	}
	else{ // IP terminal
		res->termIP = htonl(ETH_IP_ADDR);
		strcpy(res->termType, "IP终端");
	}
	if(rand() % 2){
		strcpy(res->termStat, "正常");
	}
	else{
		strcpy(res->termStat, "菜单");
	}

	char vStat[][8] = {"开机", "关机", "已登录"};
	struct VScreen *vScreen = (struct VScreen *)(sendBuf->buf + sizeof(struct CS_SubTermInfo));
	int i;
	for(i = 0; i < screenNum; i++){
		vScreen->ID = (u_char)(i+1);
		vScreen->port = htons(configFile->serverPort);
		vScreen->IP = htonl(ETH_IP_ADDR);
		strcpy(vScreen->vProtocol, "专用SSH");
		strcpy(vScreen->vStat, vStat[rand() % 3]);
		strcpy(vScreen->vPrompt, "储蓄系统");
		strcpy(vScreen->vTermType, "vt220");
		time_t curTime = time(0);
		struct tm *formTime = localtime(&curTime);
		u_int secTime = formTime->tm_hour * 3600 + formTime->tm_min * 60 \
				+ formTime->tm_sec;
		vScreen->connTime = htonl(secTime);
		vScreen->sendTByte = htonl(rand() % 4096);
		vScreen->recvTByte = htonl(rand() % 4096);
		vScreen->sendSByte = htonl(rand() % 4096);
		vScreen->recvSByte = htonl(rand() % 4096);
		vScreen->pingMax = htonl(rand() % 123456);
		vScreen->pingMin = htonl(rand() % 123456);
		vScreen->pingAvg = htonl(rand() % 123456);
		vScreen++; // next virtual screen
	}

	return OK;
}

/* generate end package */
Status GenEnd(struct SendBuf *sendBuf)
{
	if(SendBufAlloc(sendBuf, sizeof(struct CS_End)) == ERROR){
		perror("SendBufAlloc");
		return ERROR;
	}
	struct CS_End *res;
	res = (struct CS_End *)(sendBuf->buf);

	SetHeader(&(res->header), sendBuf, END_L, DEFAULT_ID);

	return OK;
}
