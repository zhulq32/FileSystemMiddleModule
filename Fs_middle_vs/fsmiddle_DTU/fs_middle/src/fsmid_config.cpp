
#include "dbmsV1.h"

#include "fsmid_def.h"
//#include "fsmid_type.h"
#include "fsmid_log.h"
#include "fsmid_point.h"

#ifdef FAST_MODE
#define FIXPT_POINT_PER_LOG		24
#define FROZEN_POINT_PER_LOG	25
#else
#define FIXPT_POINT_PER_LOG		96
#define FROZEN_POINT_PER_LOG	97
#endif

extern unsigned int db_GetInfoAddrLen();
extern const char *db_GetTerminalID();

static int format_header_soe_co(char *buf, FSLOG* pLog)
{
	int len;
	if(buf)
		len = sprintf(buf,"%s,%s\r\n%-24s,%4d,%2d\r\n",FSLOG_GetName(pLog),FILE_VERSION,db_GetTerminalID(),FSLOG_GetUnitCount(pLog),db_GetInfoAddrLen());
	else
		len = strlen(FSLOG_GetName(pLog)) + 1 + strlen(FILE_VERSION) + 2 + 24 + 1 + 4 + 1 + 2 + 2;
	return len;
}

static int format_header_exv(char *buf, FSLOG* pLog)
{
	int len;
	SYS_TIME64 tm64;
	if(buf)
	{
		time_unix2sys(pLog->timeCreateUnix,&tm64);
		len = sprintf(buf,"%s,%s\r\n%-24s,%4d%02d%02d,%2d\r\n",FSLOG_GetName(pLog),FILE_VERSION,db_GetTerminalID(),
		tm64.year,tm64.mon,tm64.day,
		db_GetInfoAddrLen());
	}
	else
		len = strlen(FSLOG_GetName(pLog)) + 1 + strlen(FILE_VERSION) + 2 + 24 + 1 + 8 + 1 + 2 + 2;
	return len;
}

static int format_header_fixpt_frz_flowrev(char *buf, FSLOG* pLog)
{
	int len;
	SYS_TIME64 tm64;
	if(buf)
	{
		time_unix2sys(pLog->timeCreateUnix,&tm64);
		len = sprintf(buf,"%s,%s\r\n%-24s,%4d%02d%02d,%2d,%2d\r\n",FSLOG_GetName(pLog),FILE_VERSION,db_GetTerminalID(),
			tm64.year,tm64.mon,tm64.day,
			FSLOG_GetUnitCount(pLog),db_GetInfoAddrLen());
	}
	else
		len = strlen(FSLOG_GetName(pLog)) + 1 + strlen(FILE_VERSION) + 2 + 24 + 1 + 8 + 1 + 2 + 1 + 2 + 2;
	return len;
}

static int format_header_ulog(char *buf, FSLOG *pLog)
{
	int len;
	if(buf)
		len = sprintf(buf,"%-24s,%4d\r\n",db_GetTerminalID(),FSLOG_GetUnitCount(pLog));
	else
		len = 24 + 1 + 4 + 2;
	return len;
}

static int format_ulog(char *buf, const void* data)
{
	int len;
	const LOG_ULOG *log = (const LOG_ULOG *)data;

	if(buf)
	{
		len = sprintf(buf,"%2d,%4d-%02d-%02d %02d:%02d:%02d.%03d,%-128s,%d\r\n",
			log->type,
			log->time.year,log->time.mon,log->time.day,
			log->time.hour,log->time.min,log->time.sec,
			log->time.msec,
			log->sts?1:0);
	}
	else
		len = 2 + 1 + 23 + 1 + 128 + 1 + 1 + 2;
	return len;
}



static int format_soe(char *buf, const void* data)
{
	int len;
	const LOG_SOE *log = (const LOG_SOE *)data;

	if(buf)
	{
		len = sprintf(buf,"%08X,%d,%4d-%02d-%02d %02d:%02d:%02d.%03d\r\n",
			log->information,log->value%10,
			log->time.year,log->time.mon,log->time.day,
			log->time.hour,log->time.min,log->time.sec,
			log->time.msec);
	}
	else
		len = 8 + 1 + 23 + 2;
	//fsmid_assert(len <= maxlen,__FILE__,__LINE__);
	return len;
}

