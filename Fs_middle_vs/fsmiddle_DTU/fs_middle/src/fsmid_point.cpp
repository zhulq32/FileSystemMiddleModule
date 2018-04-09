#include "dbmsV1.h"
#include "dpa10x.h"
#include <string.h>
#include "fsmid_port.h"
#include "fsmid_point.h"

static FSMID_POINT* tableMeasure = NULL;
static unsigned int numMeasure;

static LOG_EXTREME *tableMaximum = NULL;
static LOG_EXTREME *tableMinimum = NULL;


static FSMID_POINT* tableFrozen = NULL;
static unsigned int numFrozen;

#define MEASURE_CONFIG_SIG		0x5341454D	//"FSMD"
#define FROZEN_CONFIG_SIG		0x4E5A5246	//"FRZN"

typedef struct __st_fsmid_config{
	unsigned int signature;
	unsigned int number;
	FSMID_POINT point[0];
}FSMID_CONFIG;

void FSMID_InitConfig()
{
	int bChanged = 0;
	int i, j;
	struDpa10xFrm *pfrm;
	void *ppntcfg = NULL;
	struDpa10xMe_Cfg *pMeCfg;
	struDpa10xIt_Cfg *pItCfg;
	uint32 inf;
	FSMID_POINT *pPoint;
	FSMID_CONFIG *pConfig;

	

	pfrm = dpa101appl.pport->pfrm;
	for (i = 0,numMeasure = 0; i < dpa101appl.pport->frmnum; i++, pfrm++)
	{
		if (pfrm->pcfg->frmtype == M_ME_NC_1)//仅处理浮点遥测帧
		{
			numMeasure += pfrm->pntnum;
		}
		else if (pfrm->pcfg->frmtype == M_IT_NB_1)//仅处理浮点累积量
		{
			numFrozen += pfrm->pntnum;
		}
	}

	tableMaximum = fsmid_malloc(LOG_EXTREME,numMeasure);
	memset(tableMaximum,0,sizeof(LOG_EXTREME)*numMeasure);
	tableMinimum = fsmid_malloc(LOG_EXTREME,numMeasure);
	memset(tableMinimum,0,sizeof(LOG_EXTREME)*numMeasure);
	for(i = 0; i < numMeasure; i++)
	{
		tableMaximum[i].pointIndex = i;
		tableMaximum[i].type = 0;
		tableMinimum[i].pointIndex = i;
		tableMinimum[i].type = 1;
	}

	tableMeasure = fsmid_malloc(FSMID_POINT,numMeasure);
	tableFrozen = fsmid_malloc(FSMID_POINT,numFrozen);
	fsmid_assert(tableMeasure && tableFrozen,__FILE__,__LINE__);

	pPoint = tableMeasure;
	pfrm = dpa101appl.pport->pfrm;
	for (i = 0; i < dpa101appl.pport->frmnum; i++, pfrm++)
	{
		if (pfrm->pcfg->frmtype == M_ME_NC_1)//仅处理浮点遥测帧
		{
			inf = pfrm->pcfg->frminf;//起始信息对象
			pMeCfg = (struDpa10xMe_Cfg *)pfrm->ppntcfg;//指向flash中的浮点遥测配置表
			for (j = 0; j < pfrm->pntnum; j++, pMeCfg++, inf++,pPoint++)
			{
				pPoint->information = inf;
				pPoint->point = pMeCfg->syspnt;
			}
		}
	}

	pPoint = tableFrozen;
	pfrm = dpa101appl.pport->pfrm;
	for (i = 0; i < dpa101appl.pport->frmnum; i++, pfrm++)
	{
		if (pfrm->pcfg->frmtype == M_IT_NB_1)//仅处理浮点累积量
		{
			inf = pfrm->pcfg->frminf;//起始信息对象
			pItCfg = (struDpa10xIt_Cfg *)pfrm->ppntcfg;//指向flash中的浮点累积量配置表
			for (j = 0; j < pfrm->pntnum; j++, pItCfg++, inf++,pPoint++)
			{
				pPoint->information = inf;
				pPoint->point = pItCfg->syspnt;
			}
		}
	}

	if(numMeasure == 0 || numFrozen == 0)
	{
		fsmid_warning("No measure or frozen point.",__FILE__,__LINE__);
		return;
	}

	pConfig = fsmid_malloc(FSMID_CONFIG,1 + max(numMeasure,numFrozen)/2 + 1);
	read_flash(FSMID_CONFIG_ADDRESS,pConfig,sizeof(FSMID_CONFIG) + numMeasure*4);
	if(pConfig->signature != MEASURE_CONFIG_SIG || pConfig->number != numMeasure || memcmp(pConfig->point,tableMeasure,numMeasure*sizeof(int)))
	{
		bChanged = 1;
	}
	read_flash(FSMID_CONFIG_ADDRESS+FLASH_BLOCK_SIZE,pConfig,sizeof(FSMID_CONFIG) + numFrozen*4);
	if(pConfig->signature != FROZEN_CONFIG_SIG || pConfig->number != numFrozen || memcmp(pConfig->point,tableFrozen,numFrozen*sizeof(int)))
	{
		bChanged = 1;
	}

	if(bChanged)
	{
		erase_flash(FSMID_CONFIG_ADDRESS,FLASH_MEMORY_SIZE);

		pConfig->signature = MEASURE_CONFIG_SIG;
		pConfig->number = numMeasure;
		memcpy(pConfig->point,tableMeasure,numMeasure*sizeof(int));
		write_flash(FSMID_CONFIG_ADDRESS,pConfig,sizeof(FSMID_CONFIG) + numMeasure*4);

		pConfig->signature = FROZEN_CONFIG_SIG;
		pConfig->number = numFrozen;
		memcpy(pConfig->point,tableFrozen,numFrozen*sizeof(int));
		write_flash(FSMID_CONFIG_ADDRESS+FLASH_BLOCK_SIZE,pConfig,sizeof(FSMID_CONFIG) + numFrozen*4);
	}

	fsmid_free(pConfig);
}

