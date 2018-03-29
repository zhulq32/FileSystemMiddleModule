#ifndef _FSMID_PORT_H_
#define _FSMID_PORT_H_

#include <stdio.h>
#include <stdlib.h>

#define fsmid_malloc(typ,sz)		(typ *)malloc(sz*sizeof(typ))
#define fsmid_free(ptr)				free(ptr)

#define fsmid_assert(cond,func,line)		if(!(cond)) while(1)

#define FSMID_MUTEX					unsigned int//xSemaphoreHandle
#define fsmid_mutex_create(mtx)		//vSemaphoreCreateBinary(mtx)
#define fsmid_mutex_lock(mtx)		//xSemaphoreTake(mtx,portMAX_DELAY)
#define fsmid_mutex_unlock(mtx)		//xSemaphoreGive(mtx)
#define fsmid_mutex_release(mtx)	//vSemaphoreCreateBinary(mtx)

#define fsmid_get_systime(tm)		//(tm)


typedef unsigned int FSMID_LOG_HANDLE;
#define FSMID_INVALID_LOG_HANDLER	((FSMID_LOG_HANDLE)0xFFFFFFFF)


#endif
