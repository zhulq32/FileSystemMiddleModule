
#include "fs_middle/inc/fsmid_log.h"
#include <windows.h>

#define BLOCK_SIZE		4096
#define FILE_PATH_NAME	"C:\\test_log.bin"

static FILE* s_file = NULL;

int write_file(unsigned int address, const void* data, unsigned int length)
{
	fseek(s_file,address,SEEK_SET);
	fwrite(data,length,1,s_file);
	printf("[WRITE] 0x%8X, %5d.\n",address,length);
	return 0;
}

int read_file(unsigned int address, void* data, unsigned int length)
{
	fseek(s_file,address,SEEK_SET);
	fread(data,length,1,s_file);
	printf("[READ]  0x%8X, %5d.\n",address,length);
	return 0;
}

int erase_file(unsigned int address)
{
	unsigned char * buf = new unsigned char[BLOCK_SIZE];
	memset(buf,0xFF,BLOCK_SIZE);
	fseek(s_file,address,SEEK_SET);
	fwrite(buf,BLOCK_SIZE,1,s_file);
	printf("[ERASE] 0x%8X, %5d.\n",address,BLOCK_SIZE);
	delete [] buf;
	return 0;
}

static FSLOG_FUNCTION s_function = {
	write_file,
	read_file,
	erase_file,
};

static FSLOG_INFORMATION s_information = {
	0,
	6,
	BLOCK_SIZE,
	16,
	1024,
};

void test_log()
{
	int code;
	unsigned int data[4];
	fpos_t pos;
	FSLOG* pLog;
	int offset;
	printf("====================== TEST LOG ============================\n");
	s_file = fopen(FILE_PATH_NAME,"rb+");
	if(s_file == NULL)
	{
		s_file = fopen(FILE_PATH_NAME,"wb+");
		if(s_file == NULL)
		{
			printf("Fail to create file \"%s\".\n",FILE_PATH_NAME);
			return;
		}
	}
	offset = fseek(s_file,0,SEEK_END);
	fgetpos(s_file,&pos);
	if(pos == 0)
	{
		printf("Create new file. erase all.\n");
		for(offset = 0; offset < s_information.blockNumber * s_information.blockSize; offset += s_information.blockSize)
			erase_file(offset);
		printf("Init file done.\n\n");
	}
	fseek(s_file,0,SEEK_SET);
	pLog = FSLOG_Init(FILE_PATH_NAME,&s_function,&s_information);

	printf("LOG %dUnit, Old:%d, Wr:%d, Rd:%d\n",pLog->unitNumber,pLog->indexOldest,pLog->indexWrite,pLog->indexRead);

	while(1)
	{
		code = getchar();
		if(code == ' ') break;
		switch(code)
		{
		case 'w':
		case 'W':
			//memset(data,0,sizeof(data));
			memcpy(data,&pLog->unitNumber,sizeof(data));
			FSLOG_Write(pLog,data);
			break;
		}
	}

	printf("====================== DONE ============================\n");
	fclose(s_file);
}