/*
 * Copyright (C) 2018 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include "xparameters.h"
#include "netif/xadapter.h"
#include "platform.h"
#include "platform_config.h"
#include "lwipopts.h"
#include "xil_printf.h"
#include "sleep.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#include "user_xadc.h"
#include "oled.h"

#if LWIP_IPV6==1
#include "lwip/ip6_addr.h"
#include "lwip/ip6.h"
#else

#if LWIP_DHCP==1
#include "lwip/dhcp.h"
extern volatile int dhcp_timoutcntr;
#endif
#define DEFAULT_IP_ADDRESS	"192.168.1.10"
#define DEFAULT_IP_MASK		"255.255.255.0"
#define DEFAULT_GW_ADDRESS	"192.168.1.1"
#endif /* LWIP_IPV6 */





//************* 共有 ***************
extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
extern volatile int Sampling_Flag;
extern volatile int Oled_Show_Flag;
u8 tcp_udp_server_flag;
//TCP Server 测试全局状态标记变量
//bit7:0,TCP没有数据要发送;1,有数据要发送
//bit6:0,TCP没有收到数据;1,收到数据了.
//bit5:0,TCP没有客户端连接上;1,有客户端连接上了.
//bit4:0,UDP没有数据要发送;1,有数据要发送
//bit3:0,UDP没有收到数据;1,收到数据了.
//bit2:0,UDP没有客户端连接上;1,有客户端连接上了.
//bit1~0:保留

void platform_enable_interrupts(void);
void TCP_print_app_header(void);

//************* TCP ***************
void TCP_print_app_header(void);
void TCP_start_application(void);
extern char TCPrecvBuffer;
extern char TCPsendBuffer;
char TCP_User_Recv_Buf[2000];
char TCP_User_Send_Buf[2000];
void TCP_print_received_data(void);
void TCP_send_data(const char a[]);


//************** UDP ****************
void UDP_print_app_header(void);
void UDP_start_application(void);
extern char UDPrecvBuffer;
extern char UDPsendBuffer;
char UDP_User_Recv_Buf[2000];
char UPD_User_Send_Buf[2000];
void UDP_Send_data(const char a[]);
void UDP_print_received_data(void);


//************** XADC ****************
static XAdcPs XADCInst;

//************* all ******************



#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || \
		 XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
int ProgramSi5324(void);
int ProgramSfpPhy(void);
#endif
#endif

#ifdef XPS_BOARD_ZCU102
#ifdef XPAR_XIICPS_0_DEVICE_ID
int IicPhyReset(void);
#endif
#endif

struct netif server_netif;

#if LWIP_IPV6==1
static void print_ipv6(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf(" %s\n\r", inet6_ntoa(*ip));
}
#else
static void print_ip(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\r\n", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

static void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	print_ip("Board IP:       ", ip);
	print_ip("Netmask :       ", mask);
	print_ip("Gateway :       ", gw);
}

static void assign_default_ip(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	int err;

	xil_printf("Configuring default IP %s \r\n", DEFAULT_IP_ADDRESS);

	err = inet_aton(DEFAULT_IP_ADDRESS, ip);
	if (!err)
		xil_printf("Invalid default IP address: %d\r\n", err);

	err = inet_aton(DEFAULT_IP_MASK, mask);
	if (!err)
		xil_printf("Invalid default IP MASK: %d\r\n", err);

	err = inet_aton(DEFAULT_GW_ADDRESS, gw);
	if (!err)
		xil_printf("Invalid default gateway address: %d\r\n", err);
}
#endif /* LWIP_IPV6 */

