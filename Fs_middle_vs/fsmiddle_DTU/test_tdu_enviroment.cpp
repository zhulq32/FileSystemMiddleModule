#include "dbmsV1.h"
#include "dpa10x.h"
#include "fsmid_port.h"

#include <windows.h>
#include <time.h>
#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <direct.h>

#define TEST_UNIT_NUMBER		128//1024
#define TEST_UNIT_SIZE			128//16
#define TEST_BLOCK_SIZE			4096

#define SIM_FLASH_SIZE			(16<<20)
#define FLASH_FILE_NAME			"simDTU_FLASH.bin"
extern FILE* sim_flash_file;

#define NUM_TEST_MEASURE		10
#define NUM_TEST_FROZEN			5

#define START_INF				100
#define START_SYSPNT			10

float test_measure[NUM_TEST_MEASURE];
float test_frozen[NUM_TEST_FROZEN];

ULOGEVENT *test_ulog = NULL;
PRTLOGEVENT *test_prtlog = NULL;
SOEEVENT *test_soe = NULL;
TRDEVENT *test_trd = NULL;

HANDLE test_lock;


struDpa10xApp dpa101appl = {0};

unsigned int heapUsage = 0;
unsigned int maxHeapUsage = 0;

void *test_malloc(unsigned int size)
{
	unsigned char *p = new unsigned char[size + 4];
	*(unsigned int*)p = size;
	heapUsage += size;
	if(heapUsage > maxHeapUsage)
		maxHeapUsage = heapUsage;
	return (p + 4);
}

void test_free(void *pBuf)
{
	unsigned char *p = (unsigned char *)pBuf;
	p -= 4;
	heapUsage -= *(unsigned int*)p;
	delete [] p;
}

unsigned int db_GetInfoAddrLen()
{
	return 3;
}

const char *db_GetTerminalID()
{
	return "DTU_DEVICE_12345678";
}

uint8 db_GetDcaHand()
{
	return 1;
}

uint8 db_GetDpaHand()
{
	return 1;
}

void glb_GetDateTime( SYS_TIME64 *tm64)
{
	SYSTEMTIME sys; 
	GetLocalTime( &sys );
	
	tm64->year = sys.wYear - 2000;
	tm64->mon = sys.wMonth;
	tm64->day = sys.wDay;
	tm64->hour = sys.wHour;
	tm64->min = sys.wMinute;
	tm64->sec = sys.wSecond;
	tm64->msec = sys.wMilliseconds;
}

uint8 db_GetDi( int16 pnt)
{
	return 0;
}

SOEEVENT *db_GetSoe(uint8 hand)
{
	SOEEVENT *event;
	WaitForSingleObject(test_lock,INFINITE);
	event = test_soe;
	if(test_soe)
		delete test_soe;
	test_soe = NULL;
	ReleaseMutex(test_lock);
	return event;
}

TRDEVENT *db_GetTrd(uint8 hand)
{
	TRDEVENT *event;
	WaitForSingleObject(test_lock,INFINITE);
	event = test_trd;
	if(test_trd)
		delete test_trd;
	test_trd = NULL;
	ReleaseMutex(test_lock);
	return event;
}

PRTLOGEVENT *db_GetPrtLog(uint8 who)
{
	PRTLOGEVENT *event;
	WaitForSingleObject(test_lock,INFINITE);
	event = test_prtlog;
	if(test_prtlog)
		delete test_prtlog;
	test_prtlog = NULL;
	ReleaseMutex(test_lock);
	return event;
}

ULOGEVENT *db_GetULog(uint8 who)
{
	ULOGEVENT *event;
	WaitForSingleObject(test_lock,INFINITE);
	event = test_ulog;
	if(test_ulog)
		delete test_ulog;
	test_ulog = NULL;
	ReleaseMutex(test_lock);
	return event;
}

float db_GetAi(uint8 dpaH, int16 pnt)
{
	float res;
	fsmid_assert(pnt>=START_SYSPNT && pnt < START_SYSPNT + NUM_TEST_MEASURE,__FILE__,__LINE__);

	WaitForSingleObject(test_lock,INFINITE);
	res = test_measure[pnt-START_SYSPNT];
	ReleaseMutex(test_lock);
	return res;
}

