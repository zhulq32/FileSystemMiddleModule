#ifndef _FSMID_DEF_H_
#define _FSMID_DEF_H_

#include "fsmid_port.h"

#pragma pack(push,1)

#define FSMID_MAX_PATH			(32)
#define FSMID_MAX_FILE_NAME		(64)

enum __fsmid_result{
	FSMIDR_OK = 0,
	FSMIDR_GENERAL,
	FSMIDR_LEAK_MEMORY,
	FSMIDR_BAD_ARGUMENT,
	FSMIDR_LENGTH_LARGE,
	FSMIDR_ACCESS,
	FSMIDR_NOT_EXIST,
};

typedef enum __fsmid_create_attr{
	FSMIDA_READ = 1,
	FSMIDA_WRITE = 4,
	FSMIDA_PUSHPOP = 8,
	FSMIDA_CREATE = 16,
}FSMID_CREATE_ATTR;


typedef enum __fsmid_seek{
	FSMIDS_BEGIN = 0,
	FSMIDS_CURRENT,
	FSMIDS_END,
}FSMID_SEEK;

typedef struct __fsmid_stat{
	unsigned char attribute;
	unsigned char status;
	unsigned char reserved[2];
	unsigned int size;
	CP56TIME2A time;
	unsigned char reserved2;
}FSMID_STAT;

typedef void * FSMID_FHANDLE;
#define FSMID_INVALID_HANDLER	((FSMID_FHANDLE)0xFFFFFFFF)

#pragma pack(pop)

#endif