
#include "fsmid_inc.h"

unsigned int testGetLength(FSMID_LOG_HANDLE logHandle);
void testGetTime(FSMID_LOG_HANDLE logHandle, CP56TIME2A *pTime);
void testGetData(FSMID_LOG_HANDLE logHandle, void* pData);
//int testPutData(FSMID_LOG_HANDLE logHandle, unsigned int offset, const void* pData, unsigned int length);
void testFileDownload( const char *pPathName);

void printFileInfo(FSMID_FHANDLE handle);
void printFileContent(FSMID_FHANDLE handle);

extern struct __fsmid_callback testCallback;

void testFileCreate()
{

}

//define all functions from here
void fsmid_test_task( void * pParam)
{
	FSMID_Init(&testCallback);
	while(1)
	{

	}
}