// mConfig.c
// 1452334 Bonan Ruan
// Mon Apr 17 19:51:15 2017

#ifndef M_PUBLIC_H
#define M_PUBLIC_H
#include "mPublic.h"
#endif

const char CONFIG_FILE[] = "./ts.conf";

#define CFG_KEY_LEN	64
#define CFG_VALUE_LEN	32

/* get next key entry in the configuration file*/
Status NextCfgKey(FILE *fp, char *cfgKey, int keyLen)
{
	int tempChr = 0;
	while(((tempChr = fgetc(fp)) == ' ' || tempChr == '\t')
		&& tempChr != EOF)
		; // strip white spaces

	if(tempChr == EOF)
		return NO;

	ungetc(tempChr, fp); // attention!

	int i = 0;
	while((tempChr = fgetc(fp)) != EOF && tempChr != '\n'
		&& tempChr != ' ' && tempChr != '\t' && tempChr != '#'
		&& i < keyLen){
		cfgKey[i++] = tempChr;
	}
	cfgKey[i] = '\0';

	if(tempChr == EOF)
		return NO;

	ungetc(tempChr, fp);

	return YES;
}

/* get next value entry in the configuration file*/
Status NextCfgValue(FILE *fp, char *cfgValue, int valueLen)
{
	int tempChr = 0;

	while(((tempChr = fgetc(fp)) == ' ' || tempChr == '\t')
		&& tempChr != EOF)
		; // strip white spaces

	ungetc(tempChr, fp); // attention!

	int i = 0;
	while((tempChr = fgetc(fp)) != EOF && tempChr != '\n'
		&& tempChr != ' ' && tempChr != '\t' && tempChr != '#'
		&& i < valueLen){
		cfgValue[i++] = tempChr;
	}
	cfgValue[i] = '\0';

	ungetc(tempChr, fp); // attention!

	while((tempChr = fgetc(fp)) != '\n' && tempChr != EOF)
		; // for next key

	if(tempChr == EOF)
		ungetc(tempChr, fp);

	return YES;
}

/* read flags of configurations */
#define CFG_PORT_Y	0x0001
#define CFG_PERC_Y	0x0002
#define CFG_EXIT_Y	0x0004
#define CFG_MINT_Y	0x0008
#define CFG_MAXT_Y	0x0010
#define CFG_MINV_Y	0x0020
#define CFG_MAXV_Y	0x0040
#define CFG_DELF_Y	0x0080
#define CFG_DBGC_Y	0x0100
#define CFG_DBGS_Y	0x0200

#define TERM_NUM_MIN_DEFAULT	5
#define TERM_NUM_MAX_DEFAULT	28
#define V_SCR_NUM_MIN_DEFAULT	3
#define V_SCR_NUM_MAX_DEFAULT	10

Status ConfigParse(struct ConfigFile* configFile)
{
	char cfgKey[CFG_KEY_LEN + 1] = {0};
	char cfgValue[CFG_VALUE_LEN + 1] = {0};
	u_short cfgFlag = 0;

	FILE *fp;
	fp = fopen(CONFIG_FILE, "r");
	if(fp == NULL){
		perror("fopen");
		return ERROR;
	}
	while(NextCfgKey(fp, cfgKey, CFG_KEY_LEN) == YES){
		NextCfgValue(fp, cfgValue, CFG_VALUE_LEN);
		if(strcmp(cfgKey, "服务器IP地址") == 0){
			strncpy(configFile->serverIP, cfgValue, IP_LEN);
		}
		else if(strcmp(cfgKey, "端口号") == 0){
			configFile->serverPort = (u_short)(atoi(cfgValue));
		}
		else if(strcmp(cfgKey, "进程接收成功后退出") == 0){
			configFile->quitAfterSucc = (char)(atoi(cfgValue));
		}
		else if(strcmp(cfgKey, "最小配置终端数量") == 0){
			configFile->termNumMin = atoi(cfgValue);
			cfgFlag |= CFG_MINT_Y;
		}
		else if(strcmp(cfgKey, "最大配置终端数量") == 0){
			configFile->termNumMax = atoi(cfgValue);
			cfgFlag |= CFG_MAXT_Y;
		}
		else if(strcmp(cfgKey, "每个终端最小虚屏数量") == 0){
			configFile->vScrNumMin = atoi(cfgValue);
			cfgFlag |= CFG_MINV_Y;
		}
		else if(strcmp(cfgKey, "每个终端最大虚屏数量") == 0){
			configFile->vScrNumMax = atoi(cfgValue);
			cfgFlag |= CFG_MAXV_Y;
		}
		else if(strcmp(cfgKey, "删除日志文件") == 0){
			configFile->delLog = atoi(cfgValue);
		}
		else if(strcmp(cfgKey, "DEBUG设置") == 0){
			int i;
			for(i = 0; i < DEBUG_NUM; i++){
				configFile->debugCfg[i] = cfgValue[i] - '0';
			}
		}
		else if(strcmp(cfgKey, "DEBUG屏幕显示") == 0){
			configFile->debugShow = (char)(atoi(cfgValue));
		}
		else{
//			fprintf(stdout, "Read something strange. Oops...\n");
		}
	}
	if(!(cfgFlag & CFG_MINT_Y) || configFile->termNumMin < 3 \
		|| configFile->termNumMin > 10){
		configFile->termNumMin = TERM_NUM_MIN_DEFAULT;
	}
	if(!(cfgFlag & CFG_MAXT_Y) || configFile->termNumMax < 10 \
		|| configFile->termNumMax > 50){
		configFile->termNumMax = TERM_NUM_MAX_DEFAULT;
	}
	if(!(cfgFlag & CFG_MINV_Y) || configFile->vScrNumMin < 1 \
		|| configFile->vScrNumMin > 3){
		configFile->vScrNumMin = V_SCR_NUM_MIN_DEFAULT;
	}
	if(!(cfgFlag & CFG_MAXV_Y) || configFile->vScrNumMax < 4 \
		|| configFile->vScrNumMax > 16){
		configFile->vScrNumMax = V_SCR_NUM_MAX_DEFAULT;
	}

	fclose(fp);
	return OK;
}
