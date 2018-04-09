
#include "fsmid_def.h"
#include "fsmid_port.h"
#include "fsmid_point.h"
#include "fsmid_config.h"
#include "fsmid_log.h"

#include "dbmsV1.h"
#include "dpa10x.h"

#undef  _FSLOG_INFO_MSG_
//#define _FSLOG_INFO_MSG_
#if defined(_FSLOG_INFO_MSG_)
#define FSLOG_INFO_MSG(fmt,...)       fsmid_info(fmt, ##__VA_ARGS__)
#else
#define FSLOG_INFO_MSG(fmt,...)
#endif  //#if defined(_FSLOG_INFO_MSG_)

#ifdef FAST_MODE
#define FIFTEEN_MINUTE_CONDITION(tm64)		(!(tm64.sec%5))
#define DAILY_CONDITION(tm64)				(!((tm64.hour*60+tm64.min)%2) && !tm64.sec)
#else
#define FIFTEEN_MINUTE_CONDITION(tm64)		(!(tm64.min%15) && !tm64.sec)
#define DAILY_CONDITION(tm64)				(!tm64.hour && !tm64.min)
#endif

static uint8 handleDpa;
static uint8 handleDca;
static uint8 handleWho;

//*no for 101
FSLOG *logRawSoe;
FSLOG *logPrintLog;

//*1
FSLOG *logUlog;
FSLOG *logSoe;
FSLOG *logCo;

#define TEMP_EXV_NAME		"TEMP\\EXV"
#define TEMP_FIXPT_NAME		"TEMP\\FIXPT"
#define TEMP_FRZ_NAME		"TEMP\\FRZ"

#ifdef FAST_MODE
#define NUMBER_OF_EXV		5
#define NUMBER_OF_FIXPT		5
#define NUMBER_OF_FRZ		5
#else
#define NUMBER_OF_EXV		30
#define NUMBER_OF_FIXPT		31
#define NUMBER_OF_FRZ		31
#endif

FSLOG *logExtremeTable[NUMBER_OF_EXV];
unsigned int nExtreme;
FSLOG **logExtreme = NULL;

FSLOG *logFixptTable[NUMBER_OF_FIXPT];
unsigned int nFixpt;
FSLOG **logFixpt = NULL;

FSLOG *logFrozenTable[NUMBER_OF_FRZ];
unsigned int nFrozen;
FSLOG **logFrozen = NULL;


#define DYNAMIC_START_BLOCK		88

void FSMID_FormatLogName(FSLOG *pLog, const char* nameLowCase, const SYS_TIME64 *tm64)
{
	int i = 0;
	char nameUpCase[16];
	while(nameLowCase[i])
	{
		if(nameLowCase[i]>='a'&&nameLowCase[i]<='z')
			nameUpCase[i] = nameLowCase[i] - 'a' + 'A';
		i++;
	}
	nameUpCase[i] = 0;
	FSLOG_INFO_MSG("[FSRENAME] from:\"%s\" to ",pLog->name);
#ifdef FAST_MODE
	sprintf(pLog->name,"HISTORY\\%s\\%s%02d%02d00.msg",nameUpCase,nameLowCase,tm64->hour,tm64->min);
#else
	sprintf(pLog->name,"HISTORY\\%s\\%s20%02d%02d%02d.msg",nameUpCase,nameLowCase,tm64->year,tm64->mon,tm64->day);
#endif
	FSLOG_INFO_MSG("%s\".\n",pLog->name);
}

