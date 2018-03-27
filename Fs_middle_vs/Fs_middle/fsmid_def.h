#ifndef _FSMID_DEF_H_
#define _FSMID_DEF_H_

#include "fsmid_port.h"

#pragma pack(push,1)

#ifndef NULL
#define NULL	0
#endif


#define FSMID_MAX_PATH			(32)
#define FSMID_MAX_FILE_NAME		(64)

//#define __DEF_CP56TIME2A
#ifndef __DEF_CP56TIME2A
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
#endif

enum __fsmid_result{
	FSMIDR_OK = 0,
	FSMIDR_GENERAL,
	FSMIDR_LEAK_MEMORY,
	FSMIDR_BAD_ARGUMENT,
	FSMIDR_LENGTH_LARGE,
	FSMIDR_ACCESS,
	FSMIDR_CONFLICT,
	FSMIDR_NOT_EXIST,
	FSMIDR_POINT_NOT_NULL,
};

typedef enum __fsmid_create_attr{
	FSMIDO_READ = 1,
	FSMIDO_WRITE = 4,
	FSMIDO_PUSHPOP = 8,
	FSMIDO_CREATE = 16,
}FSMID_OPEN_ATTR;

typedef enum __fsmid_file_status{
	FSMIDS_NOT_OPEN = 0,
	FSMIDS_OPENED,
}FSMID_FILE_STATUS;


typedef enum __fsmid_seek{
	FSMIDS_BEGIN = 0,
	FSMIDS_CURRENT,
	FSMIDS_END,
}FSMID_SEEK;

typedef struct __fsmid_stat{
	FSMID_FILE_STATUS status;
	FSMID_OPEN_ATTR attribute;
	unsigned char reserved[2];
	CP56TIME2A time;
	unsigned char reserved2;
	unsigned int size;
	unsigned int offset;
}FSMID_STAT;

typedef void * FSMID_FHANDLE;
#define FSMID_INVALID_HANDLER	((FSMID_FHANDLE)0)

#pragma pack(pop)

#endif