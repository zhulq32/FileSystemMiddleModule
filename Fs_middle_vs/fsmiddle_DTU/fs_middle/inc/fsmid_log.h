#ifndef _FSMID_LOG_H_
#define _FSMID_LOG_H_

#include "sysTimerV1.h"//for SYSTIME_64

#include "list_linux.h"
#define FSLOG_NO_EVENT				0
#define FSLOG_EVENT_WRITE			0x01
#define FSLOG_EVENT_FULL			0x02

#define FSLOG_ATTR_OTP				(1<<31)
#define FSLOG_ATTR_AVAILABLE		(1<<30)

#define FSLOG_OPEN_MASK				(3<<26)
#define FSLOG_ATTR_OPEN_EXIST		(1<<27)
#define FSLOG_ATTR_CREATE_ALWAYS	(2<<26)
#define FSLOG_ATTR_OPEN_CREATE		(3<<26)


#ifndef __cplusplus
extern "C" {
#endif


#pragma pack(push,1)


typedef struct __fslog_interface{
	int (*write)(unsigned int address, const void* data, unsigned int length);
	int (*read)(unsigned int address, void* data, unsigned int length);
	int (*erase)(unsigned int address, unsigned int length);
}FSLOG_INTERFACE;


typedef struct __fslog_function{
	int (*format_header)(char *buf, struct __fslog* log);
	int (*format_data)(char *buf, const void* data);
	const SYS_TIME64 *(*time)(const void *data);
}FSLOG_FUNCTION;

typedef struct __fslog_information{
	unsigned int baseAddress;
	unsigned int blockNumber;
	//based on erase function
	unsigned int blockSize;

	unsigned int unitSize;
	unsigned int unitCount;
}FSLOG_INFORMATION;

// typedef struct __struct_log_node{
// 	char name[64];
// 	FSLOG_INFORMATION information;
// 	unsigned int timeUnix;
// 	unsigned int attribute;
// }FSLOG_NODE;

typedef struct __fslog{
	char name[64];
	const FSLOG_FUNCTION  *pFunction;
	const FSLOG_INFORMATION *pInformation;
	unsigned int timeCreateUnix;

	FSMID_MUTEX mutex;
	struct list_head _node;
	unsigned int attribute;

	//read only after initialize
	unsigned int formatedSize;
	unsigned int unitPerBlock;
	unsigned int maxUnitCount;

	//dynamic whole life cycle
	unsigned int pointerId;
	unsigned int unitNumber;
	unsigned int indexFirst;
	unsigned int indexLast;
	//unsigned int indexRead;
}FSLOG;

void FSLOG_Init( const FSLOG_INTERFACE *pInterface);

// int FSLOG_Rename(FSLOG *pLog, const char *pName);

//create initialize FSLOG object
FSLOG* FSLOG_Open( const char* pName, const FSLOG_FUNCTION * pFunction, const FSLOG_INFORMATION *pInformation, unsigned int attribute);

//write one data unit into FSLOG object's tail.
int FSLOG_Write(FSLOG *pLog, const void* log);
int FSLOG_LockWrite(FSLOG *pLog, const void* log);

#define FSLOG_Lock(pLog)		fsmid_mutex_lock((pLog)->mutex)
#define FSLOG_Unlock(pLog)		fsmid_mutex_unlock((pLog)->mutex)

//read one data unit from FSLOG object's current itemIndex.
int FSLOG_ReadData(FSLOG *pLog, unsigned int index, void* data);
int FSLOG_LockReadData(FSLOG *pLog, unsigned int index, void* data);

int FSLOG_ReadFmt(FSLOG *pLog, unsigned int index, char *buf);
int FSLOG_LockReadFmt(FSLOG *pLog, unsigned int index, char *buf);

//remove all data unit in the FSLOG object.
int FSLOG_Clear(FSLOG *pLog);

//int FSLOG_Release(FSLOG *pLog);

// FSLOG *FSLOG_GetEvent();

unsigned int FSLOG_GetPointerIndex(FSLOG *pLog, unsigned int index);

//get FSLOG object's data unit size.
#define FSLOG_GetUnitSize(plog)		((plog)->pInformation->unitSize)

//get FSLOG object's available data count.
#define FSLOG_GetUnitCount(plog)			((plog)->unitNumber)

#define FSLOG_GetPathName(plog)				((plog)->name)

const char* FSLOG_GetName(FSLOG* pLog);

void FSLOG_ReleaseFilter();

unsigned int FSLOG_Filter(const char *pCondition);

unsigned int FSLOG_GetFiltedCount();

FSLOG *FSLOG_GetFiltedItem(unsigned int index);


#pragma pack(pop)

#ifndef __cplusplus
};
#endif

#endif