float db_GetPa(int16 pnt)
{
	float res;
	fsmid_assert(pnt>=START_SYSPNT + NUM_TEST_MEASURE && pnt < START_SYSPNT + NUM_TEST_MEASURE + NUM_TEST_FROZEN,__FILE__,__LINE__);

	WaitForSingleObject(test_lock,INFINITE);
	res = test_frozen[pnt-START_SYSPNT-NUM_TEST_MEASURE];
	ReleaseMutex(test_lock);
	return res;
}

void* dpa10x_SearchSyspntInFrms(struDpa10xPort *pport, int16 syspnt, enumTYPID_GRP typgrp, int8 *frm, int16 *pnt, int16 *otherpnt, uint32 *inf)
{
	*frm = 0;
	*pnt = syspnt;
	*otherpnt = 0;
	*inf = syspnt * 10;
	return NULL;
}


int write_flash(unsigned int address, const void* data, unsigned int length);
int read_flash(unsigned int address, void* data, unsigned int length);
int erase_flash(unsigned int address, unsigned int length);

bool init_dtu_enviroment(char *pPath)
{
	fpos_t pos;
	int offset;
	int i;
	uint32 inf = START_INF;
	int16 syspnt = START_SYSPNT;
	TCHAR thePath[MAX_PATH];

	memset(thePath,0,sizeof(thePath));
	sprintf(thePath,"%s\\%s",pPath,FLASH_FILE_NAME);
	printf("Try open flash file: %s\n",thePath);
	sim_flash_file = fopen(thePath,"rb+");
	if(sim_flash_file == NULL)
	{
		sim_flash_file = fopen(thePath,"wb+");
		if(sim_flash_file == NULL)
		{
			printf("Fail to create file \"%s\".\n",thePath);
			return false;
		}
	}
	offset = fseek(sim_flash_file,0,SEEK_END);
	fgetpos(sim_flash_file,&pos);
	if(pos == 0)
	{
		printf("Create new file. erase all.\n");
		erase_flash(0,SIM_FLASH_SIZE);
		printf("Init file done.\n\n");
	}
	fseek(sim_flash_file,0,SEEK_SET);

	// 	memset(thePath,0,sizeof(thePath));
	// 	sprintf(thePath,"%s\\%s",pPath,TEST_DATA_FNAME);
	// 	printf("Try open flash file: %s\n",thePath);
	// 	test_data_file = fopen(thePath,"rb");
	// 	if(sim_flash_file == NULL)
	// 	{
	// 		printf("Fail to open file \"%s\".\n",thePath);
	// 		return false;
	// 	}
	dpa101appl.pport = new struDpa10xPort;
	ZeroMemory(dpa101appl.pport,sizeof(struDpa10xPort));
	dpa101appl.pport->frmnum = NUM_TEST_FROZEN + NUM_TEST_MEASURE;
	dpa101appl.pport->pfrm = new struDpa10xFrm[NUM_TEST_FROZEN + NUM_TEST_MEASURE];
	ZeroMemory(dpa101appl.pport->pfrm,sizeof(struDpa10xFrm)*(NUM_TEST_FROZEN + NUM_TEST_MEASURE));
	for(i = 0; i < NUM_TEST_MEASURE; i++ )
	{
		dpa101appl.pport->pfrm[i].pcfg = new struDpa10xFrm_Cfg;
		dpa101appl.pport->pfrm[i].pcfg->frmtype = M_ME_NC_1;
		dpa101appl.pport->pfrm[i].pcfg->frminf = inf++;
		dpa101appl.pport->pfrm[i].pntnum = 1;
		dpa101appl.pport->pfrm[i].ppntcfg = (void*)new int16;
		*(int16*)dpa101appl.pport->pfrm[i].ppntcfg = syspnt++;
	}
	for(i = 0; i < NUM_TEST_FROZEN; i++ )
	{
		dpa101appl.pport->pfrm[NUM_TEST_MEASURE+i].pcfg = new struDpa10xFrm_Cfg;
		dpa101appl.pport->pfrm[NUM_TEST_MEASURE+i].pcfg->frmtype = M_IT_NB_1;
		dpa101appl.pport->pfrm[NUM_TEST_MEASURE+i].pcfg->frminf = inf++;
		dpa101appl.pport->pfrm[NUM_TEST_MEASURE+i].pntnum = 1;
		dpa101appl.pport->pfrm[NUM_TEST_MEASURE+i].ppntcfg = (void*)new int16;
		*(int16*)dpa101appl.pport->pfrm[NUM_TEST_MEASURE+i].ppntcfg = syspnt++;
	}

	test_lock = CreateMutex(NULL,FALSE,"test_mutex");
	return true;
}