void FSMID_CreateLogs(const SYS_TIME64 *tm64)
{
	unsigned int i, address = DYNAMIC_START_BLOCK*FLASH_BLOCK_SIZE;
	FSLOG_INFORMATION *pInfo;
	FSLOG **ppLog;
	SYS_TIME64 sysTime;

	logRawSoe   = FSLOG_Open("SYSTEM\\RAW_SOE",		NULL,			&infoRawSoe,	FSLOG_ATTR_OPEN_EXIST);
	fsmid_assert(logRawSoe,__FILE__,__LINE__);
	logPrintLog = FSLOG_Open("SYSTEM\\PRINTLOG",	NULL,			&infoPrintLog,	FSLOG_ATTR_OPEN_EXIST);
	fsmid_assert(logPrintLog,__FILE__,__LINE__);

	logUlog		= FSLOG_Open("HISTORY\\ULOG\\ulog.msg",		&funcLogUlog,	&infoLogUlog,	FSLOG_ATTR_OPEN_EXIST);
	fsmid_assert(logUlog,__FILE__,__LINE__);
	logSoe		= FSLOG_Open("HISTORY\\SOE\\soe.msg",		&funcLogSoe,	&infoLogSoe,	FSLOG_ATTR_OPEN_EXIST);
	fsmid_assert(logSoe,__FILE__,__LINE__);
	logCo		= FSLOG_Open("HISTORY\\CO\\co.msg",			&funcLogCo,		&infoLogCo,		FSLOG_ATTR_OPEN_EXIST);
	fsmid_assert(logCo,__FILE__,__LINE__);

	for(i = 0, nExtreme = 0, ppLog = logExtremeTable; i < NUMBER_OF_EXV; i++,ppLog++)
	{
		pInfo = fsmid_malloc(FSLOG_INFORMATION,1);
		memcpy(pInfo,&infoLogExtreme,sizeof(FSLOG_INFORMATION));
		pInfo->baseAddress = address;
		pInfo->unitCount = GetMeasureCount()*2+3;
		pInfo->blockNumber = FSLOG_CalcBlockNumber(pInfo->unitSize,pInfo->blockSize,pInfo->unitCount,true);
		fsmid_assert(address + pInfo->blockNumber * pInfo->blockSize < FLASH_MEMORY_SIZE,__FILE__,__LINE__);
		logExtremeTable[i] = FSLOG_Open(TEMP_EXV_NAME,&funcLogExtreme,pInfo,FSLOG_ATTR_OPEN_EXIST|FSLOG_ATTR_OTP);
		fsmid_assert(*ppLog,__FILE__,__LINE__);
		address += pInfo->blockNumber * pInfo->blockSize;
		if((*ppLog)->timeCreateUnix)
		{
			time_unix2sys((*ppLog)->timeCreateUnix,&sysTime);
			FSMID_FormatLogName(*ppLog,"exv",&sysTime);
			nExtreme++;
		}
		else if(!logExtreme)
			logExtreme = ppLog;
	}
	if(!logExtreme)
	{
		logExtreme = logExtremeTable;
		for(i = 1, ppLog = logExtremeTable + 1; i < NUMBER_OF_EXV; i++,ppLog++)
		{
			if(strcmp((*logExtreme)->name,(*ppLog)->name) < 0)
				logExtreme = ppLog;
		}
	}

	for(i = 0, nFixpt = 0, ppLog = logFixptTable; i < NUMBER_OF_FIXPT; i++,ppLog++)
	{
		pInfo = fsmid_malloc(FSLOG_INFORMATION,1);
		memcpy(pInfo,&infoLogFixpt,sizeof(FSLOG_INFORMATION));
		pInfo->baseAddress = address;
		//pInfo->unitCount = 96;
		pInfo->blockNumber = FSLOG_CalcBlockNumber(pInfo->unitSize,pInfo->blockSize,pInfo->unitCount,true);
		fsmid_assert(address + pInfo->blockNumber * pInfo->blockSize < FLASH_MEMORY_SIZE,__FILE__,__LINE__);
		logFixptTable[i] = FSLOG_Open(TEMP_FIXPT_NAME,&funcLogFixpt,pInfo,FSLOG_ATTR_OPEN_EXIST|FSLOG_ATTR_OTP);
		fsmid_assert(ppLog,__FILE__,__LINE__);
		address += pInfo->blockNumber * pInfo->blockSize;
		if((*ppLog)->timeCreateUnix)
		{
			time_unix2sys((*ppLog)->timeCreateUnix,&sysTime);
			FSMID_FormatLogName(*ppLog,"fixpt",&sysTime);
			nFixpt++;
			//select current day file
			if(systimeSameDay(&sysTime,tm64))
			{
				fsmid_assert(!logFixpt,__FILE__,__LINE__);
				logFixpt = ppLog;
			}
		}
		else if(!logFixpt)
			logFixpt = ppLog;
	}
	if(logFixpt)
	{
		if(strcmp((*logFixpt)->name,TEMP_FIXPT_NAME)==0)
		{
			FSMID_FormatLogName(*logFixpt,"fixpt",tm64);
			(*logFixpt)->timeCreateUnix = time_sys2unix(tm64);
			nFixpt++;
		}
	}
	else
	{
		logFixpt = logFixptTable;
		for(i = 1, ppLog = logFixptTable + 1; i < NUMBER_OF_FRZ; i++,ppLog++)
		{
			if(strcmp((*logFixpt)->name,(*ppLog)->name) < 0)
				logFixpt = ppLog;
		}
	}

	for(i = 0, nFrozen = 0, ppLog = logFrozenTable; i < NUMBER_OF_FRZ; i++,ppLog++)
	{
		pInfo = fsmid_malloc(FSLOG_INFORMATION,1);
		memcpy(pInfo,&infoLogFrozen,sizeof(FSLOG_INFORMATION));
		pInfo->baseAddress = address;
		//pInfo->unitCount = 97;
		pInfo->blockNumber = FSLOG_CalcBlockNumber(pInfo->unitSize,pInfo->blockSize,pInfo->unitCount,true);
		fsmid_assert(address + pInfo->blockNumber * pInfo->blockSize < FLASH_MEMORY_SIZE,__FILE__,__LINE__);
		logFrozenTable[i] = FSLOG_Open(TEMP_FRZ_NAME,&funcLogFrozen,pInfo,FSLOG_ATTR_OPEN_EXIST|FSLOG_ATTR_OTP);
		fsmid_assert(ppLog,__FILE__,__LINE__);
		address += pInfo->blockNumber * pInfo->blockSize;
		if((*ppLog)->timeCreateUnix)
		{
			time_unix2sys((*ppLog)->timeCreateUnix,&sysTime);
			FSMID_FormatLogName(*ppLog,"frz",&sysTime);
			nFrozen++;
			//select current day file
			if(systimeSameDay(&sysTime,tm64))
			{
				fsmid_assert(!logFrozen,__FILE__,__LINE__);
				logFrozen = ppLog;
			}
		}
		else if(!logFrozen)
			logFrozen = ppLog;
	}
	if(logFrozen)
	{
		if(strcmp((*logFrozen)->name,TEMP_FRZ_NAME)==0)
		{
			FSMID_FormatLogName(*logFrozen,"frz",tm64);
			(*logFrozen)->timeCreateUnix = time_sys2unix(tm64);
			nFrozen++;
		}
	}
	else
	{
		logFrozen = logFrozenTable;
		for(i = 1, ppLog = logFrozenTable + 1; i < NUMBER_OF_FRZ; i++,ppLog++)
		{
			if(strcmp((*logFrozen)->name,(*ppLog)->name) < 0)
				logFrozen = ppLog;
		}
	}
}

