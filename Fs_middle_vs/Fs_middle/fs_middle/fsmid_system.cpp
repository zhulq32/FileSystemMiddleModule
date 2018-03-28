
#include "list_linux.h"
#include "fsmid_type.h"
#include <string.h>

FSMID_SYSTEM fsmid_system;

void FSMID_Init()
{
	INIT_LIST_HEAD(&fsmid_system.headFile);
// 	fsmid_system.headFile.prev = &fsmid_system.headFile;
// 	fsmid_system.headFile.next = &fsmid_system.headFile;
}

//package path and name to one string, need release buffer after not use
static char* __get_file_path_name(FSMID_FILE* pFile)
{
	int s_size;
	char *pPathName = NULL;
	
	if(pFile->pPath)
	{
		s_size = (int)(pFile->pName - pFile->pPath) + strlen(pFile->pName);
		pPathName = fsmid_malloc(char,s_size);
		if(pPathName == NULL) return NULL;
		memcpy(pPathName,pFile->pPath,s_size);
		pPathName[strlen(pFile->pPath)] = '\\';
	}
	else
	{
		s_size = strlen(pFile->pName);
		pPathName = fsmid_malloc(char,s_size);
		if(pPathName == NULL) return NULL;
		memcpy(pPathName,pFile->pName,s_size);
	}
	return pPathName;
}

FSMID_FILE * fsmid_search( const char* pPathName)
{
	struct list_head *container;  
	FSMID_FILE *iterator;  
	char *pFilePathName;

	list_for_each(container, &fsmid_system.headFile)  
	{  

		iterator = list_entry(container, FSMID_FILE, _node);
		pFilePathName = __get_file_path_name(iterator);
		if(pFilePathName == NULL) return NULL;
		if(strcmp(pFilePathName,pPathName) == 0)
		{
			fsmid_free(pFilePathName);
			return iterator;
		}
		fsmid_free(pFilePathName);
	}
	return NULL;
}

static const char* __get_file_name( const char *pPathName)
{
	const char* p = pPathName;
	while(*pPathName)
	{
		if(*pPathName == '\\')
			p = pPathName + 1;
		pPathName++;
	}
	return p;
}

//
int fsmid_set_path_name( FSMID_FILE *pFile, const char *pPathName)
{
	unsigned int s_size;
	const char* pName;

	//file name has exist
	if(pFile->pPath || pFile->pName)
		return FSMIDR_BAD_ARGUMENT;

	pName = __get_file_name(pPathName);
	//invalid path name string
	if(pName == NULL)
		return FSMIDR_BAD_ARGUMENT;

	s_size = strlen(pPathName) + 1;
	//no path, only name
	if(pName == pPathName)
	{
		pFile->pName = fsmid_malloc(char,s_size);
		if(pFile->pName == NULL)
			return FSMIDR_LEAK_MEMORY;
		memcpy(pFile->pName,pPathName,s_size);
	}
	//have path and name
	else
	{
		pFile->pPath = fsmid_malloc(char,s_size);
		if(pFile->pPath == NULL)
			return FSMIDR_LEAK_MEMORY;
		memcpy(pFile->pPath,pPathName,s_size);
		pFile->pName = pFile->pPath + (pName - pPathName);
		pFile->pName[-1] = '\0';
	}
	return FSMIDR_OK;
}

void fsmid_get_path_name( FSMID_FILE *pFile, char *pPathName)
{
	fsmid_assert(pFile,__LINE__);
	if(pFile->pPath)
	{
		memcpy(pPathName,pFile->pPath,strlen(pFile->pName) + (unsigned int)(pFile->pName - pFile->pPath) + 1);
		pPathName[(unsigned int)(pFile->pName - pFile->pPath)] = '\\';
	}
	else
	{
		memcpy(pPathName,pFile->pName,strlen(pFile->pName) + 1);
	}
}


FSMID_FILE* fsmid_create_new(const char *pPathName, FSMID_OPEN_ATTR attribute)
{
	FSMID_FILE *pFile;

	pFile = fsmid_malloc(FSMID_FILE,1);
	if(pFile == NULL)
		return NULL;
	memset(pFile,0,sizeof(FSMID_FILE));
	pFile->attribute = attribute;

	return pFile;
}