static int format_co(char *buf, const void* data)
{
	const char *cstrTrdOperate[] = {"分","合"};
	int len;
	const LOG_CO *log = (const LOG_CO *)data;

	if(buf)
	{
		len = sprintf(buf,"%08X,选择,%s,%4d-%02d-%02d %02d:%02d:%02d.%03d\r\n%08X,执行,%s,%4d-%02d-%02d %02d:%02d:%02d.%03d\r\n",
			log->information,cstrTrdOperate[log->value?1:0],
			log->time.year,log->time.mon,log->time.day,
			log->time.hour,log->time.min,log->time.sec,
			log->time.msec,
			log->information,cstrTrdOperate[log->value?1:0],
			log->time.year,log->time.mon,log->time.day,
			log->time.hour,log->time.min,log->time.sec,
			log->time.msec);
	}
	else
		len = (8 + 1 + 4 + 2 + 23 + 2)*2;
	//fsmid_assert(len <= maxlen,__FILE__,__LINE__);
	return len;
}

static int format_extreme(char *buf, const void* data)
{
	int len;
	FSMID_POINT * const measure = GetMeasureTable();
	const LOG_EXTREME *log = (const LOG_EXTREME *)data;

	if(log->type == 0xFFFF)//maximum
	{
		if(buf)
			len = sprintf(buf,"%3d",GetMeasureCount());
		else
			len = 3;
	}
	else if(log->type == 0xFFFE)//minimum
	{
		if(buf)
			len = sprintf(buf,"\r\n%3d",GetMeasureCount());
		else
			len = 2 + 3;
	}
	else if(log->type == 0xFFFD)//eof
	{
		if(buf)
			len = sprintf(buf,"\r\n");
		else
			len = 2;
	}
	else
	{
		if(buf)
			len = sprintf(buf,",%08X,%7.3f,%4d-%02d-%02d %02d:%02d:%02d.%03d",
				measure[log->pointIndex].information,log->value,
				log->time.year,log->time.mon,log->time.day,
				log->time.hour,log->time.min,log->time.sec,
				log->time.msec);
		else
			len = 1 + 8 + 1 + 8 + 1 +23;
	}
	//fsmid_assert(len <= maxlen,__FILE__,__LINE__);
	return len;
}

static int format_fix(char *buf, const void* data)
{
	unsigned int i,len;
	FSMID_POINT * const measure = GetMeasureTable();
	const LOG_FIXPT *log = (const LOG_FIXPT *)data;
	
	if(buf)
	{
		len = sprintf(buf,"%2d,%4d-%02d-%02d %02d:%02d:%02d.%03d",
			GetMeasureCount(),
			log->time.year,log->time.mon,log->time.day,
			log->time.hour,log->time.min,log->time.sec,
			log->time.msec);

		for( i = 0; i < GetMeasureCount(); i++ )
			len += sprintf(buf + len,",%08X,%7.3f",measure->information, log->value[i] );
		len += sprintf(buf + len,"\r\n");
	}
	else
		len = 2 + 1 + 23 + (1 + 8 + 1 + 8)*GetMeasureCount() + 2;
	//fsmid_assert(len <= maxlen,__FILE__,__LINE__);
	return len;
}

static int format_frozen(char *buf, const void* data)
{
	unsigned int i,len;
	FSMID_POINT * const frozen = GetFrozenTable();
	const LOG_FIXPT *log = (const LOG_FIXPT *)data;

	if(buf)
	{
		len = sprintf(buf,"%2d,%4d-%02d-%02d %02d:%02d:%02d.%03d",
			GetFrozenCount(),
			log->time.year,log->time.mon,log->time.day,
			log->time.hour,log->time.min,log->time.sec,
			log->time.msec);

		for( i = 0; i < GetFrozenCount(); i++ )
			len += sprintf(buf + len,",%08X,%7.3f",frozen->information, log->value[i] );
		len += sprintf(buf + len,"\r\n");
	}
	else
		len = 2 + 1 + 23 + (1 + 8 + 1 + 8)*GetMeasureCount() + 2;
	//fsmid_assert(len <= maxlen,__FILE__,__LINE__);
	return len;
}

static const SYS_TIME64 *get_log_time(const void* data)
{
	return ((const SYS_TIME64 *)data);
}


FSLOG_INTERFACE intrFslog = {
	write_flash,
	read_flash,
	erase_flash,
};

