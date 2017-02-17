#ifndef FLX_TYPES_HEADR_FILE
#define FLX_TYPES_HEADR_FILE

#include "flxdefines.h"
//////////////////////////////////////////
//类型：FLXHandle，FLXHModule，FLXHProc
//作用：句柄
/////////////////////////////////////////

#ifdef FLXWIN//windows

#include "windows.h"

typedef HANDLE FLXHandle;
typedef HMODULE FLXHModule;
typedef FARPROC FLXHProc;
#define FLXAPI WINAPI

#elif defined(FLXLINUX) || defined(FLXUNIX)		 //linux unix ect

typedef void* FLXHandle;
typedef FLXHandle FLXHModule;
typedef FLXHandle FLXHProc;
#define FLXAPI

#endif


typedef int				FLXInt32;
typedef short			FLXInt16;
typedef unsigned int	FLXUnint32;
typedef unsigned short	FLXUnint16;
typedef char			FLXChar;
typedef unsigned char	FLXByte;
typedef FLXByte			FLXBool;

#ifndef TRUE
#define TRUE 1
#endif 

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL (void *)0
#endif

#endif

#define FLX_INVALID_HANDLE (FLXHandle)-1
/////////////////////////////////////
//类型：FLXDateTime
//作用：记录网络终端信息,Ip:端口号
//成员：
//year:unsigned short,年
//month:unsigned char,月
//day:unsigned char,日
//hour:unsigned char,小时
//minute:unsigned char,分钟
//second:unsigned char,秒
//millisecond:unsigned short,毫秒
//dayOfWeek: unsigned char,星期
////////////////////////////////////
typedef struct tag_flxDateTime
{
    FLXUnint16 year;
    FLXUnint16 month;
    FLXUnint16 day;
    FLXUnint16 hour;
    FLXUnint16 minute;
    FLXUnint16 second;
    FLXUnint16 millisecond;
    FLXUnint16 dayOfWeek;
}FLXDateTime,*PFLXDateTime;

#endif
