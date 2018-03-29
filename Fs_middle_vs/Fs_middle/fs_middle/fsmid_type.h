#ifndef _FSMID_TYPE_H_
#define _FSMID_TYPE_H_

#include "fsmid_port.h"
#include "fsmid_def.h"

#pragma pack(push,1)

typedef struct __fsmid_callback{
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
	*/
	void (*get_time)(FSMID_LOG_HANDLE logHandle, CP56TIME2A *pTime);

	/**
	* get_data() - get log data from flash
	* @logHandle: handle of log component
	* @pData: data buffer output to file system
	*
	*/
	void (*get_data)(FSMID_LOG_HANDLE logHandle, void* pData);

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
	//int (*put_data)(FSMID_LOG_HANDLE logHandle, unsigned int offset, const void* pData, unsigned int length);

	/**
	* file_downloaded() - one file downloaded from COMM layer
	* @pPathName: handle of log component
	*
	* Return: error code, 0 on success
	*
	* This function will be call after one file written by COMM layer and closed.
	*/
	void (*file_downloaded)( const char *pPathName);
}FSMID_CALLBACK;

typedef struct __fsmid_element{
	struct list_head _node;
	unsigned int size;
	union {;
		FSMID_LOG_HANDLE handle;
		unsigned char *buffer;
	}DATA;
}FSMID_ELEMENT;

typedef struct __fsmid_file{
	struct list_head _node;
	
	FSMID_MUTEX	mutex;
	unsigned char *pBuffer;
	unsigned int bufferSize;
	unsigned char *pCurrent;
	unsigned int bufferOffset;
	struct list_head *pContainer;  
	unsigned int currentLogGroup;

	char *pPath;
	char *pName;
	CP56TIME2A time;
	unsigned char reserved1;
	unsigned int size;

	FSMID_FILE_STATUS status;
	unsigned char attribute;
	unsigned char reserved2[2];

	struct list_head *headTableElement;
	unsigned int groupCount;

}FSMID_FILE;

typedef struct __fsmid_system{
	struct list_head headFile;
	const struct __fsmid_callback *callback;
}FSMID_SYSTEM;

#pragma pack(pop)

#endif
