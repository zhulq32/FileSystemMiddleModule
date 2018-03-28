#ifndef _FSMID_FILE_H_
#define _FSMID_FILE_H_

/**
* FSMID_Open() - open file
* @pHandler: file handle upload from file system
* @pPathName: fully path name of file
* @attribute: read/write/push pull/create 
*
* Return: error code, 0 on success
*
* output file pointer with handler type from file system.
* if attribute CREATE, create a new file object, then add to
* file system.
* READ/WRITE feature for COMM layer.
*
*/
int FSMID_Open(FSMID_FHANDLE* pHandler, const char* pPathName, FSMID_OPEN_ATTR attribute);

/**
* FSMID_Close() - close file
* @handle: file handle upload from file system
*
* Return: error code, 0 on success
*
* release resources.
*/
int FSMID_Close(FSMID_FHANDLE handle);

/**
* FSMID_Remove() - remove file
* @pPathName: fully path name of file
*
* Return: error code, 0 on success
*
* remove file from file system, and release resources.
*/
int FSMID_Remove(const char* pPathName);


/**
* FSMID_Read() - read file content
* @handle: file handle
* @pData: buffer for data put into
* @length: number of bytes for the buffer
*
* Return: error code, 0 on success
*
* read file content into buffer(pData).
*/
int FSMID_Read(FSMID_FHANDLE handle, void* pData, unsigned int length);

/**
* FSMID_Write() - write file content
* @handle: file handle
* @pData: buffer for data write into flash
* @length: number of bytes for the buffer
*
* Return: error code, 0 on success
*
* write file content into buffer(pData)/flash.
*/
int FSMID_Write(FSMID_FHANDLE handle, const void* pData, unsigned int length);


/**
* FSMID_Seek() - move file content pointer
* @handle: file handle
* @offset: how many to move the pointer
* @method: how to move the pointer
*
* Return: error code, 0 on success
*
* move file content pointer, used to re-transfer during read/write.
*/
int FSMID_Seek(FSMID_FHANDLE handle, signed int offset, FSMID_SEEK method);

/**
* FSMID_Stat() - get file state structure object
* @pPathName: fully path name of file
* @pStat: file state object pointer
*
* Return: error code, 0 on success
*
*/
int FSMID_Stat(const char* pPathName, FSMID_STAT *pStat);

/**
* FSMID_List() - list the files under the given folder
* @pPath: string of path
* @aList: list object to get by the up layer
* @numList: number of list objects could be given.
*
* Return: real number of list item under the folder
*
*/
int FSMID_List(const char *pPath, FSMID_LIST aList[], unsigned int numList);

#endif
