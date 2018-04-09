
#include "fsmid_def.h"
#include "fsmid_log.h"
#include <string.h>

#undef  _FSLOG_INFO_MSG_
//#define _FSLOG_INFO_MSG_
#if defined(_FSLOG_INFO_MSG_)
#define FSLOG_INFO_MSG(fmt,...)       fsmid_info(fmt, ##__VA_ARGS__)
#else
#define FSLOG_INFO_MSG(fmt,...)
#endif  //#if defined(_FSLOG_INFO_MSG_)

static struct list_head headMainLog;
static struct list_head headFiltedLog;
static unsigned int nFiltedLog = 0;
static const FSLOG_INTERFACE *logInterface = NULL;

#pragma pack(push,1)

//-----------------------------------internal structure definition----------------------------------------
#define FSLOG_HEADER_SIZE		64

typedef struct __fslog_header{
	unsigned short signature;
	unsigned short flag;
	unsigned int pointerId;
	unsigned char counter[FSLOG_HEADER_SIZE - 8];
}FSLOG_HEADER;

#define FSLOG_UNIT_PERBLOCK			(sizeof(((FSLOG_HEADER*)0)->counter) * 8)

#define FSLOG_HEADER_SIGNATURE	0x5346//'F' + 'S'
#define FSLOG_FFFF_SIGNATURE	0xFFFF
#define FSLOG_FLAG_ALL			0xFFFF

#define FSLOG_BIT_MASK			(3<<0)
#define FSLOG_BIT_EMPTY			(3<<0)
#define FSLOG_BIT_FILLING		(2<<0)
#define FSLOG_BIT_FULL			(0<<0)

#pragma pack(pop)

#define log_read(a,p,l)			logInterface->read(a,p,l)
#define log_write(a,p,l)		logInterface->write(a,p,l)
#define log_erase(a,l)			logInterface->erase(a,l)


#define __fslog_index2address(index)		(pLog->pInformation->baseAddress + \
											(index)/pLog->unitPerBlock * pLog->pInformation->blockSize + \
											FSLOG_HEADER_SIZE + \
											(index)%pLog->unitPerBlock * pLog->pInformation->unitSize)

//erase block, init block header.
static int __fslog_init_block(FSLOG* pLog, unsigned int blockIndex, unsigned short flag)
{
	FSLOG_HEADER header;
	unsigned int address = pLog->pInformation->blockSize * blockIndex + pLog->pInformation->baseAddress;

	fsmid_assert(!log_erase(address,pLog->pInformation->blockSize),__FILE__,__LINE__);

	header.signature = FSLOG_HEADER_SIGNATURE;
	header.flag = flag;
	header.pointerId = (flag==FSLOG_FLAG_ALL)?(-1UL):pLog->pointerId;
	number2bitmap(0,header.counter,sizeof(header.counter));
	fsmid_assert(!log_write(address,&header,sizeof(header)),__FILE__,__LINE__);

	return 0;
}

