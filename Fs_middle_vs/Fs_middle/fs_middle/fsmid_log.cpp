
#include "list_linux.h"
#include "fsmid_type.h"

int FSMID_Register(FSMID_FHANDLE fileHandle, const FSPORT_FUNCTION *arrayFunction, unsigned int groupCount)
{
	int i;
	FSMID_FILE *pFile = (FSMID_FILE*)fileHandle;

	if(pFile->status != FSMIDS_OPENED)
		return FSMIDR_ACCESS;
	if((pFile->attribute & FSMIDO_PUSHPOP) == 0)
		return FSMIDR_ACCESS;
	if(pFile->pFunctionTable || pFile->groupCount || pFile->headPortTable)
		return FSMIDR_CONFLICT;

	fsmid_mutex_lock(pFile->mutex);

	pFile->pFunctionTable = arrayFunction;
	pFile->groupCount = groupCount;
	pFile->headPortTable = fsmid_malloc(struct list_head,groupCount);
	for(i = 0; i < groupCount; i++)
		INIT_LIST_HEAD(pFile->headPortTable + i);

	fsmid_mutex_unlock(pFile->mutex);
	return 0;
}

int FSMID_Push(FSMID_FHANDLE fileHandle, FSMID_LOG_HANDLE logHandle, unsigned int groupIndex)
{
	FSMID_FILE *pFile = (FSMID_FILE*)fileHandle;
	FSMID_LIST_LOG *pLog;

	if(pFile->status != FSMIDS_OPENED)
		return FSMIDR_ACCESS;
	if((pFile->attribute & FSMIDO_PUSHPOP) == 0)
		return FSMIDR_ACCESS;
	if( !pFile->pFunctionTable || !pFile->groupCount || !pFile->headPortTable)
		return FSMIDR_CONFLICT;
	if(groupIndex >= pFile->groupCount)
		return FSMIDR_NOT_EXIST;
	
	pLog = fsmid_malloc(FSMID_LIST_LOG,1);
	if(pLog == NULL)
		return FSMIDR_LEAK_MEMORY;

	fsmid_mutex_lock(pFile->mutex);

	pLog->handle = logHandle;
	pLog->size = pFile->pFunctionTable[groupIndex].get_length(logHandle);
	list_add_tail(&pLog->_node,pFile->headPortTable + groupIndex);
	pFile->pFunctionTable[groupIndex].get_time(logHandle,&pFile->time);
	pFile->size += pLog->size;

	fsmid_mutex_unlock(pFile->mutex);

	return 0;
}

int FSMID_Pop(FSMID_FHANDLE fileHandle, FSMID_LOG_HANDLE logHandle)
{
	FSMID_FILE *pFile = (FSMID_FILE*)fileHandle;
	int group;
	struct list_head *container;
	FSMID_LIST_LOG *iterator;

	if(pFile->status != FSMIDS_OPENED)
		return FSMIDR_ACCESS;
	if((pFile->attribute & FSMIDO_PUSHPOP) == 0)
		return FSMIDR_ACCESS;
	if( !pFile->pFunctionTable || !pFile->groupCount || !pFile->headPortTable)
		return FSMIDR_CONFLICT;


	fsmid_mutex_lock(pFile->mutex);
	
	for( group = 0; group < pFile->groupCount; group++ )
	{
		list_for_each(container,pFile->headPortTable + group)
		{
			iterator = list_entry(container, FSMID_LIST_LOG, _node);
			if(iterator->handle == logHandle)
			{
				pFile->size -= iterator->size;
				pFile->pFunctionTable[group].get_time(FSMID_INVALID_LOG_HANDLER,&pFile->time);
				list_del(container);
				fsmid_mutex_unlock(pFile->mutex);
				return FSMIDR_OK;
			}
		}
	}
	

	fsmid_mutex_unlock(pFile->mutex);

	return FSMIDR_NOT_EXIST;
}
