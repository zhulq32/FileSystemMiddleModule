/**
*
* @file fsmid_file.c
*
* @brief
*
* @author  Thomas.Zhu
*
* @version 1.0
*
* @note Copyright (C), 2018-2018.
*
* @date  2018-3-28
*
* history
*
*    -#Create
*
*/


#include "list_linux.h"
#include "fsmid_type.h"
#include "fsmid_system.h"
#include <stdio.h>
#include <string.h>

#ifndef min
#define min(a,b)		(((a)<(b))?(a):(b))
#endif

extern FSMID_SYSTEM fsmid_system;

int FSMID_Open(FSMID_FHANDLE* pHandler, const char* pPathName, FSMID_OPEN_ATTR attribute)
{
	int result = FSMIDR_OK;
	FSMID_FILE *pFile;

	fsmid_assert(pHandler && (attribute&FSMID_ACCESS_MASK),__FUNCTION__,__LINE__);
	fsmid_assert((attribute & FSMID_CREATE_MASK) != FSMIDO_CREATE_INVALID,__FUNCTION__,__LINE__);

	pFile = fsmid_search(pPathName);
	if(attribute & FSMID_CREATE_MASK)
	{
		if(pFile)
			return FSMIDR_CONFLICT;
		result = fsmid_create_new(&pFile,pPathName,attribute);
		if(result != FSMIDR_OK)
			return result;
	}
	else
	{
		pFile->attribute |= attribute;
		//((unsigned char)pFile->attribute) = (unsigned char)pFile->attribute + (unsigned char)attribute;
		result = fsmid_init_exist(pFile);
		if(result != FSMIDR_OK)
			return result;
	}
	pFile->status = FSMIDS_OPENED;
	*pHandler = (FSMID_FHANDLE)pFile;
	return FSMIDR_OK;
}

int FSMID_Close(FSMID_FHANDLE handle)
{
	int result = FSMIDR_OK;
	char pathName[FSMID_MAX_PATH];
	FSMID_FILE *pFile = (FSMID_FILE*)handle;

	fsmid_release_close(pFile);
	pFile->status = FSMIDS_NOT_OPEN;
	if(pFile->attribute & FSMIDO_CREATE_B)
	{
		fsmid_get_path_name(pFile,pathName);
		fsmid_system.callback->file_downloaded(pathName);
	}
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

	fsmid_release_close(pFile);

	result = fsmid_ulink_system(pFile);
	if(result != FSMIDR_OK)
		return result;

	fsmid_free(pFile);
	return 0;
}

int FSMID_Read(FSMID_FHANDLE handle, void* pData, unsigned int length)
{
	FSMID_FILE *pFile = (FSMID_FILE*)handle;
	unsigned char* dataPtr = (unsigned char*)pData;
	unsigned int internalOffset,copyLen;
	FSMID_ELEMENT *iterator;

	fsmid_assert(pFile && pData && length,__FUNCTION__,__LINE__);
	if(pFile->status == FSMIDS_NOT_OPEN || (pFile->attribute&FSMIDO_READ)==0)
		return FSMIDR_BAD_ARGUMENT;
	
	internalOffset = pFile->pCurrent - pFile->pBuffer;
	if(pFile->bufferOffset + internalOffset + length >= pFile->size)
		return FSMIDR_BAD_ARGUMENT;

	fsmid_mutex_lock(pFile->mutex);

	do{
		//current buffer left length.
		copyLen = min(length,pFile->bufferSize - internalOffset);
		memcpy(dataPtr,pFile->pCurrent,copyLen);
		dataPtr += copyLen;
		length -= copyLen;
		if(length == 0)
		{
			pFile->pCurrent += copyLen;
			fsmid_mutex_unlock(pFile->mutex);
			return FSMIDR_OK;
		}
		internalOffset = 0;
		iterator = fsmid_next_element(pFile);
		fsmid_assert(iterator,__FUNCTION__,__LINE__);
	}while(1);
}

#define TRY(cond,_err)		do{if(cond){result = _err; goto ErrorCondition;}}while(0)

