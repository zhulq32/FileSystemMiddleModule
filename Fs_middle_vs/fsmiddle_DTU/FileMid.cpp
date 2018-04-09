#include "dbmsV1.h"
//#include "flashshell.h"

bool flashshell_iFlash_Init(void);
bool flashshell_Erase(uint32 start, uint32 lengthInBytes);//lengthInBytes 16必须字节对齐
bool flashshell_Write(uint32 start, uint32 *src, uint32 lengthInBytes);//lengthInBytes 16必须字节对齐
bool flashshell_Read(uint32 start, uint32 *des, uint32 lengthInBytes);

//#include "board.h"

#include "dpa10x.h"
#include <string.h>

extern unsigned int dpaZzmaint_Crc16(void* pvBuff, int lens);


static bool f_ProgramUpgrade = false;
static uint8 confirmbuf[64];//从操作标识开始
static uint16 confirmlen;

enum
{
	TRANS_READ_FILE = 1,
	TRANS_WRITE_FILE,
};

typedef struct
{
	uint16 f_type;//操作类型
	uint32 addr;//上下载存储目的位置的首地址
	uint32 totsize;//文件总字节数
	uint16 currsize;//已收到的字节数

}struFileMid_FileTrans;
static struFileMid_FileTrans FileTrans = { 0, 0, 0, 0 };


/*处理文件服务请求*/
uint8 FileMid_101Requir(uint8 *rexbuf, uint16 rexlen, uint16 rcot)
{
	uint8 op;
	uint8 datacontinue = 0;
	uint16 i=0;
	uint16 m;
	uint16 tn;

	op = rexbuf[i++];//// 操作标识
	switch (op)
	{
	case 1://读目录激活
		break;
	case 3://读文件激活
		break;
	case 7://写文件激活，目前仅用于软件升级
		m = rexbuf[i++];//文件名长度 
		i += m;//文件名，忽略软件升级的文件名
		i += 4;//文件ID;
		FileTrans.totsize = *(uint32*)&rexbuf[i];//文件大小
		i += 4;

		tn = 0;
		confirmbuf[tn++] = 8;//操作标识：写文件激活确认
		confirmbuf[tn++] = 0;//0成功，1未知错误，2文件名不支持，3长度超范围 
		memcpy(&confirmbuf[tn], &rexbuf[1], rexlen - 1);

		FileTrans.f_type = TRANS_WRITE_FILE;
		FileTrans.addr = 0x80000;//MEMADDR_FLASH_PRG1;
		FileTrans.currsize = 0;
		datacontinue = 2;//有后续class2
		break;

	case 9://写文件数据
		if (FileTrans.f_type == TRANS_WRITE_FILE)
		{
			uint32 addr = FileTrans.addr + FileTrans.currsize;
			if (flashshell_Write(addr, (uint32*)&rexbuf[10], rexlen - 10))//写入FLASH
			{
				FileTrans.currsize += rexlen - 10;

				tn = 0;
				confirmbuf[tn++] = 10;//操作标识：写文件数据确认
				memcpy(&confirmbuf[tn], &rexbuf[1], 8);//ID,数据段号
				tn += 8;
				confirmbuf[tn++] = 0;//0成功，1未知错误，2文件名不支持，3长度超范围 
				datacontinue = 2;//有后续class2
			}
			else memset(&FileTrans, 0, sizeof(FileTrans));//写出错则不应答确认
		}
		else memset(&FileTrans, 0, sizeof(FileTrans));//写出错则不应答确认

		if(FileTrans.totsize && FileTrans.currsize == FileTrans.totsize)
		{
			memset(&FileTrans, 0, sizeof(FileTrans));

			if (f_ProgramUpgrade)
			{
				struPrgCfgCbk cbk;
				cbk.identify = MEMCTRLCBK_IDT;
				cbk.size = FileTrans.totsize;
				cbk.systick = glb_sysTick;
				cbk.chksum = dpaZzmaint_Crc16((uint8*)&cbk, sizeof(struPrgCfgCbk) - sizeof(uint16));
				flashshell_Write(0x80000+0x58000-64, (uint32*)&cbk, sizeof(struPrgCfgCbk));
				f_ProgramUpgrade = false;
			}
		}	
		break;
	}

	return datacontinue;
}


/*读取后续class1 class2数据*/
uint8 FileMid_101Continue(uint8 *texbuf, uint16 *texlen, uint16 *tcot)
{
	uint8 datacontinue = 0;
	int datalen = 0;

	switch (FileTrans.f_type)
	{
	case TRANS_WRITE_FILE:
		if(confirmlen)
		{
			memcpy(texbuf, confirmbuf, confirmlen);
			datalen = confirmlen;
			confirmlen = 0;
		}
		break;

	case TRANS_READ_FILE:
		if(confirmlen)
		{
			memcpy(texbuf, confirmbuf, confirmlen);
			datalen = confirmlen;
			confirmlen = 0;
		}
		if(FileTrans.totsize && FileTrans.currsize < FileTrans.totsize)
			datacontinue = 2;
		break;
	}

	*texlen = datalen;
	return datacontinue;
}


/*置软件升级标志*/
void FileMid_ProgramUpgrade(bool yes)
{
	f_ProgramUpgrade = yes;
	if(yes == true)
	{
		flashshell_Erase(0x80000/*MEMADDR_FLASH_PRG1, MEMSIZE_FLASH_PRG*/,0x58000);//擦除PRG1和CBK1
	}	
}

/************************************************************/
/************************************************************/


uint8 dpaH;

void FileMid_LogInit(void)
{
	dpaH = db_GetDpaHand();
}


typedef struct
{
	SYS_TIME64	time;//时间，定义在sysTimer.h中
	uint8	val;//数值 1合 0分
	uint32 inf;
	int16 otherpnt;//双点中的另一个点，预期的下一个soe的系统点号
}struSoeLog;

