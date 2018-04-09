
/*  IEC101 protocol definition */
/************************************************************************/
#define START1		0x10
#define START2		0x68
#define STOP		0x16
/***********************************************************************/
#define SEND_RESET_LINK 	0
#define SEND_RESET_PROCESS	1
#define SEND_TEST_LINK		2
#define SEND_REPLY_DATA		3
#define SEND_NOREPLY_DATA	4
#define REQUEST_ACCESS_BIT	8
#define REQUEST_LINK_STATUS	9
#define REQUEST_CLASS1_DATA	10
#define REQUEST_CLASS2_DATA	11

#define CONFIRM_SEND		0
#define CONFIRM_LINK_BUSY	1
#define RESPOND_DATA		8
#define RESPOND_NODATA		9
#define RESPOND_LINK_STATUS	11
/***************************************************************************/

/*  IEC104 protocol definition */
/************************************************************************/
#define STARTDT_ACT 0X04
#define STARTDT_CON 0X08
#define STOPDT_ACT  0X10
#define STOPDT_CON  0X20
#define TESTFR_ACT  0X40
#define TESTFR_CON  0X80

#define DPA104_SEND_INTERVAL	500
#define FORMAT_I	0X0
#define FORMAT_S	0X1
#define FORMAT_U	0X3
#define FRAME_LEN 240

#define TCP104_TCPPORT	2404
/************************************************************************/



// TYPE IDENTIFICATION		:= UI8(1..8)<1..255>
// Process information in monitor direction
#define M_SP_NA_1	1	// single-point information
#define M_SP_TA_1	2	// single-point information
#define M_DP_NA_1	3	// dobule-point information
#define M_DP_TA_1	4	// dobule-point information
#define M_ST_NA_1	5	// step position information
#define M_ST_TA_1	6	// step position information
#define M_BO_NA_1	7	// bitstring of 32 bit
#define M_BO_TA_1	8	// bitstring of 32 bit
#define M_ME_NA_1	9	// measured value, normalized value
#define M_ME_TA_1	10	// measured value, normalized value
#define M_ME_NB_1	11	// measured value, scaled value
#define M_ME_TB_1	12	// measured value, scaled value
#define M_ME_NC_1	13	// measured value, short floating point number
#define M_ME_TC_1	14	// measured value, short floating point number
#define M_IT_NA_1	15	// integrated totals
#define M_IT_TA_1	16	// integrated totals
#define M_EP_TA_1	17	// event of protection equipment with time tag
#define M_EP_TB_1	18	// packed start events of protection with time tag
#define M_EP_TC_1	19	// packed output circuit information of protection equipment with time tag
#define M_PS_NA_1	20	// packed single-point information with status change detection
#define M_ME_ND_1	21	// measured value, normalized value without quality descriptor
// Process information in monitor direction with time tag CP56Time2a
#define M_SP_TB_1	30	// single-point information
#define M_DP_TB_1	31	// double-point information
#define M_ST_TB_1	32	// step position information
#define M_BO_TB_1	33	// bitstring of 32 bit
#define M_ME_TD_1	34	// measured value, normalized value
#define M_ME_TE_1	35	// measured value, scaled value
#define M_ME_TF_1	36	// measured value, short floating point number
#define M_IT_TB_1	37	// integrated totals
#define M_EP_TD_1	38	// event of protection equipment
#define M_EP_TE_1	39	// packed start events of protection equipment
#define M_EP_TF_1	40	// packed output circuit information of protection equipment
// Process information in control direction
#define C_SC_NA_1	45	// single command
#define C_DC_NA_1	46	// double command
#define C_RC_NA_1	47	// regulating step command
#define C_SE_NA_1	48	// set point command, normalized value
#define C_SE_NB_1	49	// set point command, scaled value
#define C_SE_NC_1	50	// set point command, short floating point number
#define C_BO_NA_1	51	// bitstring of 32 bits
// Process information in control direction with time tag CP56Time2a
#define C_SC_TA_1	58	// single command
#define C_DC_TA_1	59	// double command
#define C_RC_TA_1	60	// regulating step command
#define C_SE_TA_1	61	// set point command, normalized value
#define C_SE_TB_1	62	// set point command, scaled value
#define C_SE_TC_1	63	// set point command, short floating point number
#define C_BO_TA_1	64	// bitstring of 32 bits
// System information in monitor direction
#define M_EI_NA_1	70	// end of initialization
// System information in control direction
#define C_IC_NA_1	100	// interrogation command
#define C_CI_NA_1	101	// counter interrogation command
#define C_RD_NA_1	102	// read command
#define C_CS_NA_1	103	// clock synchronization command
#define C_TS_NA_1	104	// test command
#define C_RP_NA_1	105	// reset process command
#define C_CD_NA_1	106	// delay acquisition command
#define C_TS_TA_1	107	// test command with CP56Time2a

