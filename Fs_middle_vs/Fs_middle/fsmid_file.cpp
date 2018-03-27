
#include "list_linux.h"
#include "fsmid_type.h"
#include "fsmid_system.h"
#include <stdio.h>
#include <string.h>

extern FSMID_SYSTEM fsmid_system;

int FSMID_Open(FSMID_FHANDLE* pHandler, const char* pPathName, FSMID_OPEN_ATTR attribute)
{
	int result = FSMIDR_OK;
	FSMID_FILE *pFile;

	if(pHandler == NULL)
		return FSMIDR_BAD_ARGUMENT;

	pFile = fsmid_search(pPathName);
	if(attribute & FSMIDO_CREATE)
	{
		if(pFile)
			return FSMIDR_CONFLICT;
		pFile = fsmid_create_new(pPathName,attribute);
	}
	else
	{
		pFile = fsmid_open_exist(pPathName,attribute);
	}
	if(pFile)
	{
		*pHandler = (FSMID_FHANDLE)pFile;
		return FSMIDR_OK;
	}
	return FSMIDR_CONFLICT;
}

int FSMID_Close(FSMID_FHANDLE handle)
{
	int result = FSMIDR_OK;
	FSMID_FILE *pFile = (FSMID_FILE*)handle;

	result = fsmid_release_resource(pFile);
	if(result != FSMIDR_OK)
		return result;

	return 0;
}

int FSMID_Remove(const char* pPathName)
{
	int result = FSMIDR_OK;
	FSMID_FILE *pFile = fsmid_search(pPathName);

	if(pFile == NULL)
		return FSMIDR_NOT_EXIST;

	if(pFile->status == FSMIDS_OPENED)
		return FSMIDR_CONFLICT;

	result = fsmid_ulink_system(pFile);
	if(result != FSMIDR_OK)
		return result;

	result = fsmid_release_resource(pFile);
	if(result != FSMIDR_OK)
		return result;

	fsmid_free(pFile);
	return 0;
}

int FSMID_Read(FSMID_FHANDLE handle, void* pData, unsigned int length)
{
	int result = FSMIDR_OK;
	FSMID_FILE *pFile = (FSMID_FILE*)handle;
	unsigned char* dataPtr = (unsigned char*)pData;
	unsigned int acturalOffset,copyLen;
	FSMID_LIST_LOG *iterator;

	if(pFile == NULL || pData == NULL || length == 0 || pFile->status == FSMIDS_NOT_OPEN || (pFile->attribute&FSMIDO_READ)==0)
		return FSMIDR_BAD_ARGUMENT;
	
	acturalOffset = pFile->bufOffset + pFile->pCurrent - pFile->pBuffer;
	if(acturalOffset + length > pFile->size)
		return FSMIDR_BAD_ARGUMENT;

	//read current buf is enough
	if(pFile->pCurrent + length <= pFile->pBuffer + pFile->bufSize)
	{
		memcpy(dataPtr,pFile->pCurrent,length);
		pFile->pCurrent += length;
		return FSMIDR_OK;
	}
	//read current buf not enough
	else
	{
		copyLen = (unsigned int)(pFile->pBuffer -pFile->pCurrent) + pFile->bufSize;
		memcpy(dataPtr,pFile->pCurrent,copyLen);
		dataPtr += copyLen;
		length -= copyLen;
		while(length){
			iterator = fsmid_next_log(pFile);
			if(iterator == NULL)
				return FSMIDR_GENERAL;

			if(pFile->pCurrent + length <= pFile->pBuffer + pFile->bufSize)
			{
				memcpy(dataPtr,pFile->pCurrent,length);
				pFile->pCurrent += length;
				return FSMIDR_OK;
			}
			else
			{
				copyLen = (unsigned int)(pFile->pBuffer -pFile->pCurrent) + pFile->bufSize;
				memcpy(dataPtr,pFile->pCurrent,copyLen);
				dataPtr += copyLen;
				length -= copyLen;
			}
		}
	}

	return 0;
}

#define TRY(cond,_err)		do{if(cond){result = _err; goto ErrorCondition;}}while(0)