// static void write_soe(SYS_TIME64 *pTime, unsigned int inf, unsigned char value)
// {
// 	LOG_SOE soe;
// 	memcpy(&soe.time,pTime,sizeof(soe.time));
// 	soe.information = inf;
// 	soe.value = value;
// 	FSLOG_LockWrite(logSoe,&soe);
// }

static void FSMID_SoeApp(const SYS_TIME64 *t64)
{
//	int i, j;
	struDpa10xFrm *pfrm;
	void *ppntcfg = NULL;
// 	struDpa10xIt_Cfg *pItCfg;
// 	struDpa10xMe_Cfg *pMeCfg;
	struDpa10xDp_Cfg *pDpCfg;
	struDpa10xSp_Cfg *pSpCfg;

	SOEEVENT *pEvent;

	uint32 inf;
	int8 frm;
	int16 pnt,otherpnt;

	/*static*/ LOG_SOE soe = {0};
	/*static*/ int16 s_otherpnt = 0;
	/*static*/ bool flag=0;//表示有临时soe未保存

	pfrm = dpa101appl.pport->pfrm;//指向第1帧
	while (pEvent = db_GetSoe(handleDpa))
	{
		//1.save soeevent, 单纯把event记录下来，给维护口读取，维护口将按记录读取，与文件无关
		//包括soe trd prtlog
		FSLOG_LockWrite(logRawSoe,pEvent);


		//2.save soe for 101
		//包括soe trd ulog, 仅soe有双点处理, ulog不需要search到inf,直接保存
		//这里仅以soe为例，以soe1为临时变量(struSoeLog soe1)
		ppntcfg = dpa10x_SearchSyspntInFrms(dpa101appl.pport, pEvent->pnt, TypeidDi, &frm, &pnt, &otherpnt, &inf);//查找系统点对应的点配置等，并得到inf

		if (ppntcfg == NULL)//没找到，说明点不在101配置中
		{
			if(flag)
			{
				flag = 0;
				FSLOG_LockWrite(logSoe,&soe);
			}
			continue;
		}


		if (pfrm[frm].pcfg->frmtype == M_DP_TA_1 || pfrm[frm].pcfg->frmtype == M_DP_TB_1)	//找到在双点帧里
		{	
			pDpCfg = (struDpa10xDp_Cfg *)ppntcfg;//指向点配置

			if(flag == 1)//有临时变量中未存SOE
			{
				if(pEvent->pnt == s_otherpnt)//找到第2条soe，丢弃上次soe，保存本次soe
				{
					flag = 0;
					memcpy(&soe.time,&pEvent->time,sizeof(soe.time));
					soe.information = inf;
					soe.value = ((db_GetDi(pDpCfg->syspnton)&1)<<1) | (db_GetDi(pDpCfg->syspntoff)&1);
					FSLOG_LockWrite(logSoe,&soe);
				}
				else //不是同一个双点双点遥信点，则保存临时变量soe，然后把本soe推入临时变量
				{
					FSLOG_LockWrite(logSoe,&soe);

					flag = 1;
					s_otherpnt = otherpnt;
					memcpy(&soe.time,&pEvent->time,sizeof(soe.time));
					soe.information = inf;
					soe.value = ((db_GetDi(pDpCfg->syspnton)&1)<<1) | (db_GetDi(pDpCfg->syspntoff)&1);
				}
			}
			else //没有未保存临时变量，把本soe推入临时变量soe1中
			{		
				flag = 1;
				s_otherpnt = otherpnt;
				memcpy(&soe.time,&pEvent->time,sizeof(soe.time));
				soe.information = inf;
				soe.value = ((db_GetDi(pDpCfg->syspnton)&1)<<1) | (db_GetDi(pDpCfg->syspntoff)&1);
			}
		}
		else //单点帧的点，直接保存log
		{
			if(flag)
				FSLOG_LockWrite(logSoe,&soe);

			pSpCfg = (struDpa10xSp_Cfg *)ppntcfg;
			flag = 0;
			memcpy(&soe.time,&pEvent->time,sizeof(soe.time));
			soe.information = inf;
			soe.value = (db_GetDi(pSpCfg->syspnt)&1)<<1;//把单点SIQ做成双点DIQ
			FSLOG_LockWrite(logSoe,&soe);
		}
	}

	if(flag)
		FSLOG_LockWrite(logSoe,&soe);
}

