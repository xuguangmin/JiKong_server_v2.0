#ifndef FLX_NET_TYPES_HEADER_FILE
#define FLX_NET_TYPES_HEADER_FILE

////////////////////////////////////////////////////////////////////////////////////
//�ļ����ƣ�tcptypes.h
//�ļ�˵����
//         FLX����ͨ��������Ͷ����ļ�
//�汾��
//         v 1.0
//����ʱ��:
//2010��9��14��
//�����ˣ�
//         ����
//�޸��ˣ�
//         ��־��
//�޸�ʱ�䣺
//      2010��11��24��
//�޸����ݣ�
//		���socket�궨��
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
//���ͣ�FLXEndIpPoint
//���ã���¼�����ն���Ϣ,Ip:�˿ں�
//��Ա��
//ipAddress:char *,IP��ַ
//port:uint �˿ں�
////////////////////////////////////
typedef struct tag_flxIpEndPoint
{
	FLXChar  ipAddress[16];
	//char * ipAddress;
	FLXUnint32 port;
}FLXIpEndPoint,*PFLXIpEndPoint;

#endif //end of ifndef FLX_NET_TYPES_HEADER_FILE