int main(void)
{
	struct netif *netif;

	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] = {
		0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	netif = &server_netif;
#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || \
		XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
	ProgramSi5324();
	ProgramSfpPhy();
#endif
#endif

	/* Define this board specific macro in order perform PHY reset
	 * on ZCU102
	 */
#ifdef XPS_BOARD_ZCU102
	IicPhyReset();
#endif

	init_platform(); //初始化平台，包括计时器和中断的初始化
	lwip_init(); //初始化lwip
	XADC_Init(&XADCInst); //初始化内置xadc
	OLED_Init();  //初始化oled显示屏

	/******  fin init  ****************/

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(netif, NULL, NULL, NULL, mac_ethernet_address,
				PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\r\n");
		return -1;
	}

#if LWIP_IPV6==1
	netif->ip6_autoconfig_enabled = 1;
	netif_create_ip6_linklocal_address(netif, 1);
	netif_ip6_addr_set_state(netif, 0, IP6_ADDR_VALID);
	print_ipv6("\n\rlink local IPv6 address is:", &netif->ip6_addr[0]);
#endif /* LWIP_IPV6 */

	netif_set_default(netif);

	/* now enable interrupts */
	platform_enable_interrupts();

	/* specify that the network if is up */
	netif_set_up(netif);

#if (LWIP_IPV6==0)
#if (LWIP_DHCP==1)
	/* Create a new DHCP client for this interface.
	 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
	 * the predefined regular intervals after starting the client.
	 */
	dhcp_start(netif);
	dhcp_timoutcntr = 24;
	while (((netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0))
		xemacif_input(netif);

	if (dhcp_timoutcntr <= 0) {
		if ((netif->ip_addr.addr) == 0) {
			xil_printf("ERROR: DHCP request timed out\r\n");
			assign_default_ip(&(netif->ip_addr),
					&(netif->netmask), &(netif->gw));
		}
	}

	/* print IP address, netmask and gateway */
#else
	assign_default_ip(&(netif->ip_addr), &(netif->netmask), &(netif->gw));
#endif
	print_ip_settings(&(netif->ip_addr), &(netif->netmask), &(netif->gw));
#endif /* LWIP_IPV6 */

	xil_printf("\r\n");

	/* print tcp and udp header */
	TCP_print_app_header();
	UDP_print_app_header();

	/* start the application*/
	TCP_start_application();
	UDP_start_application();
	xil_printf("\r\n");

	tcp_udp_server_flag = 0x00;
	int flag_first_tcp = 0;
	int flag_first_udp = 0;
	int send_sampling_flag = 0;

	//*** flag 需要滞留一下 ***
	u8 OLED_show_etat;

	char oled_head_string[12];
	char olde_stat_string[16];
	char Ethernet_data[16];
	memset(oled_head_string, 0, sizeof(oled_head_string));
	memset(olde_stat_string, 0, sizeof(olde_stat_string));
	memset(Ethernet_data, 0, sizeof(Ethernet_data));

	OLED_Clear();
	sprintf(oled_head_string, "TCP | UDP | CMD");
	OLED_ShowString(0,0, oled_head_string);
	OLED_Refresh_Gram();

	while (1) {
		if (TcpFastTmrFlag) {// every 250ms
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) { // every 500ms
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(netif);

		// user code
		OLED_show_etat = tcp_udp_server_flag; //在每次循环开始，进行刷新


		//**************** XADC *****************
		if (Sampling_Flag){ //every 250ms
			sprintf(Ethernet_data, "temp: %.4f", temper_sampling(&XADCInst));

			send_sampling_flag = 1;// 若等于0，则永远不会进行采样
			Sampling_Flag = 0;
		}


		//		//*********** TCP ****************
		if (tcp_udp_server_flag&1<<5) {  //连接成功则发送数据
			//usleep(1000);

			if (tcp_udp_server_flag&1<<6){//有新的数据收到
				//TCP_print_received_data();
				//UDP_Send_data(TCP_User_Recv_Buf);//UDP_User_Recv_Buf

				OLED_show_etat |= 1<<6; //在oled中标记tcp有新的数据收到
				tcp_udp_server_flag &= ~(1<<6); //TCP取消标记已收到新信息
			}
		}

		//**************** UDP ****************
		if(tcp_udp_server_flag&1<<3){//tcp有新的数据收到
			//void UDP_print_received_data(void);
			//TCP_send_data(UDP_User_Recv_Buf);//TCP_User_Recv_Buf

			OLED_show_etat |= 1<<3; //指示oled有新的tcp数据收到
			tcp_udp_server_flag &= ~(1<<3); //消除tcp收到新数据flag
		}


		// ******** 当tcp收到start命令进入start状态，使用udp发送数据****
		if((tcp_udp_server_flag&1<<1) && (send_sampling_flag)){
			UDP_Send_data(Ethernet_data);
			send_sampling_flag = 0;
		}

		//**************** DELAY *****************
		//usleep(20000);


				//*********** OLED ***************
		//应该在结尾之前更新oled并显示所有状态，之后清除flag
		if(Oled_Show_Flag){ //every 25ms
//			// 显示采样的温度数据
//			OLED_ShowString(0,0, oled_head_string);
//			OLED_Refresh_Gram();

			//补漏 7，4位
			OLED_show_etat |= (tcp_udp_server_flag & 0x91);

			sprintf(olde_stat_string, "%s%s%s | %s%s%s | %s%s",\
					(OLED_show_etat&(1<<7) ? "A":"_"),
					(OLED_show_etat&(1<<6) ? "V":"_"),
					(OLED_show_etat&(1<<5) ? "&":"X"),
					(OLED_show_etat&(1<<4) ? "A":"_"),
					(OLED_show_etat&(1<<3) ? "V":"_"),
					(OLED_show_etat&(1<<2) ? "&":"X"),
					(OLED_show_etat&(1<<1) ? "@":"!"),
					(OLED_show_etat&(1<<0) ? "*":"_")
					);
			//xil_printf("%s \r\n", olde_stat_string);
			OLED_Refresh_Gram();
			OLED_ShowString(0,16, olde_stat_string);
			OLED_Refresh_Gram();
			Oled_Show_Flag = 1; // 赋值0， 则可以每25ms刷新，否则可以最快速度刷新
		}

		//**************** end of the circle************
		tcp_udp_server_flag &= ~(1<<7); // 在循环结尾清除TCP发送标记 ，记得在上面补漏
		tcp_udp_server_flag &= ~(1<<4); // 在循环结尾清除UDP发送标记
		tcp_udp_server_flag &= ~(1<<0); // 清除进入cmd的标记

	}

	/* never reached */
	void udp_server_connection_close();
	cleanup_platform();

	return 0;
}