static void FSMID_CoApp(const SYS_TIME64 *t64)
{
	struDpa10xFrm *pfrm;
	void *ppntcfg = NULL;

	TRDEVENT *pEvent;

	uint32 inf;
	int8 frm;
	int16 pnt,otherpnt;

	LOG_CO co = {0};

	pfrm = dpa101appl.pport->pfrm;//指向第1帧
	while (pEvent = db_GetTrd(handleDca))
	{
		ppntcfg = dpa10x_SearchSyspntInFrms(dpa101appl.pport, pEvent->pnt, TypeidDo, &frm, &pnt, &otherpnt, &inf);//查找系统点对应的点配置等，并得到inf

		memcpy(&co.time,t64,sizeof(co.time));
		co.information = inf;
		co.value = pEvent->val;
		FSLOG_LockWrite(logCo,&co);
	}
}

static void FSMID_LogApp(const SYS_TIME64 *t64)
{
	PRTLOGEVENT *prtEvent;
	ULOGEVENT *ulogEvent;

	while(ulogEvent = db_GetULog(handleWho))
		FSLOG_LockWrite(logUlog,ulogEvent);

	while(prtEvent = db_GetPrtLog(handleWho))
		FSLOG_LockWrite(logPrintLog,prtEvent);
}

static void FSMID_ExtremeApp(const SYS_TIME64 *t64)
{
	unsigned int i;
	FSMID_POINT *point;

	for( i = 0, point = GetMeasureTable(); i < GetMeasureCount(); i++,point++ )
	{
		UpdateExtremeValue(t64, i, db_GetAi(handleDpa,point->point));
	}
}

