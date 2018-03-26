#ifndef _FSMID_TYPE_H_
#define _FSMID_TYPE_H_

#include "fsmid_port.h"
#include "fsmid_def.h"
#include "list_linux.h"

#pragma pack(push,1)

typedef struct __fsmid_file{
	struct list_head nodeFsmid;
	struct list_head headPort;

	FSMID_MUTEX	mutex;

	char path[FSMID_MAX_PATH];
	char name[FSMID_MAX_FILE_NAME];
	CP56TIME2A time;
	unsigned char reserved1;
	unsigned int size;

	unsigned char status;
	unsigned char attribute;
	unsigned char reserved2[2];

	unsigned char* buffer;
	unsigned int bufSize;
	unsigned char* currentBuf;

}FSMID_FILE;


#pragma pack(pop)

#endif