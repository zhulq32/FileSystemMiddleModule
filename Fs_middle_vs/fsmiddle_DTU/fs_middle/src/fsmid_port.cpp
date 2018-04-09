
#include "..\inc\fsmid_def.h"
#include "..\inc\fsmid_file.h"

typedef enum __fsmd_code{
	FSCODE_ReadFolderCall = 0,				//H->D
	FSCODE_ReadFolderConfirm,				//D->H
	FSCODE_ReadFileActive,					//H->D
	FSCODE_ReadFileActiveConfirm,			//D->H
	FSCODE_ReadFileTransfer,				//D->H
	FSCODE_ReadFileTransferConfirm,			//H->D
	FSCODE_WriteFileActive,					//H->D
	FSCODE_WriteFileActiveConfirm,			//D->H
	FSCODE_WriteFileTransfer,				//H->D
	FSCODE_WriteFileTransferConfirm,		//D->H
	FSCODE_Invalid,
}FSMID_CODE;


typedef struct __fsmid_file_info{
	char *pPathName;
	unsigned int size;
	CP56TIME2A time;
	unsigned char attribute;
}FSMID_FILE_INFO;

typedef union {
	struct __read_folder_call{
		char *name;
		unsigned int id;
		CP56TIME2A timeStart;
		unsigned char reserved1;
		CP56TIME2A timeEnd;
		unsigned char available;
	}READ_FOLDER_CALL;

	struct __read_folder_confirm{
		unsigned char result;
		unsigned char followed;
		unsigned char numFolder;
		unsigned char reserved1;
		unsigned int id;
		FSMID_FILE_INFO *info;
		unsigned char reserved[12];
	}READ_FOLDER_CONFIRM;

	struct __read_file_active{
		char *name;
		unsigned char reserved[20];
	}READ_FILE_ACTIVE;

	struct __read_file_active_confirm{
		char *name;
		unsigned int id;
		unsigned int size;
		unsigned char descriptor;
		unsigned char reserved[11];
	}READ_FILE_ACTIVE_CONFIRM;

	struct __read_file_transfer{
		unsigned int id;
		unsigned int segment;
		unsigned char* data;
		unsigned char size;
		unsigned char checksum;
		unsigned char continuous;
		unsigned char reserved1[9];
	}READ_FILE_TRANSFER;

	struct __read_file_transfer_confirm{
		unsigned int id;
		unsigned int segment;
		unsigned char continuous;
		unsigned char reserved1[15];
	}READ_FILE_TRANSFER_CONFIRM;

	struct __write_file_active{
		char *name;
		unsigned int id;
		unsigned int size;
		unsigned char reserved[12];
	}WRITE_FILE_ACTIVE;

	struct __write_file_active_confirm{
		char *name;
		unsigned int id;
		unsigned int size;
		unsigned char descriptor;
		unsigned char reserved[11];
	}WRITE_FILE_ACTIVE_CONFIRM;

	struct __write_file_transfer{
		unsigned int id;
		unsigned int segment;
		unsigned char* data;
		unsigned char size;
		unsigned char checksum;
		unsigned char continuous;
		unsigned char reserved1[9];
	}WRITE_FILE_TRANSFER;

	struct __write_file_transfer_confirm{
		unsigned int id;
		unsigned int segment;
		unsigned char continuous;
		unsigned char reserved1[15];
	}WRITE_FILE_TRANSFER_CONFIRM;
}FSMID_CODE_DATA;


static FSMID_CODE fsmid_operate_code;
static FSMID_CODE fsmid_response_code;
static unsigned int fsmid_operate_id;
static unsigned int fsmid_operate_index;
static unsigned char fsmid_causes;
static void* fsmid_handle;

FSMID_CODE_DATA *init_ReadFolderCall(unsigned char *pBuf, unsigned short length);
FSMID_CODE_DATA *init_ReadFileActive(unsigned char *pBuf, unsigned short length);
FSMID_CODE_DATA *init_ReadFileTransferConfirm(unsigned char *pBuf, unsigned short length);
FSMID_CODE_DATA *init_WriteFileActive(unsigned char *pBuf, unsigned short length);
FSMID_CODE_DATA *init_WriteFileTransfer(unsigned char *pBuf, unsigned short length);

FSMID_CODE_DATA *init_ReadFolderConfirm();
FSMID_CODE_DATA *init_ReadFileActiveConfirm();
FSMID_CODE_DATA *init_ReadFileTransfer();
FSMID_CODE_DATA *init_WriteFileActiverConfirm();
FSMID_CODE_DATA *init_WriteFileTransferrConfirm();

void release_Code(FSMID_CODE_DATA* pData, FSMID_CODE code,unsigned char *texbuf, unsigned short *texlen);

void *FSMID_ListFolder(const char* pPath, CP56TIME2A* timeStart, CP56TIME2A* timeEnd);
int FSMID_GetFolderCount(void*);
void FSMID_GetFolderItem(void*, /*FSMID_FOLDER_ITEM*/void*, unsigned int count);

