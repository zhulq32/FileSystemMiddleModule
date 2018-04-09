
#ifndef __COMMONEX_H__
#define __COMMONEX_H__

//#include "stdint.h"

typedef unsigned char		uint8;
typedef signed char	    int8;
typedef unsigned short	uint16;
typedef signed short		int16;
typedef unsigned int	uint32;
typedef signed int		int32;

typedef unsigned char		UINT8;
typedef signed char	    INT8;
typedef unsigned short	UINT16;
typedef signed short		INT16;
typedef unsigned int	UINT32;
typedef signed int		INT32;
typedef unsigned long long	UINT64;

//typedef unsigned char     BOOL;

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif


// digital input status
#define CLOSE		1
#define OPEN		0
#define ON		1
#define OFF		0

#ifndef NULL
#define NULL		0
#endif

#ifndef min
#define min(a,b)		(((a)<(b))?(a):(b))
#endif

#ifndef max
#define max(a,b)		(((a)>(b))?(a):(b))
#endif

#define _KILO(n)		((n)<<10)
#define _MEGA(n)		((n)<<20)

#define _BIT(n)		(1<<(n))

#define SIZE_ARY(a)		(sizeof(a)/sizeof(a[0]))

#endif