#ifndef _FSMID_DEF_H_
#define _FSMID_DEF_H_

#pragma pack(push,1)

#define FSMID_MAX_PATH			(32)
#define FSMID_MAX_FILE_NAME	(64)

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
	fsmid_ok = 0,
	fsmid_general,
	fsmid_leak_memory,
	fsmid_bad_argument,
	fsmid_length_large,
	fsmid_access,
	fsmid_file_not_exist,
};

typedef enum __fsmid_create_attr{
	fsmid_read = 1,
	fsmid_write = 2,
	fsmid_create = 4,
}FSMID_CREATE_ATTR;


typedef enum __fsmid_seek{
	fsmid_seekBegin = 0,
	fsmid_seekCurrent,
	fsmid_seekEnd,
}FSMID_SEEK;

typedef struct __fsmid_stat{
	unsigned char attribute;
	unsigned char status;
	unsigned char reserved[2];
	unsigned int size;
	CP56TIME2A time;
	unsigned char reserved2;
}FSMID_STAT;

typedef void* FSMID_FHANDLE;
#define FSMID_INVALID_HANDLER	((FSMID_FHANDLE)0)

#pragma pack(pop)

#endif