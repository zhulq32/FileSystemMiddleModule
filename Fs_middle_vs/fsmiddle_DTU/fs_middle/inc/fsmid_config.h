#ifndef _FSMID_CONFIG_H_
#define _FSMID_CONFIG_H_

#include "fsmid_log.h"

#ifndef __cplusplus
extern "C" {
#endif

extern FSLOG_INTERFACE intrFslog;

//extern FSLOG_FUNCTION funcRawData;//for raw soe and print log

extern FSLOG_FUNCTION funcLogUlog;

extern FSLOG_FUNCTION funcLogSoe;

extern FSLOG_FUNCTION funcLogCo;

extern FSLOG_FUNCTION funcLogExtreme;//for logMaximum and logMinimum

extern FSLOG_FUNCTION funcLogFixpt;

extern FSLOG_FUNCTION funcLogFrozen;

unsigned int FSLOG_CalcBlockNumber(unsigned int unitSize, unsigned int blockSize, unsigned int unitCount,bool bOtp);


extern FSLOG_INFORMATION infoRawSoe;
extern FSLOG_INFORMATION infoPrintLog;
extern FSLOG_INFORMATION infoLogUlog;
extern FSLOG_INFORMATION infoLogSoe;
extern FSLOG_INFORMATION infoLogCo;
extern FSLOG_INFORMATION infoLogExtreme;
extern FSLOG_INFORMATION infoLogFixpt;
extern FSLOG_INFORMATION infoLogFrozen;

#ifndef __cplusplus
};
#endif

#endif
