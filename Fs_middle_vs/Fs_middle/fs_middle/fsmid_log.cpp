
#include "list_linux.h"
#include "fsmid_type.h"

extern FSMID_SYSTEM fsmid_system;

int FSMID_Register(FSMID_FHANDLE fileHandle, unsigned int groupCount)
{
	unsigned int i;
	FSMID_FILE *pFile = (FSMID_FILE*)fileHandle;

	if(pFile->status != FSMIDS_OPENED)
		return FSMIDR_ACCESS;
	if((pFile->attribute & (FSMIDO_WRITE|FSMID_CREATE_MASK)) != (FSMIDO_CREATE_T|FSMIDO_WRITE))
		return FSMIDR_ACCESS;
	if(pFile->groupCount || pFile->headTableElement)
		return FSMIDR_CONFLICT;

	fsmid_mutex_lock(pFile->mutex);

	pFile->groupCount = groupCount;
	pFile->headTableElement = fsmid_malloc(struct list_head,groupCount);
	for(i = 0; i < groupCount; i++)
		INIT_LIST_HEAD(pFile->headTableElement + i);

	fsmid_mutex_unlock(pFile->mutex);
	return 0;
}

int FSMID_Push(FSMID_FHANDLE fileHandle, FSMID_LOG_HANDLE logHandle, unsigned int groupIndex)
{
	FSMID_FILE *pFile = (FSMID_FILE*)fileHandle;
	FSMID_ELEMENT *pLog;

	if(pFile->status != FSMIDS_OPENED)
		return FSMIDR_ACCESS;
	if((pFile->attribute & (FSMIDO_WRITE|FSMID_CREATE_MASK)) != (FSMIDO_CREATE_T|FSMIDO_WRITE))
		return FSMIDR_ACCESS;
	if( !pFile->groupCount || !pFile->headTableElement)
		return FSMIDR_CONFLICT;
	if(groupIndex >= pFile->groupCount)
		return FSMIDR_NOT_EXIST;
	
	pLog = fsmid_malloc(FSMID_ELEMENT,1);
	if(pLog == NULL)
		return FSMIDR_LEAK_MEMORY;

	fsmid_mutex_lock(pFile->mutex);

	pLog->DATA.handle = logHandle;
	pLog->size = fsmid_system.callback->get_length(logHandle);
	list_add_tail(&pLog->_node,pFile->headTableElement + groupIndex);
	fsmid_system.callback->get_time(logHandle,&pFile->time);
	pFile->size += pLog->size;

	fsmid_mutex_unlock(pFile->mutex);

	return 0;
}

int FSMID_Pop(FSMID_FHANDLE fileHandle, FSMID_LOG_HANDLE logHandle)
{
	FSMID_FILE *pFile = (FSMID_FILE*)fileHandle;
	unsigned int group;
	struct list_head *container;
	FSMID_ELEMENT *iterator;

	if(pFile->status != FSMIDS_OPENED)
		return FSMIDR_ACCESS;
	if((pFile->attribute & (FSMIDO_WRITE|FSMID_CREATE_MASK)) != (FSMIDO_CREATE_T|FSMIDO_WRITE))
		return FSMIDR_ACCESS;
	if( !pFile->groupCount || !pFile->headTableElement)
		return FSMIDR_CONFLICT;


	fsmid_mutex_lock(pFile->mutex);
	
	for( group = 0; group < pFile->groupCount; group++ )
	{
		list_for_each(container,pFile->headTableElement + group)
		{
			iterator = list_entry(container, FSMID_ELEMENT, _node);
			if(iterator->DATA.handle == logHandle)
			{
				pFile->size -= iterator->size;
				fsmid_get_systime(&pFile->time);
				list_del(container);
				fsmid_mutex_unlock(pFile->mutex);
				return FSMIDR_OK;
			}
		}
	}
	

	fsmid_mutex_unlock(pFile->mutex);

	return FSMIDR_NOT_EXIST;
}