// Parameter in control direction
//#define P_ME_NA_1	110	// parameter of measured value, normalized value
//#define P_ME_NB_1	111	// parameter of measured value, scaled value
//#define P_ME_NC_1	112	// parameter of measured value, short floating-point number
//#define P_AC_NA_1	113	// parameter activation

//	File transfer
//#define F_FR_NA_1	120	// file ready
//#define F_SR_NA_1	121	// section ready
//#define F_SC_NA_1	122	// call directory, select file, call section
//#define F_LS_NA_1	123	// last section, last segment
//#define F_AF_NA_1	124	// ack file, ack section
//#define F_SG_NA_1	125	// segment
//#define F_DR_TA_1	126	// directory

// 电科院扩展部分
#define M_FT_NA_1	42	// 故障事件信息
#define M_IT_NB_1	206	// 浮点累计量
#define M_IT_TC_1	207	// 浮点累计量 with timestamp
#define C_SR_NA_1	200	// 切换定值区
#define C_RR_NA_1	201	// 读定值区号
#define C_RS_NA_1	202	// 读参数和定值
#define C_WS_NA_1	203	// 写参数和定值
#define F_FR_NA_1	210	// 文件传输
#define F_SR_NA_1	211	// 软件升级



/***************************************************************************/
// VARIABLE STRUCTURE QUALIFIER=CP8{SQ,Number}
// Number=N		:= UI7(1..7)<0..127>
// <0>			ASDU contains no INFORMATION OBJECT
// <1..127>		number of INFORMATION OBJECTs or ELEMENTs
// SQ=Single/sequence	:= BS1(8)<0..1>
// <0>	addressing of an individual element or combination of elements in
//	a number of INFORMATION OBJECTs of the same type
// <1>	addressing of a SEQUENCE INFORMATION ELEMENTs in one object
// SQ<0> and N<0-127>	number of INFORMATION OBJECTs
// SQ<1> and N<0-127>	number of INFORMATION OBJECTs of single object per
//			ACDU
/***********************************************************************/
// CAUSE OF TRANSMISSION=CP16{Cause,P/N,T,Originator Address(opt)}
// CAUSE=UI6(1..6)<0..63>	0:not difined, 1-63:number of cause
// P/N=BS1[7]<0..1>		0:positive confirm, 1:negative confirm
// T=test=BS1[8]<0..1>		0:no test, 1:test
#define PER_CYC 	1	// periodic, cyclic
#define BACK		2	// background scan
#define SPONT		3	// spontaneous
#define INIT		4	// initilize
#define REQ		5	// request or requested
#define ACT		6	// activation
#define ACTCON		7	// activation confirmation
#define DEACT		8	// deactivation
#define DEACTCON	9	// deactivation confirmation
#define ACTTERM 	10	// activation termination
#define RETREM		11	// return information caused by a remote command
#define RETLOC		12	// return information caused by a local command
#define FILE_TRANS	13	// file transfer
#define INTROGEN	20	// interrogated by general interrogation
#define CNINTROGEN      1   //   INterrogate for general counter
#define INTRO1		21	// interrogated by group1 interrogation
#define INTRO2		22	// interrogated by group2 interrogation
#define INTRO3		23	// interrogated by group3 interrogation
#define INTRO4		24	// interrogated by group4 interrogation
#define INTRO5		25	// interrogated by group5 interrogation
#define INTRO6		26	// interrogated by group6 interrogation
#define INTRO7		27	// interrogated by group7 interrogation
#define INTRO8		28	// interrogated by group8 interrogation
#define INTRO9		29	// interrogated by group9 interrogation
#define INTRO10 	30	// interrogated by group10 interrogation
#define INTRO11 	31	// interrogated by group11 interrogation
#define INTRO12 	32	// interrogated by group12 interrogation
#define INTRO13 	33	// interrogated by group13 interrogation
#define INTRO14 	34	// interrogated by group14 interrogation
#define INTRO15 	35	// interrogated by group15 interrogation
#define INTRO16 	36	// interrogated by group16 interrogation
#define REQCOGEN	37	// requested by general counter request
#define REQCO1		38	// requested by group1 counter request
#define REQCO2		39	// requested by group2 counter request
#define REQCO3		40	// requested by group3 counter request
#define REQCO4		41	// requested by group4 counter request