//init all pointers with flash data
static int __fslog_init(FSLOG *pLog)
{
	//unsigned int result;
	FSLOG_HEADER header;
	unsigned int i,address;

	
	for( i = 0,address = pLog->pInformation->baseAddress; i < pLog->pInformation->blockNumber; i++, address += pLog->pInformation->blockSize)
	{
		//address = pLog->pInformation->blockSize * i + pLog->pInformation->baseAddress;
		fsmid_assert(!log_read(address,&header,sizeof(header)),__FILE__,__LINE__);
		//valid header
		if(header.signature == FSLOG_HEADER_SIGNATURE)
		{
			if((header.flag & FSLOG_BIT_MASK) == FSLOG_BIT_FILLING)
			{
				//write pointer can't init twice.
				fsmid_assert(pLog->indexLast == (-1UL),__FILE__,__LINE__);
				pLog->indexLast = i * pLog->unitPerBlock + bitmap2number(header.counter,sizeof(header.counter));
				pLog->pointerId = header.pointerId;
				pLog->unitNumber += bitmap2number(header.counter,sizeof(header.counter));
				if(pLog->indexFirst == (-1UL))
				{
					fsmid_assert( i == 0,__FILE__,__LINE__);
					pLog->indexFirst = i * pLog->unitPerBlock;
				}
					//data might be wrapped.
				if(++i == pLog->pInformation->blockNumber)
					return 0;
				address += pLog->pInformation->blockSize;
				fsmid_assert(!log_read(address,&header,sizeof(header)),__FILE__,__LINE__);
				if(header.signature == FSLOG_HEADER_SIGNATURE)
				{
					if((header.flag & FSLOG_BIT_MASK) == FSLOG_BIT_FULL)
					{
						pLog->indexFirst = i * pLog->unitPerBlock;
						pLog->unitNumber = pLog->unitNumber%pLog->unitPerBlock + pLog->unitPerBlock*(pLog->pInformation->blockNumber - 1);
					}
				}
				return 0;
			}
			else if((header.flag & FSLOG_BIT_MASK) == FSLOG_BIT_FULL)
			{
				//oldest pointer not inited.
				if(pLog->indexFirst == (-1UL))
				{
					//should be inited in block0. 
					fsmid_assert( i == 0,__FILE__,__LINE__);
					pLog->indexFirst = i * pLog->unitPerBlock;

				}
				pLog->unitNumber += pLog->unitPerBlock;
			}
			else
				fsmid_assert(0,__FILE__,__LINE__);
		}
		else if(header.signature == FSLOG_FFFF_SIGNATURE)
		{
			if(i)
				fsmid_warning("!invalid signature",__FILE__,__LINE__);
			__fslog_init_block(pLog,i,FSLOG_BIT_FILLING);
			pLog->unitNumber = i * pLog->unitPerBlock;
			pLog->indexFirst = pLog->unitNumber;
			pLog->indexLast = pLog->unitNumber;
			pLog->pointerId = pLog->unitNumber;
			return 0;
		}
		else
			fsmid_assert(0,__FILE__,__LINE__);
	}
	fsmid_assert(0,__FILE__,__LINE__);
}

//return to new block
static int __fslog_plus_write_counter(FSLOG* pLog)
{
	FSLOG_HEADER header;
	unsigned int num = (pLog->indexLast + 1)%pLog->unitPerBlock;
	unsigned int address = pLog->indexLast/pLog->unitPerBlock * pLog->pInformation->blockSize + pLog->pInformation->baseAddress;

	header.signature = FSLOG_HEADER_SIGNATURE;
	header.flag = (num)?FSLOG_BIT_FILLING:FSLOG_BIT_FULL;
	header.pointerId = pLog->pointerId;
	number2bitmap((num?num:pLog->unitPerBlock),header.counter,sizeof(header.counter));
	fsmid_assert(!log_write(address,&header,sizeof(header)),__FILE__,__LINE__);
	pLog->unitNumber++;
	pLog->indexLast++;
	if(!num)
	{
		if(pLog->indexLast >= pLog->maxUnitCount)
			pLog->indexLast = 0;
		pLog->pointerId++;
		return 1;
	}
	return 0;
}

//-----------------------------------external function definition-----------------------------------------

// static const FSLOG_INFORMATION logInfoSys = {
// 	4<<10,
// 	1,
// 	12<<10,
// 	sizeof(FSLOG_NODE),
// 	0
// };

void FSLOG_Init(const FSLOG_INTERFACE *pInterface)
{
	INIT_LIST_HEAD(&headMainLog);
	INIT_LIST_HEAD(&headFiltedLog);
	logInterface = pInterface;
}

