#ifndef _FSMID_TYPE_H_
#define _FSMID_TYPE_H_

#include "fsmid_port.h"
#include "fsmid_def.h"
#include "list_linux.h"


#ifndef __cplusplus
extern "C" {
#endif

#pragma pack(push,1)

typedef struct __fsmid_file{
	struct list_head _node;
	
	FSMID_MUTEX	mutex;
	unsigned char *pBuffer;
	unsigned int bufferSize;
	struct list_head *pContainer;  
	unsigned int containderIndex;

	char *pPath;
	char *pName;
	CP56TIME2A time;
	unsigned char reserved1;
	unsigned int size;

	FSMID_FILE_STATUS status;
	unsigned char attribute;
	unsigned char reserved2[2];

	struct list_head *lsElement;
	unsigned int elementCount;

}FSMID_FILE;

typedef struct __fsmid_system{
	struct list_head lsFile;
}FSMID_SYSTEM;

#pragma pack(pop)

#ifndef __cplusplus
};
#endif

#endif
