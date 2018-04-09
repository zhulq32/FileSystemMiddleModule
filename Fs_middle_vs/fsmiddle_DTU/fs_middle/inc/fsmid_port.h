#ifndef _FSMID_PORT_H_
#define _FSMID_PORT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>


#ifndef __cplusplus
extern "C" {
#endif


#define fsmid_delayMs(ms)			Sleep(ms)

// #define fsmid_malloc(typ,sz)		(typ *)malloc((sz)*sizeof(typ))
// #define fsmid_free(ptr)				free(ptr)
extern void* test_malloc(unsigned int);
extern void test_free(void *);
#define fsmid_malloc(typ,sz)		(typ *)test_malloc((sz)*sizeof(typ))
#define fsmid_free(ptr)				test_free(ptr)

#define fsmid_assert(cond,func,line)		do{if(!(cond)){printf("[ASSERT] %s(%d).\n",func,line);while(1);}}while(0)
#define fsmid_warning(str,func,line)		printf(str)//
#define fsmid_info(s,...)					printf(s,##__VA_ARGS__)

#define FSMID_MUTEX					HANDLE//xSemaphoreHandle
#define fsmid_mutex_create(mtx)		mtx = CreateMutex(NULL,FALSE,#mtx)//vSemaphoreCreateBinary(mtx)
#define fsmid_mutex_lock(mtx)		WaitForSingleObject(mtx,INFINITE)//xSemaphoreTake(mtx,portMAX_DELAY)
#define fsmid_mutex_unlock(mtx)		ReleaseMutex(mtx)//xSemaphoreGive(mtx)
#define fsmid_mutex_release(mtx)	CloseHandle(mtx)//vSemaphoreRelease(mtx)

#define fsmid_get_systime(tm)		//(tm)

#define FLASH_BLOCK_SIZE			4096
#define FLASH_MEMORY_SIZE			(16<<20)

#define FSMID_CONFIG_ADDRESS		0
#define FSMID_DATA_ADDRESS			(64<<10)

#define MAXIMUM_MEASURE_POINT		(16*4)
#define MAXIMUM_POWER_POINT			(4*4)

extern int write_flash(unsigned int address, const void* data, unsigned int length);
extern int read_flash(unsigned int address, void* data, unsigned int length);
extern int erase_flash(unsigned int address, unsigned int length);

// typedef unsigned int FSMID_LOG_HANDLE;
// #define FSMID_INVALID_LOG_HANDLER	((FSMID_LOG_HANDLE)0xFFFFFFFF)

#ifndef __cplusplus
};
#endif

#endif
