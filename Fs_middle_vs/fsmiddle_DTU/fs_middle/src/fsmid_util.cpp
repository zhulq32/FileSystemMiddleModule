#include "..\inc\fsmid_def.h"
#include <string.h>

//static const unsigned char ctbl_counter[8] = {0xFF,0x7F,0x3F,0x1F,0x0F,0x07,0x03,0x01};

unsigned int bitmap2number(unsigned char* bitmap, unsigned int length)
{
	unsigned int result = 0;
	int j;

	while(length)
	{
		if(*bitmap == 0)
			result += 8;
		else
		{
			for(j = 0; j < 8; j++ )
			{
				if(*bitmap == (0xFF>>j))
					return (result + j);
			}
			fsmid_assert(0,__FILE__,__LINE__);
		}
		bitmap++;
		length--;
	}

	return result;
}

void number2bitmap(unsigned int number, unsigned char* bitmap, unsigned int length)
{
	fsmid_assert(number <= (length*8),__FILE__,__LINE__);

	memset(bitmap,0,number/8);
	bitmap += number/8;
	length -= number/8;

	if(number%8)
	{
		*bitmap = 0xFF>>(number%8);
		bitmap++;
		length--;
	}
	memset(bitmap,0xFF,length);
}

bool systimeSameDay(const SYS_TIME64 *tm1, const SYS_TIME64 *tm2)
{
#ifdef FAST_MODE
	if(tm1->day != tm2->day)
		return false;
	if(tm1->hour != tm2->hour)
		return false;
	if(tm1->min != tm2->min)
		return false;
	return true;
#else
	if(tm1->year != tm2->year)
		return false;
	if(tm1->mon != tm2->mon)
		return false;
	if(tm1->day != tm2->day)
		return false;
	return true;
#endif
}

#include <time.h>
static const int MON1[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};   //平年  
static const int MON2[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};   //闰年  
static const int FOURYEARS = (366 + 365 +365 +365); //每个四年的总天数  
static const int SEC_PER_DAY = 24*3600;   //每天的秒数  

bool unixSameDay(unsigned int tm1, unsigned int tm2)
{
	tm1 /= SEC_PER_DAY;
	tm2 /= SEC_PER_DAY;
	if(tm1 == tm2)
		return true;
	return false;
}

void __get_day(int nDays, SYS_TIME64 *systime, bool IsLeapYear)  
{  
	const int *pMonths = IsLeapYear?MON2:MON1;  
	//循环减去12个月中每个月的天数，直到剩余天数小于等于0，就找到了对应的月份  
	for ( int i=0; i<12; ++i )  
	{  
		int nTemp = nDays - pMonths[i];  
		if ( nTemp<=0 )  
		{  
			systime->mon = i+1;  
			if ( nTemp == 0 )//表示刚好是这个月的最后一天，那么天数就是这个月的总天数了  
				systime->day = pMonths[i];  
			else  
				systime->day = nDays;  
			break;  
		}  
		nDays = nTemp;  
	}  
}  

unsigned int time_sys2unix(const SYS_TIME64 *systime)
{
	struct tm stm; 
	memset(&stm,0,sizeof(stm)); 

	stm.tm_year=systime->year - 1900 + 2000; 
	stm.tm_mon=systime->mon-1; 
	stm.tm_mday=systime->day; 
	stm.tm_hour=systime->hour; 
	stm.tm_min=systime->min; 
	stm.tm_sec=systime->sec; 

	return (unsigned int)_mktime32(&stm);  
}

void time_unix2sys(unsigned int unix, SYS_TIME64 *systime)
{
	unix += 8*3600;//北京时间 +8小时
	int nDays = unix/SEC_PER_DAY + 1;    //time函数获取的是从1970年以来的毫秒数，因此需要先得到天数  
	int nYear4 = nDays/FOURYEARS;   //得到从1970年以来的周期（4年）的次数  
	int nRemain = nDays%FOURYEARS;  //得到不足一个周期的天数  
	systime->year = 1970 + nYear4*4 - 2000;  
	bool bLeapYear = false;
	if ( nRemain<365 )//一个周期内，第一年  
	{//平年  

	}  
	else if ( nRemain<(365+365) )//一个周期内，第二年  
	{//平年  
		systime->year += 1;  
		nRemain -= 365;  
	}  
	else if ( nRemain<(365+365+365) )//一个周期内，第三年  
	{//平年  
		systime->year += 2;  
		nRemain -= (365+365);  
	}  
	else//一个周期内，第四年，这一年是闰年  
	{//润年  
		systime->year += 3;  
		nRemain -= (365+365+365);  
		bLeapYear = true;  
	}  
	__get_day(nRemain, systime, bLeapYear);
	systime->hour = (unix % SEC_PER_DAY)/3600;
	systime->min = (unix% 3600)/60;
	systime->sec = unix % 60;
	systime->msec = 0;
}
