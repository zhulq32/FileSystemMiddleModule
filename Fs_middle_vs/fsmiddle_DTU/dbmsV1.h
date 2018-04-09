
//////////////////////////////////////////////
#ifndef __DBME__
#define __DBME__

#ifndef VS_SIMULATE
#include "fsl_common.h"
#include "board.h"
#endif

#include "stdio.h"
#include "stdlib.h"
#include "commonDefEx.h"
#include "sysTimerV1.h"


////////////////////////////////////////////////


//database types
enum 
{
	DBDI = 0,
	DBAI = 1,
	DBACC = 2,
	DBDO = 3,
	DBAO = 4,
	DBJAI = 5,
};

/* flag , Point quality */
enum{
	IQ_IV = 0X80,	  // invalid
	IQ_SB = 0X20,	//Forced
};

/* queue buffer size */
#define	MAXDPAH		32
#define	MAXDCAH		32
#define MAXMSGH		32 //需要读取message的应用另外申请一个ID 

#define MAXSOE		100
#define MAXTRD		5
#define MAXAGC		1
#define MAXMSG			0	//消息条数
#define MAXINITLOG			50	//启动日志条数
#define MAXULOG		5	//遥控条数
#define MAXPRTLOG		100	//打印条数

#define MAXMONICOMM		4096//通讯监控, 最多存放的通讯码字节数

//////////////////////////////////////////////
// common used data structure

// 数据库各库大小
typedef struct 
{
	int16 di;		//遥信点数
	int16 ai;		//遥测点数
	int16 pa;		//电能累计量点数
	int16 doo;		//遥控点数，指出口数
	int16 ao;		//设点点数
	int16 jai;		//瞬时遥测点数
	int16 rev1;		
	int16 rev2;
	int16 rev3;
	int16 rev4;

}struDbSize;

//DCA HEAD
typedef struct
{
	INT16 distart;
	UINT16 dinum;
	INT16 aistart;
	UINT16 ainum;
	INT16 pastart;
	UINT16 panum;
	INT16 dostart;
	UINT16 donum;
	INT16 aostart;
	UINT16 aonum;
	INT16 jaistart;
	UINT16 jainum;
	INT16 rev0start;
	UINT16 rev0num;
	INT16 rev1Start;
	UINT16 rev1Num;
	INT16 rev2Start;
	UINT16 rev2Num;
	INT16 rev3Start;
	UINT16 rev3Num;

}struDcaHead;

//DEV SYS配置
typedef struct 
{
	struDbSize dbsize;
	char rev[32];

}struDevSysCfg;

/*************************************************************************/
/*************************************************************************/



extern struDbSize db_TotSize;


extern uint8 db_GetDpaHand(void);
extern uint8 db_GetDcaHand(void);
extern uint8 db_GetMsgHand(void);





/***************************************/
//数据库各库数据点structure, 读写函数
/***************************************/


typedef struct {
	uint8	val;// bit0:val bit1:sb_val bit2~7:IQ
} DI_PNT;
extern void db_PutDi(int16 pnt, uint8 val);
extern void db_ForceDi(int16 pnt, uint8 val);
extern uint8 db_GetDi(int16 pnt);//!返回包括IQ和val
extern void db_ClrDiFlag(int16 pnt, uint8 flag);
extern void db_SetDiFlag(int16 pnt, uint8 flag);
extern uint8 db_GetDiFlag(int16 pnt);
/***************************************/

#define COMMON_ID	0xFF  //db_GetAi(uint8 dpaH, int16 pnt)当dpaH为COMMON_ID时，返回该点数值. 用于任意应用对AI的读取
typedef struct {
	float val;
	//float realval;
	//float deadband;
	uint32 chg;
	uint8 flag;
} AI_PNT;
extern void db_PutAi(int16 pnt, float val);
//void db_PutAiDeadband(int16 pnt, float deadband);
extern void db_ForceAi(int16 pnt, float val);
extern bool db_GetAiChg(uint8 dpaH, int16 pnt);
extern void db_ResetAiChg(uint8 dpaH);
extern float db_GetAi(uint8 dpaH, int16 pnt);
extern void db_ClrAiFlag(int16 pnt, uint8 flag);
extern void db_SetAiFlag(int16 pnt, uint8 flag);
extern uint8 db_GetAiFlag(int16 pnt);
/***************************************/
//考虑到DCA可能不设置内部存放电能的变量，每次从数据库读取后累加上新量再写回数据库，
//当值被force后数值就不会再对了，故设runningval存放实际DCA PUT过来的值，
//DCA需要读回时调用db_GetRunningPa()