// void FSLOG_PushEvent(FSLOG* pLog)
// {
// 	struct list_head *container;
// 	FSLOG *iterator;
// 
// 	if(headMainLog.next != &headMainLog)
// 	{
// 		list_for_each(container,&headMainLog)
// 		{
// 			iterator = list_entry(container,FSLOG,_node);
// 			if(iterator == pLog)
// 				return;
// 		}
// 	}
// 	list_add_tail(&pLog->_node,&headMainLog);
// }
// 
// FSLOG *FSLOG_GetEvent()
// {
// 	FSLOG *iterator;
// 
// 	if(headMainLog.next == &headMainLog) return NULL;
// 	iterator = list_first_entry(&headMainLog,FSLOG,_node);
// 	if(iterator) list_del(headMainLog.next);
// 	return iterator;
// }

// int FSLOG_Rename(FSLOG *pLog, const char *pName)
// {
// 	memcpy(pLog->name,pName,strlen(pName) + 1);
// 	return 0;
// }

FSLOG* FSLOG_Open( const char* pName, const FSLOG_FUNCTION * pFunction, const FSLOG_INFORMATION *pInformation, unsigned int attribute)
{
	FSLOG *pLog;
	unsigned int i;
	unsigned char *data;
	const SYS_TIME64 *tm64;
	
	//create object and init internal variables
	pLog = fsmid_malloc(FSLOG,1);
	if(pLog == NULL)
		return NULL;
	memset(pLog,0,sizeof(FSLOG));
	memcpy(pLog->name,pName,min(strlen(pName),sizeof(pLog->name) - 1));
	pLog->pFunction = pFunction;
	pLog->pInformation = pInformation;
	fsmid_mutex_create(pLog->mutex);

	pLog->attribute = attribute;

	pLog->formatedSize = 0;
	pLog->unitPerBlock = (pInformation->blockSize - sizeof(FSLOG_HEADER))/pInformation->unitSize;
	pLog->unitPerBlock = min(FSLOG_UNIT_PERBLOCK,pLog->unitPerBlock);
	pLog->maxUnitCount = pInformation->blockNumber * pLog->unitPerBlock;
	if(pInformation->unitCount)
		fsmid_assert(pLog->maxUnitCount >= (pInformation->unitCount + ((attribute&FSLOG_ATTR_OTP)?0:pLog->unitPerBlock)),__FILE__,__LINE__);
	pLog->unitNumber = 0;
	pLog->pointerId = 0;
	pLog->indexFirst = -1;
	pLog->indexLast = -1;
	//init all pointers with flash data
	__fslog_init(pLog);

	if(pLog->unitNumber && pFunction &&pFunction->time)
	{
		data = fsmid_malloc(unsigned char,pInformation->unitSize);
		fsmid_assert(data,__FILE__,__LINE__);
		FSLOG_ReadData(pLog,0,data);
		tm64 = pFunction->time(data);
		pLog->timeCreateUnix = time_sys2unix(tm64);

		pLog->formatedSize += pFunction->format_header(NULL,pLog);
		for( i = 0; i < pLog->unitNumber; i++ )
		{
			FSLOG_ReadData(pLog,i,data);
			pLog->formatedSize += pFunction->format_data(NULL,data);
		}

		fsmid_free(data);
		FSLOG_INFO_MSG("[FSOPEN] \"%s\". Create:%02d-%02d-%02d. Placed: 0x%08X, ID:%d, %d unit.\n",pLog->name,tm64->year,tm64->mon,tm64->day,pLog->pInformation->baseAddress,pLog->pointerId,pLog->unitNumber);
	}
	else
		FSLOG_INFO_MSG("[FSOPEN] \"%s\". Placed: 0x%08X, ID:%d, %d unit.\n",pLog->name,pLog->pInformation->baseAddress,pLog->pointerId,pLog->unitNumber);
	list_add_tail(&pLog->_node,&headMainLog);
	return pLog;
}