FSLOG_FUNCTION funcLogUlog = {
// 	write_flash,
// 	read_flash,
// 	erase_flash,
	format_header_ulog,
	format_ulog,
	get_log_time,
};

FSLOG_FUNCTION funcLogSoe = {
// 	write_flash,
// 	read_flash,
// 	erase_flash,
	format_header_soe_co,
	format_soe,
	get_log_time,
};

FSLOG_FUNCTION funcLogCo = {
// 	write_flash,
// 	read_flash,
	// 	erase_flash,
	format_header_soe_co,
	format_co,
	get_log_time,
};

FSLOG_FUNCTION funcLogExtreme = {
// 	write_flash,
// 	read_flash,
	// 	erase_flash,
	format_header_exv,
	format_extreme,
	get_log_time,
};

FSLOG_FUNCTION funcLogFixpt = {
// 	write_flash,
// 	read_flash,
	// 	erase_flash,
	format_header_fixpt_frz_flowrev,
	format_fix,
	get_log_time,
};

FSLOG_FUNCTION funcLogFrozen = {
// 	write_flash,
// 	read_flash,
	// 	erase_flash,
	format_header_fixpt_frz_flowrev,
	format_frozen,
	get_log_time,
};

unsigned int FSLOG_CalcBlockNumber(unsigned int unitSize, unsigned int blockSize, unsigned int unitCount, bool bOtp)
{
	unsigned int res;

	res = blockSize - 64;
	res = res/unitSize;
	if(unitCount % res)
		res = unitCount/res + 1;
	else
		res = unitCount/res;
	if(!bOtp)
		res ++;

	return res;
}

//blockNumber = unitCount/((blockSize-64)/unitSize) + 1
#define START_BLOCK_RAWSOE			16
#define START_BLOCK_PRINTLOG		36

#define START_BLOCK_ULOG			64
#define START_BLOCK_LOG_SOE			72
#define START_BLOCK_LOG_CO			80
#define START_BLOCK_LOG_EXTREME		88
// #define START_BLOCK_LOG_FIX			104
// #define START_BLOCK_LOG_FROZEN		104

FSLOG_INFORMATION infoRawSoe = {
	FLASH_BLOCK_SIZE * START_BLOCK_RAWSOE,
	17,
	FLASH_BLOCK_SIZE,
	sizeof(SOEEVENT),
	1024,
};

FSLOG_INFORMATION infoPrintLog = {
	FLASH_BLOCK_SIZE * START_BLOCK_PRINTLOG,
	22,
	FLASH_BLOCK_SIZE,
	sizeof(PRTLOGEVENT),
	1024,
};

FSLOG_INFORMATION infoLogUlog = {
	FLASH_BLOCK_SIZE * START_BLOCK_ULOG,
	20,
	FLASH_BLOCK_SIZE,
	sizeof(LOG_ULOG),
	1024,
};

FSLOG_INFORMATION infoLogSoe = {
	FLASH_BLOCK_SIZE * START_BLOCK_LOG_SOE,
	6,
	FLASH_BLOCK_SIZE,
	sizeof(LOG_SOE),
	1024,
};

FSLOG_INFORMATION infoLogCo= {
	FLASH_BLOCK_SIZE * START_BLOCK_LOG_CO,
	6,
	FLASH_BLOCK_SIZE,
	sizeof(LOG_CO),
	1024,
};


FSLOG_INFORMATION infoLogExtreme = {
	FLASH_BLOCK_SIZE * START_BLOCK_LOG_EXTREME,
	0,
	FLASH_BLOCK_SIZE,
	sizeof(LOG_EXTREME),
	0,
};

//dynamic!!!
FSLOG_INFORMATION infoLogFixpt = {
	0,//FLASH_BLOCK_SIZE * START_BLOCK_LOG_FIX,
	0,
	FLASH_BLOCK_SIZE,
	sizeof(LOG_FIXPT),
	FIXPT_POINT_PER_LOG,
};

//dynamic!!!
FSLOG_INFORMATION infoLogFrozen = {
	0,//FLASH_BLOCK_SIZE * START_BLOCK_LOG_FROZEN,
	0,
	FLASH_BLOCK_SIZE,
	sizeof(LOG_FROZRN),
	FROZEN_POINT_PER_LOG,
};