static void FSMID_FixptApp(const SYS_TIME64 *t64)
{
	unsigned int i;
	FSMID_POINT *point;
	LOG_FIXPT *pFixpt = (LOG_FIXPT*)fsmid_malloc(unsigned char,sizeof(LOG_FIXPT) + GetMeasureCount()*sizeof(float));

	memcpy(&pFixpt->time,t64,sizeof(pFixpt->time));
	pFixpt->numUnit = GetMeasureCount();
	for( i = 0, point = GetMeasureTable(); i < GetMeasureCount(); i++,point++ )
	{
		pFixpt->value[i] = db_GetAi(handleDpa,point->point);
	}
	FSLOG_LockWrite(*logFixpt,pFixpt);
	fsmid_free(pFixpt);
}

static void FSMID_FrozenApp(const SYS_TIME64 *t64)
{
	unsigned int i;
	FSMID_POINT *point;
	LOG_FIXPT *pFrozen = (LOG_FIXPT*)fsmid_malloc(unsigned char,sizeof(LOG_FIXPT) + GetMeasureCount()*sizeof(float));

	memcpy(&pFrozen->time,t64,sizeof(pFrozen->time));
	pFrozen->numUnit = GetFrozenCount();
	for( i = 0, point = GetFrozenTable(); i < GetFrozenCount(); i++,point++ )
	{
		pFrozen->value[i] = db_GetPa(point->point);
	}
	FSLOG_LockWrite(*logFrozen,pFrozen);
	fsmid_free(pFrozen);
}

static void FSMID_SaveExtremeLog(const SYS_TIME64 *tm64)
{
	int i;
	LOG_EXTREME exv = {0};
	LOG_EXTREME *pExtreme = GetMaximumTable();

	if((*logExtreme)->unitNumber)
	{
		FSLOG_Clear(*logExtreme);
	}
	(*logExtreme)->timeCreateUnix = time_sys2unix(&pExtreme->time);
	FSMID_FormatLogName(*logExtreme,"exv",&pExtreme->time);
	
	FSLOG_Lock(*logExtreme);
// 	exv.time.year = pExtreme->time.year;
// 	exv.time.mon = pExtreme->time.mon;
// 	exv.time.day = pExtreme->time.day;
// 	exv.time.hour = pExtreme->time.hour;
// 	exv.time.min = pExtreme->time.min;
// 	exv.time.sec = pExtreme->time.sec;
	memcpy(&exv.time,&pExtreme->time,sizeof(exv.time));
	exv.type = EXTREME_MAX_MARK;
	FSLOG_Write(*logExtreme,&exv);

	for( i = 0, pExtreme = GetMaximumTable(); i < GetMeasureCount(); i++, pExtreme++ )
	{
		FSLOG_Write(*logExtreme,pExtreme);
	}

	exv.type = EXTREME_MIN_MARK;
	FSLOG_Write(*logExtreme,&exv);

	for( i = 0, pExtreme = GetMinimumTable(); i < GetMeasureCount(); i++, pExtreme++ )
	{
		FSLOG_Write(*logExtreme,pExtreme);
	}

	exv.type = EXTREME_EOF_MARK;
	FSLOG_Write(*logExtreme,&exv);

	FSLOG_Unlock(*logExtreme);
	fsmid_assert((*logExtreme)->unitNumber == 3+GetMeasureCount()*2,__FILE__,__LINE__);

	logExtreme ++;
	if(nExtreme < NUMBER_OF_EXV)
		nExtreme++;
	if(logExtreme >= logExtremeTable + NUMBER_OF_EXV)
		logExtreme = logExtremeTable;

}

