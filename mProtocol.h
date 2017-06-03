// mProtocol.h
// 1452334 Bonan Ruan
// Tue Apr 18 07:04:26 2017

/*
 * CS_ prefix means datagram from client to server
 * SC_ prefix means datagram from server to client
 * _H suffix means pHType in Header
 * _L suffix means pLType in Header
 * */

#ifndef M_PUBLIC_H
#define M_PUBLIC_H
#include "mPublic.h"
#endif

#define DEFAULT_ID	0x0000
#define HEADER_LEN	8 // length of header
#define SERVER_CLOSED	-2 // server closes the connection

#define SC_H	0x11
#define CS_H	0x91

/* general header */
struct Header{
	u_char pHType; // protocol type
	u_char pLType; // protocol sub type
	u_short len; // whole length
	u_short ID; // just the fourth domain
	u_short dataLen; // data length = len - 8
};

#define TOKEN_LEN	32
#define IDENTIFY_L	0x01

#define MAIN_VERSION_MIN	2
#define SUB_VERSION_1_MIN	0
#define SUB_VERSION_2_MIN	0

#define ETH_NUM		2
#define SYNC_NUM	2
#define ASYNC_NUM	8
#define TRANS_NUM	16
#define	USB_NUM		1
#define PRINT_NUM	1

/* 3 SC: Identify when the client firstly connects */
struct SC_Identify{
//	struct Header header; // no need
	u_short mainVersion;
	u_char subVersion1;
	u_char subVersion2;
	u_short retryTime; // retry when fail
	u_short reTransTime; // re-transport when succeed
	u_char allowEmpTer; // allow empty terminal
	u_char padding[3];
	char token[TOKEN_LEN]; // identification string
	u_int randomNum; // in Part 3 Step1
	u_int svrTime; // in Part 3 Step 2
};

/* SC: general requests */
struct SC_Request{
	struct Header header;
	// currently, no other things
};

#define MINVERREQ_L	0x00

/* 4 CS: send minimum version request */
struct CS_MinVerReq{
	struct Header header;
	u_short mainVersion;
	u_char subVersion1;
	u_char subVersion2;
};

#define DEV_GID_LEN		16
#define DEV_ID_LEN		16
#define DEV_VER_LEN		16
#define DEVID_IN_DEFAULT	1

/* 5 CS: response to identification from server */
struct CS_Identify{
	struct Header header;
	u_short CPUHz;
	u_short RAM; // unit: MB
	u_short Flash; // unit: MB
	u_short UID; // internal sequence ID
	char GID[DEV_GID_LEN]; // group sequence ID
	char TID[DEV_ID_LEN]; // type ID
	char VID[DEV_VER_LEN]; // software version ID
	u_char ethNum; // ethernet port number
	u_char syncNum; // sync port number
	u_char asnycNum; // async port number
	u_char transNum; // trasport port number
	u_char USBNum; // USB port number
	u_char printNum; // printer port number
	u_char padding1[2];
	u_int devid;
	u_char devidIn; // internal ID in one branch
	u_char padding2[3];
	char token[TOKEN_LEN];
	u_int randomNum;
};

#define SYSINFO_L	0x02

/* 6.1 CS: response system information */
struct CS_SysInfo{
	struct Header header;
	u_int uCPUTime; // user CPU time
	u_int nCPUTime; // nice CPU time
	u_int sCPUTime; // system CPU time
	u_int iCPUTime; // idle CPU time
	u_int freedMem; // freed memeory
};

#define CFGINFO_L	0x03

/* 6.2 CS: response configuration information */
struct CS_CfgInfo{
	struct Header header;
	char* info; // length is changable
};

#define PROCINFO_L	0x04

/* 6.3 CS: response Process information */
struct CS_ProcInfo{
	struct Header header;
	char* info; // length is changable
};

#define ETHINFO_L	0x05
#define ETH_0_ID	0x0000
#define ETH_1_ID	0x0001
#define MAC_LEN	6

