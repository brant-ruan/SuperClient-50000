// mLog.c
// 1452334 Bonan Ruan
// Fri Apr 21 08:51:05 2017

#ifndef M_PUBLIC_H
#define M_PUBLIC_H
#include "mPublic.h"
#endif

#ifndef M_PROTOCOL_H
#define M_PROTOCOL_H
#include "mProtocol.h"
#endif

#define OPEN_LOGFILE_MOD	"a"
#define PREFIX_LEN			21
#define STR_GATHER_LEN		32

const char LOG_FILE[] = "./ts.log";

Status GenPrefix(char *prefix, u_int devid)
{
	time_t t = time(0);
	struct tm *curTim = localtime(&t);
	sprintf(prefix, "%02d:%02d:%02d [%u] ", curTim->tm_hour, \
				curTim->tm_min, curTim->tm_sec, devid);
	return OK;
}

#define PER_LINE_CHR	16
#define HALF_LINE_CHR	PER_LINE_CHR / 2
#define PER_LINE_LEN	128
#define HEX_PART_LEN	PER_LINE_CHR * 3 + 2

void Format(char *perLine, char *buf, int linePos, int length)
{
	char hexPart[HEX_PART_LEN + 1] = {0};
	int i, j;
	int subLen = length - linePos;
	if(subLen <= HALF_LINE_CHR){ // no ' - '
		for(i = 0; i < subLen; i++){
			sprintf(hexPart + 3 * i, "%02x ", \
					(u_int)((u_int)buf[linePos + i] & 0x000000FF));
		}
		// for white spaces
		for(j = 0; j < HEX_PART_LEN - subLen * 3; j++){
			hexPart[subLen * 3 + j] = ' ';
		}
	}
	else{ // have ' - '
		subLen = subLen > PER_LINE_CHR ? PER_LINE_CHR : subLen;
		for(i = 0; i < HALF_LINE_CHR; i++){
			sprintf(hexPart + 3 * i, "%02x ", \
					(u_int)((u_int)buf[linePos + i] & 0x000000FF));
		}
		sprintf(hexPart + 3 * i, "- ");
		for(; i < subLen; i++){
			sprintf(hexPart + 3 * i + 2, "%02x ", \
					(u_int)((u_int)buf[linePos + i] & 0x000000FF));
		}
		// for white spaces
		for(j = 0; j < (HEX_PART_LEN - (subLen * 3 + 2)); j++){
			hexPart[subLen * 3 + 2 + j] = ' ';
		}
	}
	char chrPart[PER_LINE_CHR + 1] = {0};
	for(i = 0; i < subLen; i++){
		if(buf[linePos + i] <= '~' && buf[linePos + i] >= ' '){
			chrPart[i] = buf[linePos + i];
		}
		else{
			chrPart[i] = '.';
		}
	}

	sprintf(perLine, "  %04X:  %s %s\n", linePos, hexPart, chrPart);

	return;
}

/*
 * Log data sent/received to file in the format below:
 *   0000:  91 01 00 74 00 00 00 6c - 08 98 fc 81 d7 24 94 84  ...t...l.....$..
 * 	 0010:  64 1f 51 fb 25 99 d5 9c - 72 24 1b 0d 31 4a ad 1d  d.Q.%...r$..1J..
 */
Status LogData(u_int devid, u_char RDWR, char *buf, int length)
{
	char prefix[PREFIX_LEN + 1] = {0}; // "21:24:45 [110101001] "
	GenPrefix(prefix, devid);

	FILE *fp = fopen(LOG_FILE, OPEN_LOGFILE_MOD);
	if(fp == NULL){
		perror("fopen");
		return ERROR;
	}

	char strRDWR[][8] = {"读取", "发送"};
	int pos = 0;
	if(RDWR == RDWR_WR){
		pos = 1;
	}
	char strGather[STR_GATHER_LEN + 1] = {0};
	sprintf(strGather, "%s%d字节\n", strRDWR[pos], length);

	fwrite(prefix, sizeof(char), PREFIX_LEN, fp);
	fwrite(strGather, sizeof(char), strlen(strGather), fp);
	fwrite(prefix, sizeof(char), PREFIX_LEN, fp);

	sprintf(strGather, "(%s数据为:)\n", strRDWR[pos]);
	fwrite(strGather, sizeof(char), strlen(strGather), fp);

	char perLine[PER_LINE_LEN + 1] = {0};
	int linePos = 0;
	while(linePos < length){
		Format(perLine, buf, linePos, length);
		fwrite(perLine, sizeof(char), strlen(perLine), fp);
		linePos += PER_LINE_CHR;
	}
	fclose(fp);
	return OK;
}

/*
 * Just log one string to file
 */
Status LogStr(u_int devid, char *str, int length)
{
	char prefix[PREFIX_LEN + 1] = {0}; // "21:24:45 [110101001] "
	GenPrefix(prefix, devid);

	FILE *fp = fopen(LOG_FILE, OPEN_LOGFILE_MOD);
	if(fp == NULL){
		perror("fopen");
		return ERROR;
	}
	fwrite(prefix, sizeof(char), PREFIX_LEN, fp);
	fwrite(str, sizeof(char), length, fp);

	fclose(fp);
	return OK;
}

/*
 * log something like these:
 *	 21:24:45 [110101001] 收到客户端状态请求[intf=系统信息]
 * 	 21:24:45 [110101001] 发送客户端状态应答[intf=系统信息 len=28]
 */
Status LogState(u_int devid, u_char RDWR, char *info, int length)
{
	char prefix[PREFIX_LEN + 1] = {0}; // "21:24:45 [110101001] "
	GenPrefix(prefix, devid);

	char strRDWR[][8] = {"收到", "发送"};
	char strState[][8] = {"请求", "应答"};
	int pos = 0;
	if(RDWR == RDWR_WR){
		pos = 1;
	}
	char strGather[STR_GATHER_LEN + 1] = {0};
	sprintf(strGather, "%s客户端状态%s", strRDWR[pos], strState[pos]);

	FILE *fp = fopen(LOG_FILE, OPEN_LOGFILE_MOD);
	if(fp == NULL){
		perror("fopen");
		return ERROR;
	}

	fwrite(prefix, sizeof(char), PREFIX_LEN, fp);
	fwrite(strGather, sizeof(char), strlen(strGather), fp);

	if(RDWR == RDWR_RD){
		sprintf(strGather, "[intf=%s]\n", info);
	}
	else{
		sprintf(strGather, "[intf=%s len=%d]\n", info, length);
	}

	fwrite(strGather, sizeof(char), strlen(strGather), fp);

	fclose(fp);
	return OK;
}
