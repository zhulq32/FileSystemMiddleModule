#ifndef _FSMID_DEF_H_
#define _FSMID_DEF_H_

#include "fsmid_port.h"
#include "sysTimerV1.h"


#ifndef __cplusplus
extern "C" {
#endif

#define FILE_VERSION		"1.0"

#pragma pack(push,1)

#ifndef NULL
#define NULL	0
#endif

#ifndef min
#define min(a,b)		(((a)<(b))?(a):(b))
#endif

#ifndef xor
#define xor(a,b)		((!(a)&&(b))||((a)&&!(b)))
#endif


// #define FSMID_MAX_PATH				(128)
// #define FSMID_SAFTY_BUFFER_SIZE		(512)

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


#pragma pack(pop)

extern unsigned int bitmap2number(unsigned char* bitmap, unsigned int length);
extern void number2bitmap(unsigned int number, unsigned char* bitmap, unsigned int length);

bool systimeSameDay(const SYS_TIME64 *tm1, const SYS_TIME64 *tm2);
bool unixSameDay(unsigned int tm1, unsigned int tm2);
extern unsigned int time_sys2unix(const SYS_TIME64* systime);
extern void time_unix2sys(unsigned int unix, SYS_TIME64 *systime);

#ifndef __cplusplus
};
#endif

#endif