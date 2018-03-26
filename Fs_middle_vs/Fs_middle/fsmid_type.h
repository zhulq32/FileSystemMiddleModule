#ifndef _FSMID_TYPE_H_
#define _FSMID_TYPE_H_

#include "fsmid_def.h"
#include "list_linux.h"

#pragma pack(push,1)
typedef struct __cp56time2a {
	unsigned short milliSecond;
	unsigned char minute:6;
	unsigned char res1 :1;
	unsigned char invalid :1;

	unsigned char hour :5;
	unsigned char res2 :2;
	unsigned char summerTime :1;

	unsigned char monthDay :5;
	unsigned char weekDay :3;

	unsigned char month :4;
	unsigned char res3 :4;

	unsigned char year :7;
	unsigned char res4 :1;
}CP56TIME2A;

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
	struct __fsmid_port *currentPort;

}FSMID_FILE;

typedef struct __fsmid_port{
	
}FSMID_PORT;


#pragma pack(pop)

#endif