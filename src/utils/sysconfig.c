/*
 * sysconfig.c
 *
 *  Created on: 2012-11-5
 *      Author: flx
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>

#define MODIFY_IP_ADDR_SCRIPT           "/etc/init.d/ifconfig-eth.sh"

#if 0
int getif()
{
	int i = 0;
	int sockfd;
	struct ifconf ifconf;
	unsigned char buf[512];
	struct ifreq *ifreq;

	//初始化ifconf
	ifconf.ifc_len = 512;
	ifconf.ifc_buf = buf;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}
	ioctl(sockfd, SIOCGIFCONF, &ifconf); //获取所有接口信息
	//接下来一个一个的获取IP地址
	ifreq = (struct ifreq*) buf;
	for (i = (ifconf.ifc_len / sizeof(struct ifreq)); i > 0; i--)
	{
		// if(ifreq->ifr_flags == AF_INET){ //for ipv4
		printf("name = [%s]\n", ifreq->ifr_name);
		printf("local addr = [%s]\n",
				inet_ntoa(((struct sockaddr_in*) &(ifreq->ifr_addr))->sin_addr));
		ifreq++;
		// }
	}
	return 0;
}
#endif

unsigned int get_ipaddr(const char *str_eth)
{
	struct ifreq ifreq;
	int sock;
	unsigned int rtnval;

	if((sock=socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return 0;

	memset(&ifreq, 0, sizeof(ifreq));
	strcpy(ifreq.ifr_name, str_eth);
	((struct sockaddr_in*)(&(ifreq.ifr_addr)))->sin_family = AF_INET;

	if(ioctl(sock, SIOCGIFADDR, &ifreq) < 0)
	{
		close(sock);
		return 0;
	}

	close(sock);

	rtnval = ((unsigned int)(((struct sockaddr_in*)(&(ifreq.ifr_addr)))->sin_addr.s_addr));
	return ntohl(rtnval);
}

unsigned int get_netaddr(const char *str_eth)
{
	struct ifreq ifreq;
	int sock;
	unsigned int rtnval;

	if((sock=socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return 0;

	memset(&ifreq, 0, sizeof(ifreq));
	strcpy(ifreq.ifr_name, str_eth);
	((struct sockaddr_in*)(&(ifreq.ifr_addr)))->sin_family = AF_INET;

	if(ioctl(sock, SIOCGIFNETMASK, &ifreq) < 0)
	{
		close(sock);
		return 0;
	}
	close(sock);

	rtnval = ((unsigned int)(((struct sockaddr_in*)(&(ifreq.ifr_addr)))->sin_addr.s_addr));
	return ntohl(rtnval);
}

/*
 * ethname : eth0 eth1 ...
 */
int set_ipaddr_netmask(const char *ethname, const char* iphost, const char* netmask)
{
	FILE *pf_ifconfig;
	char sout[64] = { 0 };
	char scmd[128];
	//in_addr_t ipval[2];
	if(!ethname || !iphost)
		return 0;

	if(inet_addr(iphost) == -1)
		return 0;

	if(netmask)
	{
		if(inet_addr(netmask) == -1)
			return 0;

		sprintf(scmd, "ifconfig %s %s netmask %s", ethname, iphost, netmask);
	}
	else
		sprintf(scmd, "ifconfig %s %s", ethname, iphost);

	pf_ifconfig = popen(scmd, "r");
	if(pf_ifconfig == NULL)
		return 0;

	while(fgets(sout, 63, pf_ifconfig) != NULL)
		;

	pclose(pf_ifconfig);

	return 1;
}
int set_ipaddr_netmask2(int lan_no, const char* ipaddr, const char* netmask)
{
	char ethname[64];
	if(lan_no < 1 || lan_no > 2)
		return 0;

	sprintf(ethname, "eth%d", lan_no-1);
	return set_ipaddr_netmask(ethname, ipaddr, netmask);
}

int set_system_time(int year, int month, int day, int hour, int minute, int second)
{
	struct tm tms;
	struct timeval tv;
	struct timezone tz;

	//printf("%04d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, minute, second);

	tms.tm_year = year -1900;
	tms.tm_mon  = month - 1;
	tms.tm_mday = day;
	tms.tm_hour = hour-8;
	tms.tm_min  = minute;
	tms.tm_sec  = second;
	tv.tv_sec = mktime(&tms);
	tv.tv_usec = 0;

	tz.tz_minuteswest = 0;
	tz.tz_dsttime     = 0;

	return (0 == settimeofday(&tv, &tz)) ? 1:0;
}
/*
 * 修改一个脚本的内容
 *
 * 该脚本会在linux启动时自动加载，用来修改本机的IP
 * 参数：
 *      ip_addr  新的IP地址
 *
 * 返回值：0失败，否则成功
 *
 * example:
 *        ifconfig lo 127.0.0.1\nifconfig eth0 %s netmask %s
 */
int modify_ip_address_script(const char *ip_addr, const char *net_mask)
{
	int fd, len;
	char ipBuffer[256];
	fd = open(MODIFY_IP_ADDR_SCRIPT, O_WRONLY|O_CREAT|O_TRUNC);
	if(fd < 0)
	{
		printf("open %s error!\n", MODIFY_IP_ADDR_SCRIPT);
		return 0;
	}

	sprintf(ipBuffer, "ifconfig lo 127.0.0.1\nifconfig eth0 %s netmask %s", ip_addr, net_mask);
	len = strlen(ipBuffer);
	ipBuffer[len] = '\0';
	write(fd, ipBuffer, len);
	close(fd);
	return 1;
}

int modify_ifconfig_eth_script(const char *content)
{
	int fd, len;
	if(!content || strlen(content) <= 0)
		return 0;

	fd = open(MODIFY_IP_ADDR_SCRIPT, O_WRONLY|O_CREAT|O_TRUNC);
	if(fd < 0)
	{
		printf("open %s error!\n", MODIFY_IP_ADDR_SCRIPT);
		return 0;
	}

	len = strlen(content);
	write(fd, content, len);
	close(fd);
	return 1;
}

int str_to_ip_address(char *srcstr)
{
	char *p;
	int a1, a2, a3, a4, a, ix;
	if(!srcstr || strlen(srcstr)<= 0)
		return 0;

	ix = 1;
	p = strtok(srcstr, ".");
	if(!p) return 0;
	a1 = (atoi(p));
	if(a1 < 0 || a1 > 255)
		return 0;

	ix++;
	while((p = strtok(NULL, ".")))
	{
		a = atoi(p);
		if(a < 0 || a > 255)
			return 0;

		if     (2 == ix) a2 = a;
		else if(3 == ix) a3 = a;
		else break;
		ix++;
	}

	if(!p) return 0;
	a4 = atoi(p);
	if(a4 < 0 || a4 > 255)
		return 0;

	sprintf(srcstr, "%d.%d.%d.%d", a1, a2, a3, a4);
	return 1;
}

void reboot_device()
{
	printf("Call system function.\n");
	if(system("./reboot.sh") < 0)
	{
		printf("reboor sysem error.\n");
	}
}