#include "fsmid_def.h"
#include "fsmid_log.h"

void __create_folder(const char *fileName)
{
	const char *tag;
	for(tag=fileName;*tag;tag++)
	{
		if (*tag=='\\')
		{
			char buf[MAX_PATH],path[MAX_PATH];
			strcpy(buf,fileName);
			buf[strlen(fileName)-strlen(tag)+1]=NULL;
			strcpy(path,buf);
			if (chdir(path)==-1)
			{
				mkdir(path);
			}
		}
	}
}

void printLog(FSLOG *pLog)
{
	SYS_TIME64 tm64;
	if(!pLog)
	{
		printf("!LOG pointer is NULL.\n");
		return;
	}

	printf("Name:%s.\n",pLog->name);
	if(pLog->timeCreateUnix)
	{
		time_unix2sys(pLog->timeCreateUnix,&tm64);
		printf("Create:20%02d-%02d-%02d\n",tm64.year,tm64.mon,tm64.day);
	}
	else
		printf("Create: -N\\A-\n");
	printf("ItemCount:%5d. FormatSize:%8d.\n",pLog->unitNumber,pLog->formatedSize);
	_getch();
}

void two_sec_delay(SYS_TIME64 *tm64)
{
	SYS_TIME64 _tm64;
	do{
		fsmid_delayMs(100);
		glb_GetDateTime(&_tm64);
	}while(!(_tm64.sec != tm64->sec && _tm64.msec > 500 && !(_tm64.sec%2)));
}

float f_gen_rand(float range)
{
	float r = rand();
	r /= range;
	return r;
}

int i_gen_rand(int range)
{
	int r = rand();
	r %= range;
	return r;
}

static FILE* f_test_open(const char *path, const char *file)
{
	int i;
	char fPathName[MAX_PATH];
	FILE* fresult;

	i = sprintf(fPathName,"%s%s",path,file);
	fPathName[i] = 0;
	__create_folder(fPathName);
	fresult = fopen(fPathName,"wb+");
	if(fresult == NULL)
	{
		printf("fopen return NULL, error=%d/n", GetLastError());
		while(1);
	}
	fseek(fresult,0,SEEK_END);
	return fresult;
}

void __write_time(FILE *file, SYS_TIME64 *tm64)
{
	char buf[64];
	int i = sprintf(buf,"20%02d-%02d-%02d %02d:%02d:%02d",
		tm64->year,tm64->mon,tm64->day,
		tm64->hour,tm64->min,tm64->sec);
	fwrite(buf,i,1,file);
}

void __write_eol(FILE *file)
{
	const char buf[] = "\r\n";
	fwrite(buf,2,1,file);
}

extern char exeFullPath[256];

void __write_fixpt(/*FILE *file,*/ SYS_TIME64 *tm64, float value[])
{
	char buf[128];
	int i,j;
	FILE *file = f_test_open(exeFullPath,"\\raw\\FIXPT.csv");

	__write_time(file,tm64);
	for(j = 0; j < NUM_TEST_MEASURE; j++)
	{
		i = sprintf(buf,",%7.3f",value[j]);
		fwrite(buf,i,1,file);
	}
	__write_eol(file);
	fclose(file);
}

void __write_frz(/*FILE *file,*/ SYS_TIME64 *tm64, float value[])
{
	char buf[128];
	int i,j;
	FILE *file = f_test_open(exeFullPath,"\\raw\\FRZ.csv");

	__write_time(file,tm64);
	for(j = 0; j < NUM_TEST_FROZEN; j++)
	{
		i = sprintf(buf,",%7.3f",value[j]);
		fwrite(buf,i,1,file);
	}
	__write_eol(file);
	fclose(file);
}

void __write_ulog(/*FILE *file,*/ SYS_TIME64 *tm64, ULOGEVENT *event)
{
	char buf[128];
	int i;
	FILE *file = f_test_open(exeFullPath,"\\raw\\ULOG.csv");
	
	__write_time(file,tm64);

	i = sprintf(buf,",20%02d-%02d-%02d %02d:%02d:%02d.%03d",
		event->time.year,event->time.mon,event->time.day,
		event->time.hour,event->time.min,event->time.sec,
		event->time.msec);
	fwrite(buf,i,1,file);

	i = sprintf(buf,",%d,%d,%s",event->type,event->sts,event->buf);
	fwrite(buf,i,1,file);

	__write_eol(file);
	fclose(file);
}

