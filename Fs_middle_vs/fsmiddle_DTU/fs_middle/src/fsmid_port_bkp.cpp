
#include "..\inc\fsmid_def.h"

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
static unsigned int fsmid_operate_index;
static unsigned char causes;


int fsmid_folder_request( unsigned int id, const char* folderName, unsigned char folderNum, CP56TIME2A *pTimePare)
{
	
}

int fsmid_read_file_active( const char* folderName, unsigned char fileName)
{

}

int fsmid_read_file_data( unsigned int id, unsigned int segment, unsigned char continuous)
{

}



int FileMid_101Requir(unsigned char *pBuf, unsigned short length, unsigned short reason)
{
	FSMID_CODE code = pBuf[0];
	fsmid_operate_index = 0;
	switch(code)
	{
	case	FSCODE_ReadFolderCall:					//H->D
	case	FSCODE_ReadFileActive:					//H->D
	case	FSCODE_ReadFileTransferConfirm:			//H->D
	case	FSCODE_WriteFileActive:					//H->D
	case	FSCODE_WriteFileTransfer:				//H->D
	case (FSCODE_ReadFolder):
		memcpy(&id,pBuf + 1,4);
		len = pBuf[5];
		return fsmid_folder_request(id,pBuf + 6,len,pBuf[6+len]?(pBuf + 7 + len):NULL);
		break;
	case (FSCODE_ReadFileActive):
		break;
	case (FSCODE_ReadFileTransfer):
		break;
	case (FSCODE_WriteFileActive):
		break;
	case (FSCODE_ReadFolder):
		break;
	default:
		fsmid_assert(0,__FUNCTION__,__LINE__);
		return 0;
	}
}


int FileMid_101Continue(unsigned char *texbuf, unsigned short *texlen, unsigned short *pReason)
{

}