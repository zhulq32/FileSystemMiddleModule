
#include "fsmid_port.h"
#include "fsmid_log.h"
#include <windows.h>
#include <conio.h>
#include <ctype.h>
#include <stdio.h>

#define TEST_UNIT_NUMBER		128//1024
#define TEST_UNIT_SIZE			128//16
#define TEST_BLOCK_SIZE			4096

#define SIM_FLASH_SIZE			(16<<20)
#define FLASH_FILE_NAME			"simFLASH.bin"
#define TEST_DATA_FNAME			"Romeo and Juliet.txt"
#define TEST_RESL_FNAME			"result.txt"

FILE* sim_flash_file = NULL;
static FILE* test_data_file = NULL;
static FILE* result_file = NULL;

int write_flash(unsigned int address, const void* data, unsigned int length)
{
	unsigned int i;
	const unsigned char *raw = (const unsigned char *)data;
	unsigned char * buf = new unsigned char[length];
	fsmid_assert(address + length <= SIM_FLASH_SIZE,__FILE__,__LINE__);
	fseek(sim_flash_file,address,SEEK_SET);
	fread(buf,length,1,sim_flash_file);
	for(i = 0; i < length; i++)
		buf[i] &= raw[i];
	fseek(sim_flash_file,address,SEEK_SET);
	fwrite(buf,length,1,sim_flash_file);
	//printf("[WRITE] 0x%8X, %5d.\n",address,length);
	delete [] buf;
	return 0;
}

int read_flash(unsigned int address, void* data, unsigned int length)
{
	fsmid_assert(address + length <= SIM_FLASH_SIZE,__FILE__,__LINE__);
	fseek(sim_flash_file,address,SEEK_SET);
	fread(data,length,1,sim_flash_file);
	//printf("[READ]  0x%8X, %5d.\n",address,length);
	return 0;
}

int erase_flash(unsigned int address, unsigned int length)
{
	unsigned char * buf = new unsigned char[TEST_BLOCK_SIZE];
	fsmid_assert(address + length <= SIM_FLASH_SIZE,__FILE__,__LINE__);
	memset(buf,0xFF,TEST_BLOCK_SIZE);
	fseek(sim_flash_file,address,SEEK_SET);
	while(length)
	{
		fwrite(buf,TEST_BLOCK_SIZE,1,sim_flash_file);
		length -= TEST_BLOCK_SIZE;
	}
	//printf("[ERASE] 0x%8X, %5d.\n",address,length);
	delete [] buf;
	return 0;
}

int format_file(char *buf, const void* data)
{
	if(!buf) return TEST_UNIT_SIZE;
	memcpy(buf,data,TEST_UNIT_SIZE);
	buf[TEST_UNIT_SIZE] = '\0';
	return TEST_UNIT_SIZE;
}

static FSLOG_INTERFACE s_interface = {
	write_flash,
	read_flash,
	erase_flash,
};

static FSLOG_FUNCTION s_function = {
// 	write_flash,
// 	read_flash,
// 	erase_flash,
	NULL,
	format_file,
	NULL,
};

static FSLOG_INFORMATION s_information = {
	0,
	6,
	TEST_BLOCK_SIZE,
	TEST_UNIT_SIZE,
	TEST_UNIT_NUMBER,
};

bool init_test_enviroment(char *pPath)
{
	fpos_t pos;
	int offset;
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

	memset(thePath,0,sizeof(thePath));
	sprintf(thePath,"%s\\%s",pPath,TEST_DATA_FNAME);
	printf("Try open flash file: %s\n",thePath);
	test_data_file = fopen(thePath,"rb");
	if(sim_flash_file == NULL)
	{
		printf("Fail to open file \"%s\".\n",thePath);
		return false;
	}

	return true;
}

void log_verify(FSLOG* pLog)
{
	unsigned char *pSrc;
	unsigned char *pTar;
	unsigned char *ptr;
	unsigned int i;
	unsigned int len = FSLOG_GetUnitCount(pLog) * FSLOG_GetUnitSize(pLog);
	unsigned int offset = FSLOG_GetPointerIndex(pLog,0) * pLog->unitPerBlock * FSLOG_GetUnitSize(pLog);

	printf("File from 0x%X, %d Bytes.\n",offset,len);
	printf("LOG from item %d. %d units.\n",FSLOG_GetPointerIndex(pLog,0) * pLog->unitPerBlock, FSLOG_GetUnitCount(pLog));
	pSrc = new unsigned char[len];
	pTar = new unsigned char[len];

	for(i = 0, ptr = pSrc; i < FSLOG_GetUnitCount(pLog); i++,ptr += FSLOG_GetUnitSize(pLog))
		FSLOG_ReadData(pLog,i,ptr);
	fseek(test_data_file,offset,SEEK_SET);
	fread(pTar,len,1,test_data_file);
	for(i = 0; i < len; i++)
	{
		if(pSrc[i] != pTar[i])
		{
			printf("Verify fail at 0x%X. LOG[%02X], FILE[%02X].\n",i,pSrc[i],pTar[i]);
			break;
		}
	}
	if(i >= len)
		printf("Verify Pass.\n");
	delete [] pTar;
	delete [] pSrc;
// 	if(memcmp(pSrc,pTar,len)==0)
// 		printf(" Verify Pass.\n");
// 	else
// 		printf(" Verify Fail.\n");
}