int FSMID_Write(FSMID_FHANDLE handle, const void* pData, unsigned int length)
{
	FSMID_FILE *pFile = (FSMID_FILE*)handle;
	const unsigned char* dataPtr = (const unsigned char*)pData;
	unsigned int internalOffset,copyLen;
	FSMID_ELEMENT *iterator;

	fsmid_assert(pFile && pData && length,__FUNCTION__,__LINE__);

	if(pFile->status == FSMIDS_NOT_OPEN || (pFile->attribute&(FSMIDO_WRITE|FSMIDO_CREATE_B))!=(FSMIDO_WRITE|FSMIDO_CREATE_B))
		return FSMIDR_BAD_ARGUMENT;

	fsmid_mutex_lock(pFile->mutex);

	internalOffset = pFile->pCurrent - pFile->pBuffer;

	//current buffer left length.
	copyLen = min(length,pFile->bufferSize - internalOffset);
	memcpy(pFile->pCurrent,dataPtr,copyLen);
	dataPtr += copyLen;
	length -= copyLen;
	if(length == 0)
	{
		pFile->pCurrent += copyLen;
		fsmid_get_systime(&pFile->time);
		fsmid_mutex_unlock(pFile->mutex);
		return FSMIDR_OK;
	}
	internalOffset = 0;
	iterator = fsmid_next_element(pFile);
	if(iterator == NULL)
	{
		fsmid_assert(pFile->bufferOffset + pFile->bufferSize == pFile->size,__FUNCTION__,__LINE__);
		iterator = fsmid_malloc(FSMID_ELEMENT,1);
		if(iterator == NULL)
			goto ErrorCondition;
		iterator->size = length;
		iterator->DATA.buffer = fsmid_malloc(unsigned char,length);
		if(iterator->DATA.buffer == NULL)
		{
			fsmid_free(iterator);
			goto ErrorCondition;
		}
		list_add_tail(&iterator->_node,&pFile->headTableElement[0]);
		pFile->bufferOffset += pFile->bufferSize;
		pFile->pBuffer = iterator->DATA.buffer;
		pFile->bufferSize = length;
		memcpy(pFile->pBuffer,dataPtr,length);
		pFile->pCurrent = pFile->pBuffer + length;
		pFile->pContainer = &iterator->_node;
		fsmid_get_systime(&pFile->time);
		fsmid_mutex_unlock(pFile->mutex);
		return FSMIDR_OK;
	}

ErrorCondition:
	fsmid_mutex_unlock(pFile->mutex);
	return FSMIDR_LEAK_MEMORY;

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
		new_off = pFile->bufferOffset + offset + (unsigned int)(pFile->pCurrent - pFile->pBuffer);
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
		if(new_off < pFile->bufferOffset)
		{
			//could be optimize search from current log item
			result = fsmid_search_log(pFile, new_off);
		}
		//next portNode or more
		else if( new_off >= pFile->bufferOffset + pFile->bufferSize)
		{
			//could be optimize search from current log item
			result = fsmid_search_log(pFile, new_off);
		}
		//current node
		else
		{
			pFile->pCurrent = new_off - pFile->bufferOffset + pFile->pBuffer;
		}
	}
	fsmid_mutex_unlock(pFile->mutex);
	return 0;

ErrorCondition:
	fsmid_mutex_unlock(pFile->mutex);
	return result;
}

int FSMID_Stat(const char *pPathName, FSMID_STAT *pStat)
{
	FSMID_FILE *pFile;
	
	if(pStat == NULL)
		return FSMIDR_BAD_ARGUMENT;
	pFile = fsmid_search(pPathName);
	if(pFile == NULL)
		return FSMIDR_NOT_EXIST;

	pStat->attribute = (FSMID_OPEN_ATTR)pFile->attribute;
	pStat->status = pFile->status;
	memcpy(&pStat->time,&pFile->time,sizeof(pFile->time));
	pStat->size = pFile->size;
	pStat->offset = pFile->bufferOffset + (unsigned int)(pFile->pCurrent - pFile->pBuffer);

	return 0;
}

int FSMID_List(const char *pPath, FSMID_LIST aList[], unsigned int numList)
{
	unsigned int realNum = 0;
	unsigned int nameLength = strlen(pPath);
	struct list_head *containter;
	FSMID_FILE *iterator;

	if(pPath[nameLength - 1] == '\\')
		nameLength --;
	list_for_each(containter,&fsmid_system.headFile)
	{
		iterator = list_entry(containter,FSMID_FILE,_node);
		memset(aList[realNum].pathName,0,FSMID_MAX_PATH);
		fsmid_get_path_name(iterator,aList[realNum].pathName);
		if(memcmp(pPath,aList[realNum].pathName,nameLength) == 0 && aList[realNum].pathName[nameLength] == '\\')
		{
			memcpy(&aList[realNum].time,&iterator->time,sizeof(CP56TIME2A));
			aList[realNum].attribute,&iterator->attribute;
			aList[realNum].size,&iterator->size;
			realNum ++;
			if(realNum >= numList)
				return realNum;
		}
	}
	return realNum;	
}