/***********************************************************************/
// 101 controlword bits
#define	DIR  0x80
#define PRM  0x40
#define FCB  0x20
#define FCV  0x10
#define	ACD  0x20
#define DFC  0x10

/*******************************/
#define QULI_SINGLE	0x0
#define QULI_SEQUENCE	0x80

#define DP_ON	0X2
#define DP_OFF	0X1



/***********************************************************************/



typedef enum
{
	TypeidDi,
	TypeidAi,
	TypeidDo,
	TypeidPa,
	TypeidPara,
}enumTYPID_GRP;


#define RXSIZE	263
#define TXSIZE  272
#define CLASS1BUFSIZE	1024
#define CLASS2BUFSIZE	128

#define DPA10x_SUPPORT_LOG	
/***************************************************************/

//配置structures, 在cpu internal flash中配置数据的结构格式

typedef struct  //单点遥信SPTB，长时标，SPTA，短时标
{
	int16 syspnt;			//系统点号
	uint16 option;			//选项
	int8 rev[4];			//保留
}struDpa10xSp_Cfg;

typedef struct  //双点遥信DPTB，长时标，DPTA短时标
{
	int16 syspnton;			//合位系统点号
	int16 syspntoff;		//分位系统点号
	uint16 option;			//选项
	int8 rev[2];			//保留
}struDpa10xDp_Cfg;

typedef struct  //遥测MENA MENB MENC   Na规一化 Nb标度化 Nc浮点
{
	int16 syspnt;			//系统点号
	uint16 option;			//选项
	float multiply;			//系数  发往主站的值=数据库的值*系数
	int8 rev[4];			//保留
}struDpa10xMe_Cfg;


typedef struct  //电能量ITNA
{
	int16 syspnt;			//系统点号
	uint16 option;			//选项
	float multiply;			//系数	发往主站的值=数据库的值*系数
	int8 rev[4];			//保留
}struDpa10xIt_Cfg;

typedef struct  //单点SCNA、双点DCNA遥控
{
	int16 syspnton;			//控合系统点号
	int16 syspntoff;		//控分系统点号
	uint16 option;			//选项
	int8 rev[2];			//保留
}struDpa10xDc_Cfg;

typedef struct  //设点SENA SENB SENC  Na规一化 Nb标度化 Nc浮点 遥调
{
	int16 syspnt;			//系统点号
	uint16 option;			//选项
	float multiply;			//系数  置入数据库的值=主站下发值*系数
	int8 rev[4];			//保留
}struDpa10xSe_Cfg;

typedef struct //定值，参数
{
	INT16 syspnt;			//系统点号
	INT8  datatype;			//0投退控制字 1定值 2运行参数 
	INT8 Tag;				//104中的类型TAG
	INT8 Len;				//104中的参数长度
	char Rev[3];
	float multiply;			//系数  置入数据库的值=主站下发值*系数
}struDpa10xPara_Cfg;


typedef struct
{
	int16 grpnr;			//组号 1～16
	uint16 frmtype;		//帧类型，SPTB DPTB MENA MENB MENC ITNA SCNA DCNA SENA SENB SENC
	int16 startpnt;			//各类型相关点表的起始点序号
	int16 endpnt;			//各类型相关点表的结束点序号
	uint8	rev[4];
}struDpa10xGrp_Cfg;