typedef struct {			
	float val;		//可以被FORCE
	float runningval;	//真实由DCA PUT进来的值，在unforce后copy回val
	uint8 flag;
} PA_PNT;
extern void db_PutPa(int16 pnt, float val);
extern void db_ForcePa(int16 pnt, float val);//hgy
extern float db_GetPa(int16 pnt);
extern float db_GetRunningPa(int16 pnt);
extern void db_ClrPaFlag(int16 pnt, uint8 flag);
extern void db_SetPaFlag(int16 pnt, uint8 flag);
extern uint8 db_GetPaFlag(int16 pnt);

/***************************************/

typedef struct {
	float	val;
} AO_PNT;
extern void db_PutAo(int16, float);
extern float db_GetAo(int16 pnt);
/***************************************/

typedef struct {
	float	val;
} JAI_PNT;
extern void db_PutJAi(int16 pnt, float);
extern float db_GetJAi(int16 pnt);
/***************************************/




/***************************************/

#define DB_SOEVALNUM  8
#define DB_SOEFVALNUM  8

typedef struct {
	SYS_TIME64	time;//时间，定义在sysTimer.h中
	int16	pnt;//系统点号
	uint8	val;//数值 1合 0分
	UINT8 ainum;//附带的遥测点数
	INT16 aipnt[DB_SOEFVALNUM];//附带遥测点的点号
	float aival[DB_SOEFVALNUM];//附带遥测点的值
} SOEEVENT;

typedef struct {
	int16		cnt[MAXDPAH];
	int16		rpnt[MAXDPAH];
	int16		wpnt;
	SOEEVENT	soeevent[MAXSOE];
} SOE_QUEUE;

extern void db_PutSoe(int16 pnt, uint8 val, SYS_TIME64 *time);
extern void db_PutVirSoe(int16 pnt, uint8 val);
extern void db_ForceVirSoe(int pnt, uint8 val);
extern SOEEVENT * db_GetSoe(uint8 dpaH);
extern int16	db_GetSoeCount(uint8 dpaH);
extern void db_ResetSoe(uint8 dpaH);
extern void db_PutEvSoe(SOEEVENT * evsoe);//推入保护事件
/***************************************/


typedef struct{
	int16 pnt;//系统点号
	uint8 val;//值 1合 0分
} TRDEVENT;
typedef struct {
	int16		cnt[MAXDCAH];
	int16		rpnt[MAXDCAH];
	int16		wpnt;
	TRDEVENT	trdevent[MAXTRD];
} TRD_QUEUE;
extern void db_PutTrd(int16 pnt, uint8 val);
extern TRDEVENT * db_GetTrd(uint8 dcaH);
extern int16 db_GetTrdCount(uint8 dcaH);
extern void db_ResetTrd(uint8 dcaH);
/***************************************/

typedef struct{
	int16  	pnt;
	float	val;
} AGCEVENT;
typedef struct{
	int16		cnt[MAXDCAH];
	int16		rpnt[MAXDCAH];
	int16		wpnt;
	AGCEVENT agcevent[MAXAGC];
} AGC_QUEUE;
extern void db_PutAgc(int16 pnt, float val);
extern AGCEVENT * db_GetAgc(uint8 dcaH);
extern int16 db_GetAgcCount(uint8 dcaH);
extern void db_ResetAgc(uint8 dcaH);
/***************************************/


//消息队列，应用间传递信息
typedef struct{
	uint32 msgid;//消息编号，表示什么消息
	uint32 data;//附加信息
	uint32 *retsignal;//由消息接收方填入回应
}MSGEVENT;
typedef struct{
	int16 cnt[MAXMSGH];
	int16 rpnt[MAXMSGH];
	int16 wpnt;
	MSGEVENT msgevent[MAXMSG];
}  MSG_QUEUE;
extern void db_PutMsg(uint32 msgid, uint32 data, uint32* retsignal);
extern MSGEVENT * db_GetMsg(uint8 msgH);
extern int16 db_GetMsgCount(uint8 msgH);
extern void db_ResetMsg(uint8 msgH);

/***************************************/



//应用信息，集成入哪些应用，其版本
#define LOGINITSTRLEN 64
typedef struct{
	char buf[64];
}INITLOGEVENT;
typedef struct{		
	int16 cnt[2];		//固定为：cnt[0]dpaZzmap,cnt[1]FileMid
	int16 rpnt[2];
	int16 wpnt;
	INITLOGEVENT logevent[MAXINITLOG];
}  INITLOG_QUEUE;
extern void db_ResetInitLog(uint8 who);
extern void db_PutInitLog(char *buf);
extern INITLOGEVENT* db_GetInitLog(uint8 who);
extern int16 db_GetInitLogCount(uint8 who);