FSMID_FILE* fsmid_open_exist(const char *pPathName, FSMID_OPEN_ATTR attribute)
{
	FSMID_FILE *pFile;

	pFile = fsmid_malloc(FSMID_FILE,1);
	if(pFile == NULL)
		return NULL;
	memset(pFile,0,sizeof(FSMID_FILE));
	pFile->attribute = attribute;

	return pFile;
}

int fsmid_ulink_system(FSMID_FILE* pFile)
{
	return 0;
}

int fsmid_release_resource(FSMID_FILE *pFile)
{
	return 0;
}


int fsmid_search_log(FSMID_FILE *pFile, unsigned int offset)
{
	struct list_head *container;  
	FSMID_LIST_LOG *iterator;  

	pFile->bufOffset = 0;
	for( pFile->currentLogGroup = 0; pFile->currentLogGroup < pFile->groupCount; pFile->currentLogGroup++ )
	{
		list_for_each(container, (pFile->headPortTable + pFile->currentLogGroup))
		{
			iterator = list_entry(container, FSMID_LIST_LOG, _node);
			if(pFile->bufOffset <= offset && pFile->bufOffset + iterator->size > offset)
			{
				pFile->pBuffer = fsmid_malloc(unsigned char,iterator->size);
				if(pFile->pBuffer == NULL)
					return FSMIDR_LEAK_MEMORY;
				pFile->bufSize = iterator->size;
				pFile->pCurrent = pFile->pBuffer + offset - pFile->bufOffset;
				pFile->pContainer = container;
				return ___fsmid_get_data(pFile,iterator->handle);
			}
			pFile->bufOffset += iterator->size;
		}   
	}      
	return FSMIDR_GENERAL;
}

FSMID_LIST_LOG* fsmid_next_log(FSMID_FILE *pFile)
{
	FSMID_LIST_LOG* iterator;

	pFile->pContainer = pFile->pContainer->next;
	if(pFile->pContainer != &pFile->headPortTable[pFile->currentLogGroup])
	{
		iterator = list_entry(pFile->pContainer, FSMID_LIST_LOG, _node);
		if(iterator == NULL) return NULL;
		goto EOFunc;
	}
	pFile->currentLogGroup ++;
	if(pFile->currentLogGroup >= pFile->groupCount)
		return NULL;
	pFile->pContainer = pFile->headPortTable[pFile->currentLogGroup].next;
	iterator = list_entry(pFile->pContainer, FSMID_LIST_LOG, _node);
	if(iterator == NULL) return NULL;
EOFunc:
	if(pFile->pBuffer && iterator->size != pFile->bufSize)
	{
		fsmid_free(pFile->pBuffer);
		pFile->pBuffer = NULL;
	}
	if(pFile->pBuffer == NULL)
	{
		pFile->pBuffer = fsmid_malloc(unsigned char,iterator->size);
		fsmid_assert(pFile->pBuffer,__LINE__);
	}
	pFile->bufSize = iterator->size;
	pFile->pCurrent = pFile->pBuffer;
	fsmid_assert(___fsmid_get_data(pFile,iterator->handle)==0,__LINE__);
	return iterator;
}

FSMID_LIST_LOG* fsmid_prev_log(FSMID_FILE *pFile)
{
	FSMID_LIST_LOG* iterator;

	pFile->pContainer = pFile->pContainer->prev;
	if(pFile->pContainer != &pFile->headPortTable[pFile->currentLogGroup])
	{
		iterator = list_entry(pFile->pContainer, FSMID_LIST_LOG, _node);
		if(iterator == NULL) return NULL;
		goto EOFunc;
	}
	if(pFile->currentLogGroup == 0)
		return NULL;
	pFile->currentLogGroup --;
	pFile->pContainer = pFile->headPortTable[pFile->currentLogGroup].prev;
	iterator = list_entry(pFile->pContainer, FSMID_LIST_LOG, _node);
	if(iterator == NULL) return NULL;
EOFunc:
	if(pFile->pBuffer && iterator->size != pFile->bufSize)
	{
		fsmid_free(pFile->pBuffer);
		pFile->pBuffer = NULL;
	}
	if(pFile->pBuffer == NULL)
	{
		pFile->pBuffer = fsmid_malloc(unsigned char,iterator->size);
		fsmid_assert(pFile->pBuffer,__LINE__);
	}
	pFile->bufSize = iterator->size;
	pFile->pCurrent = pFile->pBuffer;
	fsmid_assert(___fsmid_get_data(pFile,iterator->handle)==0,__LINE__);
	return iterator;
}

