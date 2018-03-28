#ifndef _FSMID_TYPE_H_
#define _FSMID_TYPE_H_

#include "fsmid_port.h"
#include "fsmid_def.h"

#pragma pack(push,1)

typedef struct __fsport_function{
	/**
	* get_length() - get formated(msg) log data length
	* @logHandle: handle of log component
	*
	* Return: formated(msg) log data length. 0 means fail.
	*
	*/
	unsigned int (*get_length)(FSMID_LOG_HANDLE logHandle);

	/**
	* get_time() - get time of log data
	* @logHandle: handle of log component
	* @pTime: time output to file system
	*
	* Return: error code, 0 on success
	*
	* When logHandle equal to FSMID_INVALID_LOG_HANDLER, output current 
	* system time.
	*/
	int (*get_time)(FSMID_LOG_HANDLE logHandle, CP56TIME2A *pTime);

	/**
	* get_data() - get log data from flash
	* @logHandle: handle of log component
	* @pData: data buffer output to file system
	*
	* Return: error code, 0 on success
	*
	*/
	int (*get_data)(FSMID_LOG_HANDLE logHandle, void* pData);

	/**
	* put_data() - put log data to flash
	* @logHandle: handle of log component
	* @offset: data buffer place offset of the log unit
	* @pData: data buffer input from file system
	* @length: number of data buffer bytes
	*
	* Return: error code, 0 on success
	*
	* This function used for write file. COMM->FSMID->LOG_FLASH
	*/
	int (*put_data)(FSMID_LOG_HANDLE logHandle, unsigned int offset, const void* pData, unsigned int length);
}FSPORT_FUNCTION;

#define ___fsmid_get_data(file,lhandle)		\
	(file)->pFunctionTable[(file)->currentLogGroup].get_data(lhandle,(file)->pBuffer)

typedef struct __fsmid_list_log{
	FSMID_LOG_HANDLE handle;
	unsigned int size;
	struct list_head _node;
}FSMID_LIST_LOG;

typedef struct __fsmid_file{
	struct list_head _node;
	
	FSMID_MUTEX	mutex;
	unsigned char *pBuffer;
	unsigned int bufSize;
	unsigned char *pCurrent;
	unsigned int bufOffset;
	struct list_head *pContainer;  
	unsigned int currentLogGroup;

	char *pPath;
	char *pName;
	CP56TIME2A time;
	unsigned char reserved1;
	unsigned int size;

	FSMID_FILE_STATUS status;
	FSMID_OPEN_ATTR attribute;
	unsigned char reserved2[2];

	const FSPORT_FUNCTION* pFunctionTable;
	struct list_head *headPortTable;
	unsigned int groupCount;

}FSMID_FILE;

typedef struct __fsmid_system{
	struct list_head headFile;
}FSMID_SYSTEM;

#pragma pack(pop)

#endif