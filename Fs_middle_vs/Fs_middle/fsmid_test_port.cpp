
#include "fsmid_inc.h"

unsigned int testGetLength(FSMID_LOG_HANDLE logHandle)
{
	return 0;
}

void testGetTime(FSMID_LOG_HANDLE logHandle, CP56TIME2A *pTime)
{

}

void testGetData(FSMID_LOG_HANDLE logHandle, void* pData)
{

}

//int testPutData(FSMID_LOG_HANDLE logHandle, unsigned int offset, const void* pData, unsigned int length)
// {
// 
// }

void testFileDownload( const char *pPathName)
{

}

struct __fsmid_callback testCallback = {
	testGetLength,
	testGetTime,
	testGetData,
	//testPutData,
	testFileDownload,
};

 char *const csAccessTable[] = {
	"  ","RO","WO","RW",
};

void printFileInfo(FSMID_FHANDLE handle)
{
	unsigned int i,j;
	struct list_head *container;
	FSMID_ELEMENT *iterator;
	FSMID_FILE *pFile = (FSMID_FILE*)handle;
	if(handle == FSMID_INVALID_HANDLER)
	{
		printf("\n[!ERROR!] invalid file handle 0x%08X.\n\n",handle);
		return;
	}
	if(pFile->pPath)
		printf("----------[PATH:%s]-[NAME:%s]----------\n",pFile->pPath,pFile->pName);
	else
		printf("----------[PATH:N/A]-[NAME:%s]----------\n",pFile->pName);
	printf(" Size:%6dBytes.     Date:%4d-%2d-%2d %2d:%2d:%2.3f.\n",
		pFile->size,
		pFile->time.year,pFile->time.month,pFile->time.monthDay,
		pFile->time.hour,pFile->time.minute,(float)pFile->time.milliSecond/1000.0);
	printf(" Status:%s,    Type:%s,    Access:%d \n",
		(pFile->status==FSMIDS_OPENED)?"Opened":"Closed",
		(pFile->attribute&FSMIDO_CREATE_B)?"Binary":"Text  ",
		csAccessTable[pFile->attribute&FSMID_ACCESS_MASK]);

	if(pFile->size == 0) return;
	printf(" list table:\n");
	for( i = 0; i < pFile->groupCount; i++ )
	{
		j = 0;
		printf("[[%3d]:");
		list_for_each(container,pFile->headTableElement + i)
		{
			iterator = list_entry(container,FSMID_ELEMENT,_node);
			if(container == pFile->pContainer)
				printf("*");
			printf("[%d|%dBytes 0%X]=>",i,j,iterator->size,(unsigned int)(iterator->DATA.buffer));
			j++;
		}
		printf("\n");
	}
	printf("------------------------------------------------------\n\n");
}

void printFileContent(FSMID_FHANDLE handle)
{
	unsigned int i,j;
	unsigned char* data;
	char str[FSMID_SAFTY_BUFFER_SIZE + 1];
	struct list_head *container;
	FSMID_ELEMENT *iterator;
	FSMID_FILE *pFile = (FSMID_FILE*)handle;
	if(handle == FSMID_INVALID_HANDLER)
	{
		printf("\n[!ERROR!] invalid file handle 0x%08X.\n\n",handle);
		return;
	}
	if(pFile->size == 0)
	{
		printf("\n[!ERROR!] file length is 0.\n\n");
		return;
	}
	if(pFile->pPath)
		printf("----------[PATH:%s]-[NAME:%s]----------\n",pFile->pPath,pFile->pName);
	else
		printf("----------[PATH:N/A]-[NAME:%s]----------\n",pFile->pName);
	printf(" Size:%6dBytes.     Date:%4d-%2d-%2d %2d:%2d:%2.3f.\n",
		pFile->size,
		pFile->time.year,pFile->time.month,pFile->time.monthDay,
		pFile->time.hour,pFile->time.minute,(float)pFile->time.milliSecond/1000.0);
	printf(" Status:%s,    Type:%s,    Access:%d \n",
		(pFile->status==FSMIDS_OPENED)?"Opened":"Closed",
		(pFile->attribute&FSMIDO_CREATE_B)?"Binary":"Text  ",
		csAccessTable[pFile->attribute&FSMID_ACCESS_MASK]);

	printf(" Content:\n");
	if(pFile->attribute&FSMIDO_CREATE_B)
	{
		printf("ADDRESS:");
		i = 0;
		for(j = 0; j < 16; j++)
			printf(" %02X",j);
		list_for_each(container,pFile->headTableElement)
		{
			iterator = list_entry(container,FSMID_ELEMENT,_node);
			data = iterator->DATA.buffer;
			for(j = 0; j <iterator->size; j++,data++,i++)
			{
				if(i%16 == 0)
					printf("\n%08X",i);
				printf(" %02X",*data);
			}
		}
	}
	else
	{
		for( i = 0; i < pFile->groupCount; i++ )
		{
			list_for_each(container,pFile->headTableElement + i)
			{
				iterator = list_entry(container,FSMID_ELEMENT,_node);
				testGetData(iterator->DATA.handle,str);
				str[iterator->size] = '\0';
				sprintf("%s",str);
			}
		}
	}
	printf("------------------------------------------------------\n\n");
}