int FileMid_101Requir(unsigned char *pBuf, unsigned short length, unsigned short causes)
{
	FSMID_CODE_DATA *param;
	int dataContinue = 0;
	fsmid_operate_code = (FSMID_CODE)pBuf[0];
	fsmid_operate_index = 0;
	fsmid_causes = causes;
	switch(fsmid_operate_code)
	{
	case	FSCODE_ReadFolderCall:					//H->D
		param = init_ReadFolderCall(pBuf,length);
		fsmid_handle = FSMID_ListFolder(param->READ_FOLDER_CALL.name, &param->READ_FOLDER_CALL.timeStart,&param->READ_FOLDER_CALL.timeEnd);
		fsmid_operate_id = param->READ_FOLDER_CALL.id;
		fsmid_response_code = FSCODE_ReadFolderConfirm;
		break;
	case	FSCODE_ReadFileActive:					//H->D
		param = init_ReadFileActive(pBuf,length);
		fsmid_handle = FSMID_Open(param->READ_FILE_ACTIVE.name,FSMIDO_READ);
		fsmid_response_code = FSCODE_ReadFileActiveConfirm;
		break;
	case	FSCODE_ReadFileTransferConfirm:			//H->D
		param = init_ReadFileTransferConfirm(pBuf,length);
		fsmid_operate_id = param->READ_FILE_TRANSFER_CONFIRM.id;
		if(!param->READ_FILE_TRANSFER_CONFIRM.continuous)
		{
			FSMID_Close(fsmid_handle);
			fsmid_handle = NULL;
		}
		fsmid_response_code = FSCODE_Invalid;//FSCODE_ReadFileTransferConfirm;
		break;
	case	FSCODE_WriteFileActive:					//H->D
		param = init_WriteFileActive(pBuf,length);
		fsmid_handle = FSMID_Open(param->WRITE_FILE_ACTIVE.name,FSMIDO_WRITE);
		fsmid_operate_id = param->WRITE_FILE_TRANSFER.id;
		FSMID_Seek(fsmid_handle,param->WRITE_FILE_ACTIVE.size,FSMIDS_BEGIN);
		fsmid_response_code = FSCODE_Invalid;//FSCODE_ReadFolderConfirm;
		break;
	case	FSCODE_WriteFileTransfer:
		param = init_WriteFileTransfer(pBuf,length);
		//FSMID_Seek(fsmid_handle,param->WRITE_FILE_TRANSFER.segment,FSMIDS_BEGIN);
		FSMID_Write(fsmid_handle,param->WRITE_FILE_TRANSFER.data,param->WRITE_FILE_TRANSFER.size);
		fsmid_operate_id = param->WRITE_FILE_TRANSFER.id;
		if(param->WRITE_FILE_TRANSFER.continuous)
		{
			FSMID_Close(fsmid_handle);
			fsmid_handle = NULL;
			fsmid_response_code = FSCODE_Invalid;//FSCODE_ReadFolderConfirm;
		}
		else
		{
			FSMID_Close(fsmid_handle);
			fsmid_response_code = FSCODE_WriteFileTransferConfirm;
		}
		break;
	default:
		fsmid_assert(0,__FUNCTION__,__LINE__);
		return 0;
	}
	release_Code(param,fsmid_operate_code,NULL,NULL);
	return 0;
}


int FileMid_101Continue(unsigned char *texbuf, unsigned short *texlen, unsigned short *pCauses)
{
	FSMID_CODE_DATA *param;
	unsigned int i;
	switch(fsmid_operate_code)
	{
	case	FSCODE_ReadFolderConfirm:					//H->D
		param = init_ReadFolderConfirm();
		i = FSMID_GetFolderCount(fsmid_handle);
		param->READ_FOLDER_CONFIRM.numFolder = min(i,2);
		FSMID_GetFolderItem(fsmid_handle,param->READ_FOLDER_CONFIRM.info,param->READ_FOLDER_CONFIRM.numFolder);
		param->READ_FOLDER_CONFIRM.followed = ((i == 2)?0:1);
		param->READ_FOLDER_CONFIRM.result = 0;			
		fsmid_response_code = FSCODE_ReadFolderConfirm;
		break;
	case	FSCODE_ReadFileActiveConfirm:					//H->D
		param = init_ReadFileActiveConfirm();
		if(fsmid_handle == NULL)
			param->READ_FILE_ACTIVE_CONFIRM.descriptor = 1;
		else
		{
			param->READ_FILE_ACTIVE_CONFIRM.descriptor = 0;
			FSMID_Stat((const char*)fsmid_handle,(FSMID_STAT*)param);
		}
		fsmid_response_code = FSCODE_ReadFileActiveConfirm;
		break;
	case	FSCODE_ReadFileTransfer:			//H->D
		param = init_ReadFileTransfer();
		FSMID_Read(fsmid_handle,param->READ_FILE_TRANSFER.data,param->WRITE_FILE_TRANSFER.size);
		fsmid_response_code = FSCODE_Invalid;//FSCODE_ReadFileTransferConfirm;
		break;
	case	FSCODE_WriteFileActiveConfirm:					//H->D
		param = init_WriteFileActiverConfirm();
		if(fsmid_handle == NULL)
			param->READ_FILE_ACTIVE_CONFIRM.descriptor = 1;
		else
		{
			param->READ_FILE_ACTIVE_CONFIRM.descriptor = 0;
			FSMID_Stat((const char*)fsmid_handle,(FSMID_STAT*)param);
		}
		fsmid_response_code = FSCODE_Invalid;//FSCODE_ReadFolderConfirm;
		break;
	case	FSCODE_WriteFileTransferConfirm:
		param = init_WriteFileTransferrConfirm();
		FSMID_Stat((const char*)fsmid_handle,(FSMID_STAT*)param);
		break;
	default:
		fsmid_assert(0,__FUNCTION__,__LINE__);
		return 0;
	}
	release_Code(param,fsmid_response_code,texbuf,texlen);
	*pCauses = fsmid_causes;
	return 0;
}