void FileMid_LogSave(void)
{
	int i, j;
	struDpa10xFrm *pfrm;
	void *ppntcfg = NULL;
	struDpa10xIt_Cfg *pItCfg;
	struDpa10xMe_Cfg *pMeCfg;
	struDpa10xDp_Cfg *pDpCfg;
	struDpa10xSp_Cfg *pSpCfg;

	struSoeLog soe1;
	SOEEVENT *psoe;

	uint32 inf;
	int8 frm;
	int16 pnt, otherpnt;
	bool flag=0;//表示有临时soe未保存


	//定点5分钟，极值1秒查1次，0点存。
	//1.定点记录, 时间到遍历浮点遥测配置表，逐点保存
	//2.极值记录， 遍历浮点遥测配置表，逐点处理最大值，在0:0:0逐点保存
	pfrm = dpa101appl.pport->pfrm;
	for (i = 0; i < dpa101appl.pport->frmnum; i++, pfrm++)
	{
		if (pfrm->pcfg->frmtype == M_ME_NC_1)//仅处理浮点遥测帧
		{
			inf = pfrm->pcfg->frminf;//起始信息对象
			pMeCfg = (struDpa10xMe_Cfg *)pfrm->ppntcfg;//指向flash中的浮点遥测配置表
			for (j = 0; j < pfrm->pntnum; j++, pMeCfg++, inf++)
			{
				int16 syspnt = pMeCfg->syspnt;
				float val = db_GetAi(dpaH, syspnt);
				//......save(inf,val)
			}
		}
	}

	//电能量冻结
	pfrm = dpa101appl.pport->pfrm;
	for (i = 0; i < dpa101appl.pport->frmnum; i++, pfrm++)
	{
		if (pfrm->pcfg->frmtype == M_IT_NB_1)//仅处理浮点累积量
		{
			pItCfg = (struDpa10xMe_Cfg *)pfrm->ppntcfg;//指向flash中的浮点遥测配置表
			for (j = 0; j < pfrm->pntnum; j++, pItCfg++, inf++)
			{
				int16 syspnt = pItCfg->syspnt;
				float val = db_GetPa(syspnt);
				//......save(inf,val)
			}
		}
	}

	//事件触发的记录：
	//1.SOE记录 维护口+ 101
	//2.trd记录 维护口+ 101
	//3.ulog  仅101
	//4.prtlog 仅维护口
	pfrm = dpa101appl.pport->pfrm;//指向第1帧
	while (psoe = db_GetSoe(dpaH))
	{
		//1.save soeevent, 单纯把event记录下来，给维护口读取，维护口将按记录读取，与文件无关
		//包括soe trd prtlog
		;//save_raw_soe(*psoe);


		//2.save soe for 101
		//包括soe trd ulog, 仅soe有双点处理, ulog不需要search到inf,直接保存
		//这里仅以soe为例，以soe1为临时变量(struSoeLog soe1)
		ppntcfg = dpa10x_SearchSyspntInFrms(dpa101appl.pport, psoe->pnt, TypeidDi, &frm, &pnt, &otherpnt, &inf);//查找系统点对应的点配置等，并得到inf

		if (ppntcfg == NULL)//没找到，说明点不在101配置中
		{
			if(flag)
			{
				flag = 0;
				;//savesoelog(soe1.inf, soe1.val, soe1.time);
			}
			continue;
		}


		if (pfrm[frm].pcfg->frmtype == M_DP_TA_1 || pfrm[frm].pcfg->frmtype == M_DP_TB_1)	//找到在双点帧里
		{	
			pDpCfg = (struDpa10xDp_Cfg *)ppntcfg;//指向点配置

			if(flag == 1)//有临时变量中未存SOE
			{
				if(psoe->pnt == soe1.otherpnt)//找到第2条soe，丢弃上次soe，保存本次soe
				{
					flag = 0;
					soe1.time = psoe->time;
					soe1.val = ((db_GetDi(pDpCfg->syspnton)&1)<<1) | (db_GetDi(pDpCfg->syspntoff)&1);
					soe1.inf = inf;
					;//savesoelog(soe1.inf, soe1.val, soe1.time);
				}
				else //不是同一个双点双点遥信点，则保存临时变量soe，然后把本soe推入临时变量
				{
					;//savesoelog(soe1.inf, soe1.val, soe1.time);//保存上次soe

					flag = 1;
					soe1.time = psoe->time;
					soe1.val = ((db_GetDi(pDpCfg->syspnton)&1)<<1) | (db_GetDi(pDpCfg->syspntoff)&1);
					soe1.inf = inf;
					soe1.otherpnt = otherpnt;
				}
			}
			else //没有未保存临时变量，把本soe推入临时变量soe1中
			{		
				flag = 1;
				soe1.time = psoe->time;
				soe1.val = ((db_GetDi(pDpCfg->syspnton)&1)<<1) | (db_GetDi(pDpCfg->syspntoff)&1);
				soe1.inf = inf;
				soe1.otherpnt = otherpnt;
			}
		}
		else //单点帧的点，直接保存log
		{
			if(flag)
				;//savesoelog(soe1.inf, soe1.val, soe1.time);

			pSpCfg = (struDpa10xSp_Cfg *)ppntcfg;
			flag = 0;
			soe1.time = psoe->time;
			soe1.val = (db_GetDi(pSpCfg->syspnt)&1)<<1;//把单点SIQ做成双点DIQ
			soe1.inf = inf;
			;//savesoelog(soe1.inf, soe1.val, soe1.time);
		}
	}

	if(flag)
		;//savesoelog(soe1.inf, soe1.val, soe1.time);
}
