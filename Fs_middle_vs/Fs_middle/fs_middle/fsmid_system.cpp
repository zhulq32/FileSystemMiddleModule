
#include "list_linux.h"
#include "fsmid_type.h"
#include "fsmid_system.h"
#include <string.h>

FSMID_SYSTEM fsmid_system;

void FSMID_Init(const struct __fsmid_callback *pCallback)
{
	INIT_LIST_HEAD(&fsmid_system.headFile);
// 	fsmid_system.headFile.prev = &fsmid_system.headFile;
// 	fsmid_system.headFile.next = &fsmid_system.headFile;

	fsmid_system.callback = pCallback;
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
	fsmid_assert(pFile,__FUNCTION__,__LINE__);
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

static void __fsmid_clear_element(FSMID_FILE *pFile)
{
	struct list_head *container;
	FSMID_ELEMENT *iterator;

	if(pFile->pPath)
		fsmid_free(pFile->pPath);
	else if(pFile->pName)
		fsmid_free(pFile->pName);

	for( pFile->currentLogGroup = 0; pFile->currentLogGroup < pFile->groupCount; pFile->currentLogGroup++ )
	{
		list_for_each(container, (pFile->headTableElement + pFile->currentLogGroup))
		{
			if(pFile->attribute&FSMIDO_CREATE_B)
			{
				iterator = list_entry(container, FSMID_ELEMENT, _node);
				fsmid_free(iterator->DATA.buffer);
			}
			list_del(container);
		}
	}
	pFile->size = 0;
	if(pFile->attribute&FSMIDO_CREATE_T)
		fsmid_free(pFile->pBuffer);
	pFile->pBuffer = NULL;
	pFile->pCurrent = NULL;
}


int fsmid_create_new(FSMID_FILE **ppFile, const char *pPathName, FSMID_OPEN_ATTR attribute)
{
	int result;
	FSMID_FILE *pFile = NULL;

	if((attribute&FSMID_CREATE_MASK) == FSMIDO_CREATE_INVALID||(attribute&FSMID_CREATE_MASK) == FSMIDO_CREATE_NONE)
		return FSMIDR_BAD_ARGUMENT;

	pFile = fsmid_malloc(FSMID_FILE,1);
	if(pFile == NULL)
		return FSMIDR_LEAK_MEMORY;

	memset(pFile,0,sizeof(FSMID_FILE));
	pFile->attribute = attribute;

	fsmid_mutex_create(pFile->mutex);
	result = fsmid_set_path_name(pFile,pPathName);
	if(result != FSMIDR_OK) goto ErrorCondition;
	fsmid_get_systime(&pFile->time);
	//binary file
	if(attribute & FSMIDO_CREATE_B)
	{
		pFile->headTableElement = fsmid_malloc(struct list_head,1);
		fsmid_assert(pFile->headTableElement,__FUNCTION__,__LINE__);
		pFile->groupCount = 1;
	}
	*ppFile = pFile;
	list_add_tail(&pFile->_node,&fsmid_system.headFile);
	return FSMIDR_OK;
ErrorCondition:
	if(pFile)
	{
		__fsmid_clear_element(pFile);
		fsmid_release_close(pFile);
		fsmid_mutex_release(pFile->mutex);
		fsmid_free(pFile);
	}
	return result;
}

void __fsmid_get_element_data(FSMID_FILE *pFile, FSMID_ELEMENT *iterator)
{
	if(pFile->attribute & FSMIDO_CREATE_B)
	{
		pFile->pBuffer = iterator->DATA.buffer;
	}
	else
	{
		if(pFile->pBuffer && pFile->bufferSize < iterator->size)
		{
			fsmid_free(pFile->pBuffer);
			pFile->pBuffer = fsmid_malloc(unsigned char,iterator->size);
			fsmid_assert(pFile->pBuffer,__FUNCTION__,__LINE__);
			pFile->bufferSize = iterator->size;
		}
		fsmid_system.callback->get_data(iterator->DATA.handle,pFile->pBuffer);
	}
	pFile->pCurrent = pFile->pBuffer;
}


int fsmid_init_exist(FSMID_FILE *pFile)
{
	FSMID_ELEMENT *iterator;
	if(pFile->groupCount && pFile->size)
	{
		pFile->pContainer = pFile->headTableElement[0].next;
		iterator = list_first_entry(pFile->pContainer,FSMID_ELEMENT,_node);
		__fsmid_get_element_data(pFile,iterator);
		pFile->bufferOffset = 0;
		pFile->currentLogGroup = 0;
	}
	return 0;
}

void fsmid_release_close(FSMID_FILE *pFile)
{
	if((pFile->attribute & FSMIDO_CREATE_T) && pFile->pBuffer)
	{
		fsmid_free(pFile->pBuffer);
		pFile->pBuffer = NULL;
		pFile->bufferSize = 0;
		pFile->pCurrent = NULL;
		pFile->bufferOffset = 0;
		pFile->pContainer = NULL;
		pFile->currentLogGroup = 0;
	}
}

int fsmid_ulink_system(FSMID_FILE* pFile)
{
	struct list_head *container;
	FSMID_FILE* iterator;

	if(pFile->status == FSMIDS_OPENED)
		return FSMIDR_ACCESS;
	pFile->status = FSMIDS_OPENED;

	list_for_each(container,&fsmid_system.headFile)
	{
		iterator = list_entry(container,FSMID_FILE,_node);
		if(iterator == pFile)
		{
			list_del(container);
			__fsmid_clear_element(pFile);
			fsmid_mutex_release(pFile->mutex);
			fsmid_free(pFile);
			return FSMIDR_OK;
		}
	}

	return FSMIDR_NOT_EXIST;
}

int fsmid_search_log(FSMID_FILE *pFile, unsigned int offset)
{
	struct list_head *container;  
	FSMID_ELEMENT *iterator;  

	pFile->bufferOffset = 0;
	for( pFile->currentLogGroup = 0; pFile->currentLogGroup < pFile->groupCount; pFile->currentLogGroup++ )
	{
		list_for_each(container, (pFile->headTableElement + pFile->currentLogGroup))
		{
			iterator = list_entry(container, FSMID_ELEMENT, _node);
			if(pFile->bufferOffset <= offset && pFile->bufferOffset + iterator->size > offset)
			{
				if(pFile->attribute & FSMIDO_CREATE_B)
					pFile->pBuffer = iterator->DATA.buffer;
				else
					fsmid_system.callback->get_data(iterator->DATA.handle,pFile->pBuffer);
				pFile->bufferSize = iterator->size;
				pFile->pCurrent = pFile->pBuffer + offset - pFile->bufferOffset;
				pFile->pContainer = container;
				return FSMIDR_OK;
			}
			pFile->bufferOffset += iterator->size;
		}   
	}      
	return FSMIDR_GENERAL;
}

FSMID_ELEMENT* fsmid_next_element(FSMID_FILE *pFile)
{
	FSMID_ELEMENT* iterator;

	pFile->pContainer = pFile->pContainer->next;
	if(pFile->pContainer != &pFile->headTableElement[pFile->currentLogGroup])
	{
		iterator = list_entry(pFile->pContainer, FSMID_ELEMENT, _node);
		if(iterator == NULL) return NULL;
		goto EOFunc;
	}
	pFile->currentLogGroup ++;
	if(pFile->currentLogGroup >= pFile->groupCount)
		return NULL;
	pFile->pContainer = pFile->headTableElement[pFile->currentLogGroup].next;
	iterator = list_entry(pFile->pContainer, FSMID_ELEMENT, _node);
	if(iterator == NULL) return NULL;
EOFunc:
	pFile->bufferOffset += pFile->bufferSize;
	if(pFile->attribute & FSMIDO_CREATE_B)
		pFile->pBuffer = iterator->DATA.buffer;
	else
		fsmid_system.callback->get_data(iterator->DATA.handle,pFile->pBuffer);
	pFile->bufferSize = iterator->size;
	pFile->pCurrent = pFile->pBuffer;
	return iterator;
}

FSMID_ELEMENT* fsmid_prev_element(FSMID_FILE *pFile)
{
	FSMID_ELEMENT* iterator;

	pFile->pContainer = pFile->pContainer->prev;
	if(pFile->pContainer != &pFile->headTableElement[pFile->currentLogGroup])
	{
		iterator = list_entry(pFile->pContainer, FSMID_ELEMENT, _node);
		if(iterator == NULL) return NULL;
		goto EOFunc;
	}
	if(pFile->currentLogGroup == 0)
		return NULL;
	pFile->currentLogGroup --;
	pFile->pContainer = pFile->headTableElement[pFile->currentLogGroup].prev;
	iterator = list_entry(pFile->pContainer, FSMID_ELEMENT, _node);
	if(iterator == NULL) return NULL;
EOFunc:
	if(pFile->attribute & FSMIDO_CREATE_B)
		pFile->pBuffer = iterator->DATA.buffer;
	else
		fsmid_system.callback->get_data(iterator->DATA.handle,pFile->pBuffer);
	pFile->bufferSize = iterator->size;
	pFile->pCurrent = pFile->pBuffer;
	pFile->bufferOffset -= pFile->bufferSize;
	return iterator;
}

