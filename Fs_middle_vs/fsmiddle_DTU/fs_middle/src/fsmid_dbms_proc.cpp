
#include "dbmsV1.h"

unsigned char dpaHandle;
unsigned char dcaHandle;

int DATALOG_Init()
{
	unsigned short count;
	dpaHandle = db_GetDpaHand();
	dcaHandle = db_GetDcaHand();
	count = db_GetSoeCount(dpaHandle);
	return 0;
}

