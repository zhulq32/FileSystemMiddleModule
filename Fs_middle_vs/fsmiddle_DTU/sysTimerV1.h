#ifndef __SYSTIMER_H_
#define __SYSTIMER_H_

//#include "stdint.h"

typedef struct
{
	unsigned char year;
	unsigned char mon;
	unsigned char day;
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
	unsigned short msec;
}SYS_TIME64;

/****系统 TICK 和 RTC************************************/
extern unsigned int glb_sysTick;//FREERUNNING TICK, 程序直接用本变量，不通过函数读取，程序启动后从0开始计
extern unsigned int glb_sysSecond;//RTC.TSR值
extern unsigned short glb_RtcMsec;

//bool glb_sysTick_Init(void);//由RTOS XTick的 vApplicationTickHook()完成，在user.c中
bool glb_sysRtc_Init(void);
void glb_GetDateTime(SYS_TIME64 *datetime);
void glb_SetDateTime(SYS_TIME64 *datetime);

#endif