void __write_prtlog(/*FILE *file,*/ SYS_TIME64 *tm64, PRTLOGEVENT *event)
{
	char buf[128];
	int i;
	FILE *file = f_test_open(exeFullPath,"\\raw\\PRTLOG.csv");

	__write_time(file,tm64);

	i = sprintf(buf,",20%02d-%02d-%02d %02d:%02d:%02d.%03d",
		event->time.year,event->time.mon,event->time.day,
		event->time.hour,event->time.min,event->time.sec,
		event->time.msec);
	fwrite(buf,i,1,file);

	i = sprintf(buf,",%s",event->buf);
	fwrite(buf,i,1,file);

	__write_eol(file);
	fclose(file);
}

void __write_soe(/*FILE *file,*/ SYS_TIME64 *tm64, SOEEVENT *event)
{
	char buf[128];
	int i;
	FILE *file = f_test_open(exeFullPath,"\\raw\\SOE.csv");

	__write_time(file,tm64);

	i = sprintf(buf,",20%02d-%02d-%02d %02d:%02d:%02d.%03d",
		event->time.year,event->time.mon,event->time.day,
		event->time.hour,event->time.min,event->time.sec,
		event->time.msec);
	fwrite(buf,i,1,file);

	i = sprintf(buf,"%d,%d,%d",event->pnt,event->val,event->ainum);
	fwrite(buf,i,1,file);

	__write_eol(file);
	fclose(file);
}

void __write_trd(/*FILE *file,*/ SYS_TIME64 *tm64, TRDEVENT *event)
{
	char buf[128];
	int i;
	FILE *file = f_test_open(exeFullPath,"\\raw\\TRD.csv");

	__write_time(file,tm64);

	i = sprintf(buf,"%d,%d",event->pnt,event->val);
	fwrite(buf,i,1,file);

	__write_eol(file);
	fclose(file);
}

DWORD WINAPI signal_generator(LPVOID lpParameter)
{
	int i;
	int tickSoe,tickTrd,tickUlog,tickPrtlog;
	SYS_TIME64 tm64;
	//FILE *fsoe,*ftrd,*fprtlog,*fulog,*fmea,*ffrz;

// 	fsoe = f_test_open((char *)lpParameter,"\\raw\\SOE.csv");
// 	ftrd = f_test_open((char *)lpParameter,"\\raw\\TRD.csv");
// 	fprtlog = f_test_open((char *)lpParameter,"\\raw\\PRTLOG.csv");
// 	fulog = f_test_open((char *)lpParameter,"\\raw\\ULOG.csv");
// 	fmea = f_test_open((char *)lpParameter,"\\raw\\FIXPT.csv");
// 	ffrz = f_test_open((char *)lpParameter,"\\raw\\FRZ.csv");

	tickSoe = i_gen_rand(59) + 1;
	tickTrd = i_gen_rand(59) + 1;
	tickUlog = i_gen_rand(59) + 1;
	tickPrtlog = i_gen_rand(59) + 1;

	while(1)
	{
		glb_GetDateTime(&tm64);

		WaitForSingleObject(test_lock,INFINITE);
		if(tm64.sec < 2)
		{
			tickSoe = i_gen_rand(59) + 1;
			tickTrd = i_gen_rand(59) + 1;
			tickUlog = i_gen_rand(59) + 1;
			tickPrtlog = i_gen_rand(59) + 1;
		}

		for( i = 0; i < NUM_TEST_MEASURE; i++ )
		{
			test_measure[i] = f_gen_rand(32.768f);
			test_measure[i] = min(test_measure[i],9999.0);
			test_measure[i] = max(test_measure[i],0.001);
		}
		__write_fixpt(&tm64,test_measure);
		for( i = 0; i < NUM_TEST_FROZEN; i++ )
		{
			test_frozen[i] = f_gen_rand(32.768f);
			test_frozen[i] = min(test_frozen[i],9999.0);
			test_frozen[i] = max(test_frozen[i],0.001);
		}
		__write_frz(&tm64,test_frozen);

		if(tm64.sec == tickUlog)
		{
			test_ulog = new ULOGEVENT;
			memcpy(&test_ulog->time,&tm64,sizeof(tm64));
			test_ulog->type = i_gen_rand(11) + 1;
			test_ulog->sts = i_gen_rand(2);
			sprintf(test_ulog->buf,"ULOG trigged at %02d:%02d:%02d.%03d",tm64.hour,tm64.min,tm64.sec,tm64.msec);
			__write_ulog(&tm64,test_ulog);
		}
		if(tm64.sec == tickPrtlog)
		{
			test_prtlog = new PRTLOGEVENT;
			ZeroMemory(test_prtlog,sizeof(PRTLOGEVENT));
			memcpy(&test_prtlog->time,&tm64,sizeof(tm64));
			sprintf(test_prtlog->buf,"ULOG trigged at %02d:%02d:%02d.%03d",tm64.hour,tm64.min,tm64.sec,tm64.msec);
			__write_prtlog(&tm64,test_prtlog);
		}
		if(tm64.sec == tickSoe)
		{
			test_soe = new SOEEVENT;
			ZeroMemory(test_soe,sizeof(SOEEVENT));
			memcpy(&test_soe->time,&tm64,sizeof(tm64));
			test_soe->pnt = i_gen_rand(NUM_TEST_FROZEN + NUM_TEST_MEASURE) + START_SYSPNT;
			test_soe->val = i_gen_rand(2);
			__write_soe(&tm64,test_soe);
		}
		if(tm64.sec == tickTrd)
		{
			test_trd = new TRDEVENT;
			ZeroMemory(test_trd,sizeof(TRDEVENT));
			//memcpy(&test_trd->time,&tm64,sizeof(tm64));
			test_trd->pnt = i_gen_rand(NUM_TEST_FROZEN + NUM_TEST_MEASURE) + START_SYSPNT;
			test_trd->val = i_gen_rand(2);
			__write_trd(&tm64,test_trd);
		}
		ReleaseMutex(test_lock);

		two_sec_delay(&tm64);
	}

	return 0;
}