//----
typedef struct
{
	uint16 frmtype;		//帧类型 
	uint16 pntnum1frm;		//每一帧的点数，0表示无限制
	UINT32 frminf;		//起始信息号
	UINT8  rev[6];			//预留

	uint16 pntnum;			//本类型帧总点数
	int16 pntcfg;		//pntcfg struct根据frmtype定
}struDpa10xFrm_Cfg;

//----
typedef struct
{
	uint32 portsize;		//通道的配置长度（字节数）

	union{
		int32 portnr1;			//装置端口
		uint32 remoteip3;       //远方主IP
	};
	union{
		int32 portnr2;			//装置端口
		uint32 remoteip4;       //远方备IP
	};
	union{
		uint8 baud1;		    //装置波特率
		int8 localip1;			//本地主IP
	};
	union{
		uint8 baud2;		    //装置波特率
		int8 localip2;			//本地备IP
	};
	union{
		int8 parity;            //装置校验位
		int8 rev2;
	};
	uint8 commkey;   

	uint8 commaddrlen;
	int8 linkaddrlen;		//链路地址字节数 1～2
	int8 inflen;			//信息体地址字节数 2～3
	int8 causelen;			//传送原因字节数 1～2

	int16	linkaddr;		//本通道链路地址
	int16 commaddr;		//本通道公共地址

	int8 timesynen;		//是否由本通道主站对时
	int8 soecos;			//变位传送方式 0不送变位 1仅送SOE 2仅送COS 3SOE+COS
	uint16 giinterval;	//周期发送的间隔时间ms  0不支持周期发送

	uint16 ciinterval;	//周期发送的间隔时间ms  0不支持周期发送
	int16 remotectrlen;		//遥控软压板对应的遥信系统点号

	int8 rev1[8];

	uint32 frmnum;
	struDpa10xFrm_Cfg frmcfg;
}struDpa10xPort_Cfg;

//----

typedef struct
{
	uint16 appdelay;		//应用延时多少秒后执行
	uint16 portnum;			//通讯通道数
	struDpa10xPort_Cfg  portcfg;
}struDpa10xApp_Cfg;



////////////////////////////////////////////////////////////

//运行时structures, 在ram中
#define DPA10x_DOSTS_SELECTED	1
#define DPA10x_DOTIMEOUT		60000
typedef struct
{
	uint32 inf;//信息号
	uint8  val;//0分或1合
	uint8  sts;//状态标识
	uint32 timetick;//遥控开始时间
}struDpa10xDoevent;

typedef struct
{
	SOEEVENT soeevent;//SOE信息
	int8 frm;//在PORT中的帧偏移
	int16 pnt;//在FRM中的点偏移
	int16 otherpnt;//如果为真双点，存放本变位点对应的另一个点
}struDpa10xSoe;

typedef struct
{
	int16 pntnum;//预置的点数
	uint8 pack[256];//预置的ASDU数据包
	uint8 f_select;//已选择标志
	uint32 selecttime;
}struDpa10xSzDzEvent;



//////////////////////////////////
typedef struct
{
	struDpa10xFrm_Cfg		*pcfg;//帧配置
	int16		pntnum;//本帧点数
	void		*ppntcfg;//点配置
}struDpa10xFrm;



