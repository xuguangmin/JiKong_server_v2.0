#ifndef FLX_NET_TYPES_HEADER_FILE
#define FLX_NET_TYPES_HEADER_FILE

////////////////////////////////////////////////////////////////////////////////////
//文件名称：tcptypes.h
//文件说明：
//         FLX网络通信相关类型定义文件
//版本：
//         v 1.0
//创建时间:
//2010年9月14日
//创建人：
//         韩明
//修改人：
//         陈志涛
//修改时间：
//      2010年11月24日
//修改内容：
//		添加socket宏定义
///////////////////////////////////////////////////////////////////////////////////
#include "flxdefines.h"
#include "flxtypes.h"

#define PACKAGE_SIZE        1024
#define RECV_DATA_BUFFER    1536

#ifdef FLXWIN //windows
#include <winsock.h>
#pragma comment(lib, "Ws2_32.lib")
typedef SOCKET FLXSocket;

#elif defined(FLXUNIX) || defined(FLXLINUX) //linux or unix
#include <sys/socket.h>
typedef FLXInt32 FLXSocket;

#define INVALID_SOCKET             -1 //(FLXSocket)(~0)
#define SOCKET_ERROR			   (-1)

#endif

/////////////////////////////////////
//类型：FLXEndIpPoint
//作用：记录网络终端信息,Ip:端口号
//成员：
//ipAddress:char *,IP地址
//port:uint 端口号
////////////////////////////////////
typedef struct tag_flxIpEndPoint
{
	FLXChar  ipAddress[16];
	//char * ipAddress;
	FLXUnint32 port;
}FLXIpEndPoint,*PFLXIpEndPoint;

#endif //end of ifndef FLX_NET_TYPES_HEADER_FILE