void save_log_file(FSLOG *pLog)
{
	char fname[MAX_PATH];
	int i,j;
	FILE *file;
	unsigned char *buf;
	
	i = sprintf(fname,"%s\\log\\%s",exeFullPath,pLog->name);
	fname[i] = 0;

	remove(fname);
	__create_folder(fname);
	file = fopen(fname,"wb+");
	if(!file)
	{
		printf("Fail to create file. err: %d.\n",GetLastError());
		return;
	}
	FSLOG_Lock(pLog);
	buf = new unsigned char[pLog->pInformation->unitSize];
	j = FSLOG_ReadFmt(pLog,-1UL,fname);
	fwrite(fname,j,1,file);
	for(i = 0; i < FSLOG_GetUnitCount(pLog); i++ )
	{
		j = FSLOG_ReadFmt(pLog,i,fname);
		fwrite(fname,j,1,file);
	}
	FSLOG_Unlock(pLog);

	delete [] buf;
	fclose(file);
}

DWORD WINAPI threadConsole(LPVOID lpParameter)
{
	int code;
	char cmd[MAX_PATH];
	unsigned int i,nItem = 0;
	FSLOG *pLog;

	init_dtu_enviroment((char*)lpParameter);
	srand((unsigned int)time(NULL));
	Sleep(2000);
	HANDLE t1 = CreateThread(NULL, 0, signal_generator, lpParameter, 0, NULL);  
	while(1)
	{
		printf("input command code:");
		code = _getch();
		printf("\n");
		if(code == 0x1B)//ESC
			break;
		switch(code)
		{
		case 'l':
		case 'L':
			ZeroMemory(cmd,MAX_PATH);
			printf("Please input filter string:");
			for(i = 0; i < MAX_PATH-1;i ++)
			{
				cmd[i] = _getch();
				_putch(cmd[i]);
				if(cmd[i] == 0xD || cmd[i] == 0x1B)
				{
					printf("\r\n");
					cmd[i] = 0;
					break;
				}
			}
			nItem = FSLOG_Filter(cmd);
			printf("\n%d files selected.\n",nItem);
			break;
		case 'd':
		case 'D':
			printf("%d item will be listed.\n",nItem);
			for(i = 0; i < nItem; i++)
				printLog(FSLOG_GetFiltedItem(i));
			printf("All item listed.\n");
			break;
		case 's':
		case 'S':
			printf("Please input index to save:");
			scanf("%d",&i);
			pLog = FSLOG_GetFiltedItem(i);
			if(!pLog)
				printf("\n!Invalid selection.\n");
			else
			{
				save_log_file(pLog);
				printf("\n \"%s\" will be saved.\n",pLog->name);
			}
			break;
		case 'r':
		case 'R':
			FSLOG_ReleaseFilter();
			printf("Filer released.\n");
			break;
		}
	}
	exit(1);
	return 0;
}