/******************************************************/
//事件日志，指下载配置、启动、异常等信息
#define ULOGSTRLEN 64
typedef struct{
	SYS_TIME64 time;
	uint8 type;//信息类型编号
	uint8 sts;//信息值
	char buf[64];
}ULOGEVENT;
typedef struct{		
	int16 cnt[2];		//固定为：cnt[0]dpaZzmap,cnt[1]FileMid
	int16 rpnt[2];
	int16 wpnt;
	ULOGEVENT logevent[MAXULOG];
}  ULOG_QUEUE;
extern void db_ResetULog(uint8 who);
extern void db_PutULog(uint8 type, uint8 sts, char *buf);
extern ULOGEVENT * db_GetULog(uint8 who);
extern int16 db_GetULogCount(uint8 who);
/******************************************************/
//打印信息，主要为程序的调试诊断提示信息
#define LOGPRINTSTRLEN 72
typedef struct{
	SYS_TIME64 time;
	char buf[72];
}PRTLOGEVENT;
typedef struct{		
	int16 cnt[2];		//固定为：cnt[0]dpaZzmap,cnt[1]FileMid
	int16 rpnt[2];
	int16 wpnt;
	PRTLOGEVENT logevent[MAXPRTLOG];
}  PRTLOG_QUEUE;
extern void db_ResetPrtLog(uint8 who);
extern void db_PutPrtLog(char *buf);
extern PRTLOGEVENT * db_GetPrtLog(uint8 who);
extern int16 db_GetPrtLogCount(uint8 who);
/******************************************************/



/*****************************************************
应用向维护口发送通讯报文等，这些函数放在DBMS，使维护口应用可以不link到程序中
*****************************/

#define MONICOMM_DIR_TX	0X40
#define MONICOMM_DIR_RX  0X41
#define MONICOMM_DIR_NONE 0X42

typedef struct{
	uint8 MonCom;//监视的串口，指portnr, 0为无效口
	uint32 RemoteIP;//监视的对方IP,当监视网络通讯时有效
	uint32 LocalIP;//监视的本地IP,当监视网络通讯时有效
	int16 rpnt;
	int16 wpnt;
	char buf[MAXMONICOMM];
}  MONCOMM_QUEUE;
extern void glb_ResetMoniComm(void);
extern void glb_PutComLog(uint8 portnr, char *buf, uint8 dir);//最多79个字符
extern void glb_PutComCode(uint8 portnr, uint8 *buf, uint16 len, uint8 dir);
extern void glb_PutNetLog(uint32 LocalIP, uint32 remotIP, char *buf, uint8 dir);//最多79个字符
extern void glb_PutNetCode(uint32 LocalIP, uint32 remotIP, uint8 *buf, uint16 len, uint8 dir);
extern bool glb_GetMoniCommCode(uint8 *code);
extern int16 db_GetMoniCommCount(void);
/******************************************************/


/*******************************************************/
extern void db_unForce(void);
extern bool db_Init(void);
/*******************************************************/




/*******************************************************/

extern uint8 glb_f_ApplsRunEn;//允许APP运行标志，当没有配置、下载配置，下载程序时glb_ApplsEn清0
extern uint8* glb_pSfgFile;   //指向配置文件的首地址，同时作为有无正确配置的标识，NULL表示无


#define MEMCTRLCBK_IDT	0X123456
typedef struct
{
	uint32 identify;
	uint32 size;
	uint32 systick;//时间作为随机数参与CHKSUM
	uint16 dummy1;
	uint16 chksum;//对PRG/CFG区算CRC
} struPrgCfgCbk;//程序、配置下载状态控制块

typedef struct
{
	uint32 identify;
	uint8 isMacSet;
	uint8 isIpSet;
	uint8 isSnSet;
	uint8 isSzParaSet;
	uint8 isDzParaSet;
	uint8 isTtParaSet;
	uint8 isFixParaSet;
	uint8 dummy1;
	uint16 dummy2;
	uint16 chksum;
} struParaCbk;//参数设置状态控制块


typedef struct
{
	uint32 IpAddr;
	uint32 Mask;
	uint32 Gateway;
} struIp;


extern int8 glb_cfgCurr;
extern struParaCbk glb_ParaCbk;

extern struIp glb_DefaultLocalIp[];


extern bool glb_RebootFlag;
extern uint32 glb_RebootTick;

/**************************************/
extern void glb_Init(void);
extern uint8 * glb_FindAppCfg(uint32 appidver);//ver:A.B.C  appidver: bit76:A, bit54:B, bit3210:appid
extern struIp* glb_getLocalIp(uint8 n);


extern float glb_GetDz(int16 pnt);//定值
extern float glb_GetSz(int16 pnt);//运行参数
extern bool glb_GetTt(int16 pnt);//投退状态
extern uint8 glb_GetLed(uint8 pnt);//LED灯状态，bit0:1亮0灭 bit67:0常灭1常亮2慢闪3快闪
extern void glb_PutLed(uint8 pnt, uint8 val);



/******************************************************/
#endif
