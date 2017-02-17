#ifndef TCP_SERVER_HEAHER_FILE
#define TCP_SERVER_HEAHER_FILE
////////////////////////////////////////////////////////////////////////////////////
//文件名称：tcpserver.h
//文件说明：
//			 Tcp服务器端组件接口函数头文件
//版本：
//			v 1.0
//创建时间:
//			2010年9月13日
//创建人：
//			 韩明
//修改人：
//			陈志涛(请如实填写以便好维护)
//修改时间：
//			2010年11月23日(请如实填写以便好维护)
//修改内容：
//			添加window平台下socket及代码优化
///////////////////////////////////////////////////////////////////////////////////


#ifdef __cplusplus 
extern "C" {
#endif

#include "flxtypes.h"
#include "flxnettypes.h"
#include <stdio.h>
#include <string.h>

#ifdef FLXWIN
#include <windows.h>
#include <winbase.h>
#include <stddef.h>
#include <string.h>

#elif defined(FLXLINUX) || defined(FLXUNIX)
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif


#ifdef DEBUG
#define TCP_SERVER_LIB_DEBUG(args)	(printf("[tcp_server_lib:]"), printf args)
#else
#define TCP_SERVER_LIB_DEBUG(args)
#endif

//////////////////////////////////////////////////////////////////////////
//   函数名称：tcp_server_acceptClientCallBack
//   功能：接受客户端连接回调函数
//   参数：
//        serverSock:FlxSocket，客户端套接字
//        endpoint:FLXEndPoint *，客户端
//   返回值：
//       成功，则返回0,否则返回-1
/////////////////////////////////////////////////////////////////////////
typedef FLXInt32 (*tcp_server_acceptClientCallBack)(FLXSocket *sock, FLXIpEndPoint* ipEndpoint);

////////////////////////////////////////////////////////////////////////////
//   函数名称:tcp_server_initialize
//   功能：初始化Tcp服务器端组件
//   参数：无
//   返回值：
//       如果初始化成功，则返回已初始化的套接字,否则返回-1
///////////////////////////////////////////////////////////////////////////
FLXSocket tcp_server_initialize(void);

////////////////////////////////////////////////////////////////////////////
//   函数名称：tcp_server_start
//   功能：启动Tcp监听，开始接受客户端连接
//   参数：
//       sock:FLXInt32,服务器端套接字
//       port:unsigned FLXInt32,端口号
//       callback:tcp_server_acceptClientCallBack
//   返回值：
//   如果启动成功则返回0,否则返回-1
///////////////////////////////////////////////////////////////////////////
FLXInt32 tcp_server_start(FLXSocket sock, FLXUnint32 port, tcp_server_acceptClientCallBack callback);

//////////////////////////////////////////////////////////////////////////
//   函数名称：tcp_server_sendData
//   功能：发送数据给客户端sock
//   参数：
//        sock:FLXInt32，客户端套接字
//        data:FLXChar *，数据
//        len:FLXInt32，data的长度
//   返回值：
//       如果发送成功，则返回0,否则返回-1
/////////////////////////////////////////////////////////////////////////
FLXInt32 tcp_server_sendData(FLXSocket sock, FLXByte *data, FLXUnint32 dataLength);

//////////////////////////////////////////////////////////////////////////
//   函数名称：tcp_server_readData
//   功能：从客户端套接字sock,读取数据
//   参数：
//        sock:FLXInt32，客户端套接字
//        data:FLXChar *，数据
//        len:FLXInt32，data的长度
//   返回值：
//       如果读取成功，则返回读取数据的长度,否则返回0，出错返回-1
/////////////////////////////////////////////////////////////////////////
FLXInt32 tcp_server_readData(FLXSocket sock, FLXChar *buffer, FLXUnint32 bufferSize);

///////////////////////////////////////////////////////////////////////////
//   函数名称：tcp_server_stop
//   功能：停止Tcp监听，停止接受客户端连接
//   参数：
//       sock:FLXInt32,服务器端套接字
//   返回值：
//       如果成功，则返回0,否则返回-1
//////////////////////////////////////////////////////////////////////////
FLXInt32 tcp_server_stop(FLXSocket sock);
FLXInt32 tcp_server_slow_stop(FLXSocket sock);


#ifdef __cplusplus 
}
#endif

#endif //end of ifndef TCP_SERVER_HEAHER_FILE