static void FSMID_SaveFixptLog(const SYS_TIME64 *tm64)
{
	logFixpt ++;
	if(nFixpt < NUMBER_OF_FIXPT)
		nFixpt++;
	if(logFixpt >= logFixptTable + NUMBER_OF_FIXPT)
		logFixpt = logFixptTable;

// 	if((*logFixpt)->unitNumber != 96)
// 		fsmid_warning("!FIXPT NUMBER",__FILE__,__LINE__);

	if((*logFixpt)->unitNumber)
		FSLOG_Clear(*logFixpt);
	(*logFixpt)->timeCreateUnix = time_sys2unix(tm64);
	FSMID_FormatLogName(*logFixpt,"fixpt",tm64);
	ResetExtremeTable();
}

static void FSMID_SaveFrozenLog(const SYS_TIME64 *tm64)
{

	//save current day's first point as daily frozen.
//	LOG_FROZRN frozen;
// 	FSLOG_Read(*logFrozen,0,&frozen);
// 	FSLOG_LockWrite(*logFrozen,&frozen);

	//frozen new day's first log as daily log
	FSMID_FrozenApp(tm64);//FROZEN

// 	if((*logFrozen)->unitNumber != 97)
// 		fsmid_warning("!FROZEN NUMBER",__FILE__,__LINE__);

	logFrozen ++;
	if(nFrozen < NUMBER_OF_FRZ)
		nFrozen++;
	if(logFrozen >= logFrozenTable + NUMBER_OF_FRZ)
		logFrozen = logFrozenTable;

	if((*logFrozen)->unitNumber)
		FSLOG_Clear(*logFrozen);
	(*logFrozen)->timeCreateUnix = time_sys2unix(tm64);
	FSMID_FormatLogName(*logFrozen,"frz",tm64);
}

static void one_sec_delay(SYS_TIME64 *tm64)
{
	SYS_TIME64 _tm64;
	do{
		fsmid_delayMs(300);
		glb_GetDateTime(&_tm64);
	}while(_tm64.sec == tm64->sec);
}

void FSMID_Task(void*)
{
	SYS_TIME64 tm64;

	glb_GetDateTime(&tm64);
	handleDpa = db_GetDpaHand();
	handleDca = db_GetDcaHand();
	handleWho = 1;

	FSLOG_Init(&intrFslog);
	FSMID_InitConfig();
	FSMID_CreateLogs(&tm64);
	ResetExtremeTable();

	FSLOG_INFO_MSG("============================== APP START==============================\n");
	while(1)
	{
		one_sec_delay(&tm64);
		glb_GetDateTime(&tm64);

		if(FIFTEEN_MINUTE_CONDITION(tm64))
		{
			FSLOG_INFO_MSG("[TIME] Tick:20%02d-%02d-%02d %02d:%02d:%02d %03d\n",tm64.year,tm64.mon,tm64.day,tm64.hour,tm64.min,tm64.sec,tm64.msec);
			if(DAILY_CONDITION(tm64))
			{
				FSMID_SaveExtremeLog(&tm64);
				FSMID_SaveFixptLog(&tm64);
				FSMID_SaveFrozenLog(&tm64);
			}
			FSMID_FixptApp(&tm64);//FIXPT
			FSMID_FrozenApp(&tm64);//FROZEN
		}
		FSMID_SoeApp(&tm64);//SOE and RawSoe
		FSMID_CoApp(&tm64);//CO
		FSMID_LogApp(&tm64);//ULOG and PrintLog
		FSMID_ExtremeApp(&tm64);//EXV
	}
}