int FSLOG_Write(FSLOG *pLog, const void* data)
{
	unsigned int address;

	if((pLog->attribute & FSLOG_ATTR_OTP) && (pLog->unitNumber >= pLog->pInformation->unitCount))
		return FSMIDR_LENGTH_LARGE;
	
	//write data into flash
	address = __fslog_index2address(pLog->indexLast);
	fsmid_assert(!log_write(address,data,pLog->pInformation->unitSize),__FILE__,__LINE__);
	if(pLog->pFunction)
		pLog->formatedSize += pLog->pFunction->format_data(NULL,data);
	else
		pLog->formatedSize += pLog->pInformation->unitSize;

	//plus counter, if to new block
	if(__fslog_plus_write_counter(pLog))
	{
		__fslog_init_block(pLog,pLog->indexLast/pLog->unitPerBlock,FSLOG_BIT_FILLING);
		//write pointer cover the oldest&read pointer?
		if(pLog->indexLast == pLog->indexFirst)
		{
			//move oldest&read pointer
			pLog->indexFirst += pLog->unitPerBlock;
			if(pLog->indexFirst >= pLog->maxUnitCount)
				pLog->indexFirst -= pLog->maxUnitCount;
			pLog->unitNumber -= pLog->unitPerBlock;
			if(pLog->pFunction)
				pLog->formatedSize -= pLog->pFunction->format_data(NULL,0) * pLog->unitPerBlock;
			else
				pLog->formatedSize -= pLog->pInformation->unitSize * pLog->unitPerBlock;
		}
	}
	FSLOG_INFO_MSG("[FSWRITE] \"%s\". Placed: 0x%08X, ID:%d, %d unit.\n",pLog->name,pLog->pInformation->baseAddress,pLog->pointerId,pLog->unitNumber);
	//FSLOG_PushEvent(pLog);
	return 0;
}

int FSLOG_LockWrite(FSLOG *pLog, const void* data)
{
	int result;
	fsmid_mutex_lock(pLog->mutex);
	result = FSLOG_Write(pLog,data);
	fsmid_mutex_unlock(pLog->mutex);
	return result;
}

int FSLOG_ReadData(FSLOG *pLog, unsigned int index, void* data)
{
	unsigned int address;

	//check read pointer is same as write pointer
	if(index >= pLog->unitNumber)
		return FSMIDR_BAD_ARGUMENT;

	index += pLog->indexFirst;
	if(index >= pLog->maxUnitCount)
		index -= pLog->maxUnitCount;

	address = __fslog_index2address(index);
	fsmid_assert(!log_read(address,data,pLog->pInformation->unitSize),__FILE__,__LINE__);

	return 0;
}

int FSLOG_LockReadData(FSLOG *pLog, unsigned int index, void* data)
{
	int result;
	fsmid_mutex_lock(pLog->mutex);
	result = FSLOG_ReadData(pLog,index,data);
	fsmid_mutex_unlock(pLog->mutex);
	return result;
}

int FSLOG_ReadFmt(FSLOG *pLog, unsigned int index, char *buf)
{
	int len;
	unsigned char* data;

	if(!pLog->pFunction)
		return 0;

	if(index == -1UL)
		return pLog->pFunction->format_header(buf,pLog);

	data = fsmid_malloc(unsigned char,FSLOG_GetUnitSize(pLog));
	fsmid_assert(data,__FUNCTION__,__LINE__);
	if(FSLOG_ReadData(pLog,index,data))
	{
		fsmid_free(data);
		return 0;
	}

	len = pLog->pFunction->format_data(buf,data);
	fsmid_free(data);

	return len;
}

int FSLOG_LockReadFmt(FSLOG *pLog, unsigned int index, char *buf)
{
	int result;
	fsmid_mutex_lock(pLog->mutex);
	result = FSLOG_ReadFmt(pLog,index,buf);
	fsmid_mutex_unlock(pLog->mutex);
	return result;
}

// int FSLOG_Move(FSLOG *pLog, unsigned int indexOffset)
// {
// 	indexOffset = min(indexOffset,pLog->unitNumber);
// 	pLog->indexRead = pLog->indexFirst + indexOffset;
// 	if(pLog->indexRead >= pLog->maxUnitCount)
// 		pLog->indexRead -= pLog->maxUnitCount;
// 	return indexOffset;
// }

