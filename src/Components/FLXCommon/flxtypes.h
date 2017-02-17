#ifndef FLX_TYPES_HEADR_FILE
#define FLX_TYPES_HEADR_FILE

#include "flxdefines.h"
//////////////////////////////////////////
//���ͣ�FLXHandle��FLXHModule��FLXHProc
//���ã����
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
//���ͣ�FLXDateTime
//���ã���¼�����ն���Ϣ,Ip:�˿ں�
//��Ա��
//year:unsigned short,��
//month:unsigned char,��
//day:unsigned char,��
//hour:unsigned char,Сʱ
//minute:unsigned char,����
//second:unsigned char,��
//millisecond:unsigned short,����
//dayOfWeek: unsigned char,����
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