/* 6.4 CS: response ethernet information */
struct CS_EthInfo{
	struct Header header;
	u_char existed;
	u_char configured;
	u_char upDown; // up: 0 / down: 1
	u_char padding;
	u_char MAC[MAC_LEN]; // mac address
	u_short options;
	u_int IP;
	u_int mask;
	u_int subIfaceIP1;
	u_int subIfaceMask1;
	u_int subIfaceIP2;
	u_int subIfaceMask2;
	u_int subIfaceIP3;
	u_int subIfaceMask3;
	u_int subIfaceIP4;
	u_int subIfaceMask4;
	u_int subIfaceIP5;
	u_int subIfaceMask5;
	u_int recvByte;
	u_int recvPack; // received package
	u_int recvErr;
	u_int recvDrop;
	u_int recvFifo;
	u_int recvFrame;
	u_int recvComp; // compress
	u_int recvMCast; // multicast
	u_int sendByte;
	u_int sendPack; // received package
	u_int sendErr;
	u_int sendDrop;
	u_int sendFifo;
	u_int sendFrame;
	u_int sendComp; // compress
	u_int sendMCast; // multicast
};

#define SYNCINFO_L	0x06

/* 6.5 CS: response Sync port (not used currently) */
struct CS_SyncInfo{
	struct Header header;
};

#define USBINFO_L 	0x07

/* 6.6 CS: response USB */
struct CS_USBInfo{
	struct Header header;
	u_char uDiskIn; // U-disk inserted
	u_char padding[3];
};

#define USBFILEINFO_L	0x0c

/* 6.7 CS: response USB File */
struct CS_USBFileInfo{
	struct Header header;
	char* info;
};

#define PRINTER_NAME_LEN	32
#define PRINTINFO_L			0x08

/* 6.8 CS: response printer */
struct CS_PrintInfo{
	struct Header header;
	u_char isOn; // service on
	u_char padding;
	u_short taskNum; // current task number
	char printerName[PRINTER_NAME_LEN];
};

#define PRINTQUEUEINFO_L	0x0d

/* 6.9 CS: response printer queue */
struct CS_PrintQueueInfo{
	struct Header header;
	char info;
	char padding[3];
};

/* 6.10 CS: response trans (currently not used) */
struct CS_TransInfo{
	struct Header header;
};

#define V_TERM_NUM		16
#define IP_TERM_NUM		254
#define TERMINFO_L		0x09

/* 6.11 CS: response terminal info */
struct CS_TermInfo{
	struct Header header;
	u_char vTermUsed[V_TERM_NUM];
	u_char ipTermUsed[IP_TERM_NUM];
	u_short termNum;
};

#define TERM_TYPE_LEN	12
#define TERM_STAT_LEN	8
#define SUBTERMINFO_L_1	0x0a // dumb terminal
#define SUBTERMINFO_L_2	0x0b // IP terminal

/* 6.12 CS: response v-terminal/ip-terminal config info */
struct VScreen; // declare first
struct CS_SubTermInfo{
	struct Header header;
	u_char port;
	u_char portCfg;
	u_char actVScr; // active vitrual screen
	u_char vScrNum; // virtual screen number
	u_int termIP;
	char termType[TERM_TYPE_LEN];
	char termStat[TERM_STAT_LEN];
};

#define V_PROTOCOL_LEN	12
#define V_STAT_LEN	8
#define V_PROMPT_LEN	24
#define V_TERM_TYPE_LEN	TERM_TYPE_LEN

struct VScreen{
	u_char ID; // screen
	u_char padding;
	u_short port;
	u_int IP;
	char vProtocol[V_PROTOCOL_LEN];
	char vStat[V_STAT_LEN];
	char vPrompt[V_PROMPT_LEN];
	char vTermType[V_TERM_TYPE_LEN];
	u_int connTime;
	u_int sendTByte; // send to terminal
	u_int recvTByte; // recv
	u_int sendSByte; // send to remote server
	u_int recvSByte; // recv
	u_int pingMin;
	u_int pingAvg;
	u_int pingMax;
};

#define END_L		0xff

/* 7 CS: end */
struct CS_End{
	struct Header header;
};