int FSLOG_Clear(FSLOG *pLog)
{
	unsigned int i;

	fsmid_mutex_lock(pLog->mutex);
	for( i = 0; i <pLog->pInformation->blockNumber; i++ )
	{
// 		address = pLog->pInformation->blockSize * i + pLog->pInformation->baseAddress;
// 		fsmid_assert(!log_erase(address),__FILE__,__LINE__);
		__fslog_init_block(pLog,i,(i==0)?FSLOG_BIT_FILLING:FSLOG_FLAG_ALL);
	}
	pLog->pointerId = 0;
	pLog->unitNumber = 0;
	pLog->indexFirst = 0;
	pLog->indexLast = 0;
	FSLOG_INFO_MSG("[FSCLEAR] \"%s\". Placed: 0x%08X.\n",pLog->name,pLog->pInformation->baseAddress);
	fsmid_mutex_unlock(pLog->mutex);
	return 0;
}

unsigned int FSLOG_GetPointerIndex(FSLOG *pLog, unsigned int index)
{
	unsigned int address;
	FSLOG_HEADER header;

	//check read pointer is same as write pointer
	if(index >= pLog->unitNumber)
		return FSMIDR_BAD_ARGUMENT;

	index += pLog->indexFirst;
	if(index >= pLog->maxUnitCount)
		index -= pLog->maxUnitCount;

	address = __fslog_index2address(index) - FSLOG_HEADER_SIZE;
	fsmid_assert(!log_read(address,&header,FSLOG_HEADER_SIZE),__FILE__,__LINE__);
	return header.pointerId;
}

const char* FSLOG_GetName(FSLOG* pLog)
{
	char *pFolderMark = NULL;
	char *pName = pLog->name;
	
	while(*pName)
	{
		if(*pName == '\\')
			pFolderMark = pName;
		pName++;
	}
	if(pFolderMark)
		return pFolderMark+1;
	return pLog->name;
	
}

static bool __is_filted_name(const char *pFileName, const char *pFilterName)
{
	int l = strlen(pFilterName);

	if(!pFilterName || pFilterName[0] == '\0')
		return true;
	if(memcmp(pFileName,pFilterName,l) == 0)
	{
		if(pFilterName[l-1] == '\\')
			pFileName += l;
		else if( pFileName[l] == '\\' )
			pFileName += l + 1;
		else
			return false;
		while(*pFileName)
		{
			if(*pFileName == '\\')
				return false;
			pFileName++;
		}
		return true;
	}
	return false;
}

void FSLOG_ReleaseFilter()
{
//	list_head *container;
	FSLOG *iterator;

	while(headFiltedLog.next != headFiltedLog.prev)
	{
		iterator = list_first_entry(&headFiltedLog,FSLOG,_node);
		list_del(&iterator->_node);
		list_add_tail(&iterator->_node,&headMainLog);
	}
	nFiltedLog = 0;
}

unsigned int FSLOG_Filter(const char *pCondition)
{
	list_head *container;
	FSLOG *iterator;

	if(nFiltedLog)
		FSLOG_ReleaseFilter();
Looper:
	list_for_each(container,&headMainLog)
	{
		iterator = list_entry(container,FSLOG,_node);
		if(__is_filted_name(iterator->name,pCondition))
		if(iterator->unitNumber)
		{
			list_del(&iterator->_node);
			list_add_tail(&iterator->_node,&headFiltedLog);
			nFiltedLog++;
			//fsmid_info("%-40s filted. UNIT:%4d. SIZE:%6d.\n",iterator->name,iterator->unitNumber,iterator->formatedSize);
			goto Looper;
		}
	}
	return nFiltedLog;
}

unsigned int FSLOG_GetFiltedCount()
{
	return nFiltedLog;
}

FSLOG *FSLOG_GetFiltedItem(unsigned int index)
{
	int i = 0;
	list_head *container;
	FSLOG *iterator;

	list_for_each(container,&headFiltedLog)
	{
		iterator = list_entry(container,FSLOG,_node);
		if(i == index)
			return iterator;
		i++;
	}
	return NULL;
}