typedef struct
{
	struDpa10xPort_Cfg	*pcfg;//port配置
	int16				frmnum;//本port帧数
	struDpa10xFrm		*pfrm;
	int32				ginum;//分组召唤组数
	struDpa10xGrp_Cfg	*pgicfg;//分组召唤配置
	int32				cinum;//累积量分组召唤组数
	struDpa10xGrp_Cfg	*pcicfg;//累积量分组召唤配置

	uint16	acd_count;	//未上送的1类数据包数
	uint16 noacknum;//发出后未被确认的CLASS1数据包数
	uint16	class2_count;	//未上送的2类数据包数
	uint8  class1_buf[CLASS1BUFSIZE];//一类数据BUFFER
	uint8  *class1_buf_in, *class1_buf_out, *class1_buf_tmpout;//存放1类数据的BUF class1_buf_in:Wr指针，class1_buf_out:Re指针；buf_tmpout：上送但未回应buf指针
	uint8  class2_buf[CLASS2BUFSIZE];
	uint8  *class2_buf_in, *class2_buf_out;

	uint8	dpaH;//DPA号
	uint8	rbuf[RXSIZE], tbuf[TXSIZE];
	int16	rlen, tlen;

	uint8 gi_grp;//非零值表示正在总召，此处存放总召报文的QOI 20:总 21:组1 36:组16
	int8 gi_currfrm;//当前扫描的帧
	int16 gi_currpnt, gi_endpnt;//当前扫描的点， 本帧应上送的最后一个点

	uint8 ci_grp;//QCC 1:组1 2:组2... 5:总
	int8 ci_currfrm;
	int16 ci_currpnt, ci_endpnt;

	int8 ai_currfrm;//变化遥测扫描的当前帧
	int16 ai_currpnt;//当前点

	uint32 scan_timetick;//每150ms执行一次scan(), 完成soe扫描等
	uint8	portsts1;//主通讯口状态 0未初始化，ERR错误，RUN工作中
	uint8	portsts2;
	uint8	activeport;//仅指接收报文是由哪个口收到的。对101来说哪个口进哪个口回；
	//对104来说U帧也是哪个口进哪个口回，其他帧哪个口STARTDT就发哪个，与从那里接收无关。

	uint32	porttick1;//用于通讯口状态(仅主通道),对101通讯口发出帧时的TICK，接收到的可能是问别人的啦，用来判断通讯口状态； 对104为通讯口接收到报文的TICK，104是一直主动发出的哦

	struDpa10xDoevent doevent;//存放遥控选择命令
	uint8 f_sendframe;//应答帧类型
	uint8 f_resend;//resend次数

	struDpa10xSzDzEvent SzDzEvent;//参数预置的整个报文
	uint16 para_sn;


	uint8 last_remote_fcb;//仅101

	uint16 remote_receive_sn, remote_send_sn;//仅104，接收序号等
	uint16 local_receive_sn, local_send_sn;
	uint8 startdt1;//START DT标识
	uint8 startdt2;
	int	sockid1;
	int	sockid2;
	uint32 LocalIp1Addr;
	uint32 LocalIp2Addr;
	uint8 sockinf1;//链接建立时填入对应的SockInf[]下标，在主动close时用来关闭SockInf.used
	uint8 sockinf2;

	int  test_t1;   //发送或测试APDU的超时的时间
	uint8 k_num;  //某一时间内未被主站确认的连续编号的I帧的最大数目
	uint32 send_timetick;//发送间隔

	uint16 Mess_type;
	uint8 App_type;
	uint8 dat_rbuf[256],dat_tbuf[256]; //发送，接收的封装数据域buf（不加密数据），
	uint8 Master_R1[8],Send_Rand[8];

	uint8 F_SendCER;//是否有证书包需要上送
	uint16 dat_tlen,dat_rlen;//发送,接收的封装数据域长度
	uint8 F_NeedACK;
	uint8 F_CONFIRM;  //是否认证标识
	uint32 sendcer_tick;
	uint32 Tool_Rectick;

#ifdef DPA10x_SUPPORT_LOG
	uint8 f_FileContinue;//bit0有后续CLASS1 bit1有后续CLASS2
#endif

	uint8 DTStart;
}struDpa10xPort;


typedef struct
{
	struDpa10xApp_Cfg *pcfg;
	int16 portnum;
	struDpa10xPort *pport;
}struDpa10xApp;

extern struDpa10xApp dpa101appl;


/*
input:
pport: &dpa101appl.pport[0]
syspnt:要search的系统点号
typgrp:遥信、遥测的大类

output:
frm: 填入所在pport的帧偏移
pnt: 填入该帧点偏移
otherpnt: 当查找到的是双点遥信帧时，填入另一个系统点的syspnt；
inf: 填入该点信息对象号

return:
指向点配置（如struDpa10xSp_Cfg *）的指针
*/
extern void* dpa10x_SearchSyspntInFrms(struDpa10xPort *pport, int16 syspnt, enumTYPID_GRP typgrp, int8 *frm, int16 *pnt, int16 *otherpnt, uint32 *inf);