int FSMID_Write(FSMID_FHANDLE handle, const void* pData, unsigned int length)
{
	int result = FSMIDR_OK;
	FSMID_FILE *pFile = (FSMID_FILE*)handle;
	FSMID_LIST_LOG* pLog;

	if(pFile == NULL || pData == NULL || length == 0 || pFile->status == FSMIDS_NOT_OPEN || (pFile->attribute&FSMIDO_WRITE)==0)
		return FSMIDR_BAD_ARGUMENT;

	fsmid_mutex_lock(pFile->mutex);
	//write log implement not ready
	if(pFile->pFunctionTable[0].put_data == NULL)
	{
		TRY(pFile->pBuffer != NULL, FSMIDR_LEAK_MEMORY);
		pFile->pBuffer = fsmid_malloc(unsigned char,length);
		TRY(pFile->pBuffer == NULL, FSMIDR_LEAK_MEMORY);

		pFile->bufSize = length;
		pFile->pCurrent = pFile->pBuffer;
		pFile->bufOffset = 0;
		pFile->size = length;
	}
	else
	{
		pLog = list_entry(pFile->headPortTable->prev, FSMID_LIST_LOG, _node);
		TRY(pLog == NULL, FSMIDR_LEAK_MEMORY);
		result = pFile->pFunctionTable[0].put_data(pLog->handle,pFile->bufOffset,pData, length);
		TRY(result != FSMIDR_OK,result);
		pFile->bufOffset += length;
		//offset might be move forward before.
		if(pFile->bufOffset > pFile->size)
			pFile->size = pFile->bufOffset;
	}
	fsmid_get_systime(&pFile->time);

	fsmid_mutex_unlock(pFile->mutex);
	return 0;

ErrorCondition:
	fsmid_mutex_unlock(pFile->mutex);
	return result;
}

int FSMID_Seek(FSMID_FHANDLE handle, signed int offset, FSMID_SEEK method)
{
	int result = FSMIDR_OK;
	unsigned int new_off;
	FSMID_FILE *pFile = (FSMID_FILE*)handle;

	if(pFile == NULL)
		return FSMIDR_BAD_ARGUMENT;

	fsmid_mutex_lock(pFile->mutex);

	switch(method)
	{
	case FSMIDS_BEGIN:
		new_off = offset;
		TRY(pFile->size < offset, FSMIDR_BAD_ARGUMENT);
		break;
	case FSMIDS_CURRENT:
		new_off = pFile->bufOffset + offset + (unsigned int)(pFile->pCurrent - pFile->pBuffer);
		TRY(new_off < 0, FSMIDR_BAD_ARGUMENT);
		TRY(new_off > pFile->size, FSMIDR_BAD_ARGUMENT);
		break;
	case FSMIDS_END:
		new_off = pFile->size - offset;
		TRY(pFile->size < offset, FSMIDR_BAD_ARGUMENT);
		break;
	default:
		TRY(1, FSMIDR_BAD_ARGUMENT);
	}

	if(pFile->attribute & FSMIDO_READ)
	{
		//previous portNode or less
		if(new_off < pFile->bufOffset)
		{
			fsmid_free(pFile->pBuffer);
			//could be optimize search from current log item
			result = fsmid_search_log(pFile, new_off);
		}
		//next portNode or more
		else if( new_off >= pFile->bufOffset + pFile->bufSize)
		{
			fsmid_free(pFile->pBuffer);
			//could be optimize search from current log item
			result = fsmid_search_log(pFile, new_off);
		}
		//current node
		else
		{
			pFile->pCurrent = new_off - pFile->bufOffset + pFile->pBuffer;
		}
	}
	fsmid_mutex_unlock(pFile->mutex);
	return 0;

ErrorCondition:
	fsmid_mutex_unlock(pFile->mutex);
	return result;
}

int FSMID_Stat(const char* pPathName, FSMID_STAT *pStat)
{
	FSMID_FILE *pFile;
	
	if(pStat == NULL)
		return FSMIDR_BAD_ARGUMENT;
	pFile = fsmid_search(pPathName);
	if(pFile == NULL)
		return FSMIDR_NOT_EXIST;

	pStat->attribute = pFile->attribute;
	pStat->status = pFile->status;
	memcpy(&pStat->time,&pFile->time,sizeof(pFile->time));
	pStat->size = pFile->size;
	pStat->offset = pFile->bufOffset + (unsigned int)(pFile->pCurrent - pFile->pBuffer);

	return 0;
}