#ifndef _FSMID_PORT_H_
#define _FSMID_PORT_H_

#define fsmid_malloc(typ,sz)		(typ *)malloc(sz*sizeof(typ))
#define fsmid_free(ptr)				free(ptr)

#define FSMID_MUTEX					unsigned int//xSemaphoreHandle
#define fsmid_mutex_create(mtx)		//vSemaphoreCreateBinary(mtx)
#define fsmid_mutex_lock(mtx)		//xSemaphoreTake(mtx,portMAX_DELAY)
#define fsmid_mutex_unlock(mtx)		//xSemaphoreGive(mtx)



#pragma pack(push,1)
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

#pragma pack(pop)

typedef unsigned int FSMID_LOG_HANDLE;
#define FSMID_INVALID_LOG_HANDLER	((FSMID_LOG_HANDLE)0xFFFFFFFF)

/**
 * fsport_length() - get formated(msg) log data length
 * @logHandle: handle of log component
 *
 * Return: formated(msg) log data length. 0 means fail.
 *
 */
extern unsigned int fsport_length(FSMID_LOG_HANDLE logHandle);

/**
 * fsport_time() - get time of log data
 * @logHandle: handle of log component
 * @pTime: time output to file system
 *
 * Return: error code, 0 on success
 *
 * When logHandle equal to FSMID_INVALID_LOG_HANDLER, output current 
 * system time.
 */
extern int fsport_time(FSMID_LOG_HANDLE logHandle, CP56TIME2A *pTime);

/**
 * fsport_data() - get log data
 * @logHandle: handle of log component
 * @data: data buffer output to file system
 *
 * Return: error code, 0 on success
 *
 */
extern int fsport_data(FSMID_LOG_HANDLE logHandle, void* data);

#endif
