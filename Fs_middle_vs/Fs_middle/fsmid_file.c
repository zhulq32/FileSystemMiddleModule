
#include "fsmid_def.h"

int fsmid_open(FSMID_FHANDLE* pHandler, const char* pName, FSMID_CREATE_ATTR attribute)
{
	return 0;
}

int fsmid_close(FSMID_FHANDLE handle)
{
	return 0;
}

int fsmid_remove(const char* pName)
{
	return 0;
}

int fsmid_read(FSMID_FHANDLE handle, void* pData, unsigned int length)
{
	return 0;
}

int fsmid_write(FSMID_FHANDLE handle, const void* pData, unsigned int length)
{
	return 0;
}

int fsmid_seek(FSMID_FHANDLE handle, unsigned int offset, FSMID_SEEK method)
{
	return 0;
}

int fsmid_stat(FSMID_FHANDLE handle, FSMID_STAT *pStat)
{
	return 0;
}