FSMID_POINT * const GetMeasureTable()
{
	return tableMeasure;
}

unsigned int GetMeasureCount()
{
	return numMeasure;
}

LOG_EXTREME * const GetMaximumTable()
{
	return tableMaximum;
}

LOG_EXTREME * const GetMinimumTable()
{
	return tableMinimum;
}

void ResetExtremeTable()
{
	#define FLT_MAX         3.402823466e+38F
	#define FLT_MIN         1.175494351e-38F
	unsigned int i;
// 	unsigned int *pMax = (unsigned int*)tableMaximum;
// 	unsigned int *pMin = (unsigned int*)tableMinimum;
	for( i = 0; i < numMeasure; i++ )
	{
		tableMaximum[i].value = FLT_MIN;
		tableMinimum[i].value = FLT_MAX;
	}
}

void UpdateExtremeValue(const SYS_TIME64 *t64, unsigned int index, float fValue)
{
	if(tableMaximum[index].value >= 0 && fValue >= 0)
	{
		if(fValue > tableMaximum[index].value)
		{
			memcpy(&tableMaximum[index].time,t64,sizeof(SYS_TIME64));
			tableMaximum[index].value = fValue;
		}
	}
	else if(tableMaximum[index].value >= 0 && fValue < 0)
	{
		if( fValue * -1.0 > tableMaximum[index].value)
		{
			memcpy(&tableMaximum[index].time,t64,sizeof(SYS_TIME64));
			tableMaximum[index].value = fValue;
		}
	}
	else if(tableMaximum[index].value < 0 && fValue >= 0)
	{
		if( fValue > tableMaximum[index].value * -1.0 )
		{
			memcpy(&tableMaximum[index].time,t64,sizeof(SYS_TIME64));
			tableMaximum[index].value = fValue;
		}
	}
	else if(tableMaximum[index].value < 0 && fValue < 0)
	{
		if(fValue < tableMaximum[index].value)
		{
			memcpy(&tableMaximum[index].time,t64,sizeof(SYS_TIME64));
			tableMaximum[index].value = fValue;
		}
	}

	if(tableMinimum[index].value >= 0 && fValue >= 0)
	{
		if(fValue < tableMinimum[index].value)
		{
			memcpy(&tableMinimum[index].time,t64,sizeof(SYS_TIME64));
			tableMinimum[index].value = fValue;
		}
	}
	else if(tableMinimum[index].value >= 0 && fValue < 0)
	{
		if( fValue * -1.0 < tableMinimum[index].value)
		{
			memcpy(&tableMinimum[index].time,t64,sizeof(SYS_TIME64));
			tableMinimum[index].value = fValue;
		}
	}
	else if(tableMinimum[index].value < 0 && fValue >= 0)
	{
		if( fValue < tableMinimum[index].value * -1.0 )
		{
			memcpy(&tableMinimum[index].time,t64,sizeof(SYS_TIME64));
			tableMinimum[index].value = fValue;
		}
	}
	else if(tableMinimum[index].value < 0 && fValue < 0)
	{
		if(fValue > tableMinimum[index].value)
		{
			memcpy(&tableMinimum[index].time,t64,sizeof(SYS_TIME64));
			tableMinimum[index].value = fValue;
		}
	}
}


FSMID_POINT * const GetFrozenTable()
{
	return tableFrozen;
}

unsigned int GetFrozenCount()
{
	return numFrozen;
}
