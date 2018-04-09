#include "fsmid_def.h"
#include "dtu_event.h"
#include <stdio.h>

int FORMAT_SOE_Event(char *buf, unsigned int maxlen, const void *event)
{
	int len;
	const EVENT_SOE *soe = (const EVENT_SOE *)event;
	len = sprintf(buf,"%5X,%d,%4d-%02d-%02d %02d:%02d:%02d.%03d\r\n",
		soe->point,soe->value,
		soe->time.year,soe->time.mon,soe->time.day,
		soe->time.hour,soe->time.min,soe->time.sec,
		soe->time.msec);
	fsmid_assert(len <= maxlen,__FUNCTION__,__LINE__);
	return len;
}

const char *cstrTrdOperate[] = {
	"分","合"
};

int FORMAT_TRD_Event(char *buf, unsigned int maxlen, const void *event)
{
	int len;
	const EVENT_TRD *trd = (const EVENT_TRD *)event;
	len = sprintf(buf,"%5X,选择,%s,%4d-%02d-%02d %02d:%02d:%02d.%03d\r\n%5X,执行,%s,%4d-%02d-%02d %02d:%02d:%02d.%03d\r\n",
		trd->point,cstrTrdOperate[trd->value?1:0],
		trd->time.year,trd->time.mon,trd->time.day,
		trd->time.hour,trd->time.min,trd->time.sec,
		trd->time.msec,
		trd->point,cstrTrdOperate[trd->value?1:0],
		trd->time.year,trd->time.mon,trd->time.day,
		trd->time.hour,trd->time.min,trd->time.sec,
		trd->time.msec);
	fsmid_assert(len <= maxlen,__FUNCTION__,__LINE__);
	return len;
}

int FORMAT_EXTREME_Event(char *buf, unsigned int maxlen, const void *event)
{
	return 0;
}

int FORMAT_ULOG_Header(char *buf, unsigned int maxlen, const void *event)
{

}

int FORMAT_ULOG_Event(char *buf, unsigned int maxlen, const void *event)
{
	int len;
	const EVENT_ULOG *ulog = (const EVENT_ULOG *)event;
	len = sprintf(buf,"%2d,%4d-%02d-%02d %02d:%02d:%02d.%03d,%-64.64s,%d\r\n",
		ulog->type,
		ulog->time.year,ulog->time.mon,ulog->time.day,
		ulog->time.hour,ulog->time.min,ulog->time.sec,
		ulog->time.msec,
		ulog->buf,ulog->status);
	fsmid_assert(len <= maxlen,__FUNCTION__,__LINE__);
	return len;
}