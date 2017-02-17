/*
 * wol.c
 *
 *  Created on: 2013-2-1
 *      Author: flx
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif

#include <netdb.h>
#include <netinet/ether.h>

int wol_passwd_sz = 0;
u_char wol_passwd[6];
int debug = 1;

int get_wol_pw(const char *optarg)
{
		int passwd[6];
		int byte_cnt;
		int i;
		byte_cnt = sscanf(optarg, "%2x:%2x:%2x:%2x:%2x:%2x",
										  &passwd[0], &passwd[1], &passwd[2],
										  &passwd[3], &passwd[4], &passwd[5]);
		if (byte_cnt < 4)
				byte_cnt = sscanf(optarg, "%d.%d.%d.%d",
												  &passwd[0], &passwd[1], &passwd[2], &passwd[3]);
		if (byte_cnt < 4) {
				fprintf(stderr, "Unable to read the Wake-On-LAN password.\n");
				return 0;
		}
		printf(" The Magic packet password is %2.2x %2.2x %2.2x %2.2x (%d).\n",
				   passwd[0], passwd[1], passwd[2], passwd[3], byte_cnt);
		for (i = 0; i < byte_cnt; i++)
				wol_passwd[i] = passwd[i];
		return wol_passwd_sz = byte_cnt;
}

#define MAXLINE 80
#define SERV_PORT 8000

static int mac_addr_str_to_eaddr(const char *hostid, struct ether_addr *eaddr)
{
	struct ether_addr *eap;
	eap = ether_aton(hostid);
	if (eap)
	{
		*eaddr = *eap;
		//if (debug) fprintf(stderr, "The target station address is %s.\n", ether_ntoa(eaddr));
	}
	else if (ether_hostton(hostid, eaddr) == 0)
	{
		// if (debug) fprintf(stderr, "Station address for hostname %s is %s.\n", hostid, ether_ntoa(eaddr));
	}
	else
	{
		/*
		(void) fprintf(stderr,
				"ether-wake: The Magic Packet host address must be "
						"specified as\n"
						"  - a station address, 00:11:22:33:44:55, or\n"
						"  - a hostname with a known 'ethers' entry.\n");
		*/
		return 0;
	}
	return 1;
}


/*
 * 构造一个以太网包
 *
 * 参数
 *     packet      ：缓存
 *     eaddr       ：目标MAC地址
 *     b_broadcast ：是否是广播包
 */
static int fill_ethernet_packet(unsigned char *packet, struct ether_addr *eaddr, int b_broadcast)
{
	int offset, i;
	unsigned char *mac_addr = eaddr->ether_addr_octet;
	if(!packet || !eaddr)
		return 0;

	/* 目标地址 */
	if (b_broadcast)
		memset(packet, 0xFF, 6);
	else
		memcpy(packet, mac_addr, 6);

	/* 源地址，暂时用目的地址填充 */
	memcpy(packet + 6, mac_addr, 6);

	packet[12] = 0x08; /* Or 0x0806 for ARP, 0x8035 for RARP */
	packet[13] = 0x42;

	/* 构造magic packet */
	offset = 14;
	memset(packet + offset, 0xFF, 6);
	offset += 6;
	for (i = 0; i < 16; i++)
	{
		memcpy(packet + offset, mac_addr, 6);
		offset += 6;
	}
	return offset;
}

static void print_ethernet_packet(u_char *eth_packet, int len)
{
	if(eth_packet && len > 0)
	{
		int k = 0;
		while(k < len)
		{
			if(k <= 12)
			{
				if((k % 6) == 0) printf("\n");
			}
			else if(k > 12 && k <= 14)
			{
				if(14 == k) printf("\n");
			}
			else
			{
				if(((k -2) % 6) == 0) printf("\n");
			}
			printf(" %2.2X", eth_packet[k]);
			k++;
		}
	}
}


//static int opt_no_src_addr = 0;
static int opt_broadcast = 0;
/*
 * 参数：
 *     des_mac_addr : 目标PC的MAC地址
 *     src_ifname   ：发送端设备的接口名，一般默认为eth0
 *
 *
 * format
 *     00:11:22:33:44:55
 */
static int internal_wake_on_lan(const char *des_mac_addr, const char *src_ifname)
{
	int k;
	int pktsize;
	int one = 1;
	struct ether_addr eaddr;
	int s; /* Raw socket */
	struct sockaddr_ll whereto;
	struct ifreq ifr;
	u_char outpack[1000];
	struct ifreq if_hwaddr;

	if(!des_mac_addr || !src_ifname)
		return 0;

	opt_broadcast = 1;
	if (!mac_addr_str_to_eaddr(des_mac_addr, &eaddr))
		return 0;

	pktsize = fill_ethernet_packet(outpack, &eaddr, opt_broadcast);
	if (pktsize <= 0)
		return 0;

	s = socket(PF_PACKET, SOCK_RAW, 0);
	if (s < 0)
	{
		fprintf(stderr, "socket open error: %s\n", strerror(errno));
		return 0;
	}

	/* 获取本机MAC地址*/
	// unsigned char *hwaddr = (unsigned char *) if_hwaddr.ifr_hwaddr.sa_data; //jia
	strcpy(if_hwaddr.ifr_name, src_ifname);
	if (ioctl(s, SIOCGIFHWADDR, &if_hwaddr) < 0)
	{
		fprintf(stderr, "SIOCGIFHWADDR on %s failed: %s\n", src_ifname, strerror(errno));
		/* Magic packets still work if our source address is bogus, but
		 we fail just to be anal. */
		return 0;
	}

	/* 填充以太网包的源MAC地址*/
	memcpy(outpack + 6, if_hwaddr.ifr_hwaddr.sa_data, 6);

	print_ethernet_packet(outpack, pktsize);
	printf("\n");

	/* This is necessary for broadcasts to work */
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char *) &one, sizeof(one)) < 0)
	{
		perror("setsockopt: SO_BROADCAST");
	}

	strncpy(ifr.ifr_name, src_ifname, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCGIFINDEX, &ifr) == -1)
	{
		fprintf(stderr, "SIOCGIFINDEX on %s failed: %s\n", src_ifname, strerror(errno));
		close(s);
		return 0;
	}

	memset(&whereto, 0, sizeof(whereto));
	whereto.sll_family = AF_PACKET;
	whereto.sll_ifindex = ifr.ifr_ifindex;
	/* The manual page incorrectly claims the address must be filled.
	 We do so because the code may change to match the docs. */
	whereto.sll_halen = ETH_ALEN;
	memcpy(whereto.sll_addr, outpack, ETH_ALEN);

	if ((k = sendto(s, outpack, pktsize, 0, (struct sockaddr *) &whereto, sizeof(whereto))) < 0)
		perror("sendto");
	else if (debug)
		printf("Sendto worked ! %d.\n", k);

	close(s);
	return 1;
}

/*
 * 参数：
 *     des_mac_addr : 目标PC的MAC地址
 *     src_ifname   ：发送端设备的接口名，一般默认为eth0
 *
 *
 * format
 *     00:11:22:33:44:55
 */
int wake_on_lan(const char *des_mac_addr)
{
	int result = internal_wake_on_lan(des_mac_addr, "eth0");
	if(!result)
		result = internal_wake_on_lan(des_mac_addr, "eth1");

	return result;
}