void test_log(TCHAR* pPath)
{
	fpos_t pos;
	int code;
	FSLOG* pLog;
	FSLOG* pEvent;
	TCHAR resPath[MAX_PATH];
	unsigned char data[TEST_UNIT_SIZE];
	unsigned int indexRead = 0,i,cycle;

	FSLOG_Init(&s_interface);
	memset(resPath,0,sizeof(resPath));
	sprintf(resPath,"%s\\%s",pPath,TEST_RESL_FNAME);
	printf("====================== TEST LOG ============================\n");
	if(!init_test_enviroment(pPath))
		return;
	pLog = FSLOG_Open(FLASH_FILE_NAME,&s_function,&s_information,FSLOG_EVENT_WRITE|FSLOG_EVENT_FULL);

	printf("LOG %dUnit, First:%d, Last:%d.\n",pLog->unitNumber,pLog->indexFirst,pLog->indexLast);

	while(1)
	{
		printf("input command code:");
		code = _getch();
		printf("\n");
		if(code == ' ') break;
		switch(code)
		{
// 		case 'e':
// 			do{
// 				pEvent = FSLOG_GetEvent();
// 				if(pEvent)
// 					printf("[EVENT] Log:%s\n",pEvent->name);
// 			}while(pEvent);
// 			printf(" done.\n");
// 			break;
		case '\\':
			fseek(test_data_file,0,SEEK_SET);
			printf("Reset log.\n");
			FSLOG_Clear(pLog);
			break;
		case '?':
			printf("LOG %dUnit, First:%d, Last:%d.\n",pLog->unitNumber,pLog->indexFirst,pLog->indexLast);
			break;
		case 'o':
			printf("input offset: ");
			scanf("%d", &cycle);
			fseek(test_data_file,cycle*TEST_UNIT_SIZE,SEEK_SET);
			printf("\noffset set to unit %d\n",cycle);
			break;
		case 'w':
		case 'W':
			printf("input unit number: ");
			scanf("%d", &cycle);
			//_getch();
// 			if(cycle > TEST_UNIT_NUMBER)
// 			{
// 				printf("number should less or equal %d\n",TEST_UNIT_NUMBER);
// 				break;
// 			}
			for(i = 0; i < cycle; i++)
			{
				fread(data,TEST_UNIT_SIZE,1,test_data_file);
				FSLOG_Write(pLog,data);
			}
			printf("\n done. %d Blocks written.\n",pLog->pointerId);
			break;
		case 'm':
		case 'M':
			printf("input move index: ");
			scanf("%d", &cycle);
			if(cycle > FSLOG_GetUnitCount(pLog))
			{
				printf("number should less or equal %d\n",FSLOG_GetUnitCount(pLog));
				break;
			}
			indexRead = cycle;
			printf("\n done.\n");
			break;
		case 'r':
		case 'R':
			printf("input unit number: ");
			scanf("%d", &cycle);
			//_getch();
			if(cycle > FSLOG_GetUnitCount(pLog))
			{
				printf("number should less or equal %d\n",FSLOG_GetUnitCount(pLog));
				break;
			}
			result_file = fopen(resPath,"wb+");
			if(result_file == NULL)
			{
				printf("Fail to open file \"%s\".\n",resPath);
				break;
			}
			for(i = 0; i < cycle; i++,indexRead++)
			{
				FSLOG_ReadData(pLog,indexRead,data);
				fwrite(data,TEST_UNIT_SIZE,1,result_file);
			}
			fclose(result_file);
			printf("\n done.\n");
			break;
		case 'v':
		case 'V':
			cycle = FSLOG_GetUnitCount(pLog);
			printf("verify data: %d\n", cycle * TEST_UNIT_SIZE);
			log_verify(pLog);
			break;
		}
	}
	fseek(test_data_file,0,SEEK_CUR);
	fgetpos(test_data_file,&pos);
	printf("Data source file pointer %d.\n",pos/TEST_UNIT_SIZE);
	printf("====================== DONE ============================\n");
	fclose(sim_flash_file);
	if(test_data_file) fclose(test_data_file);
}

