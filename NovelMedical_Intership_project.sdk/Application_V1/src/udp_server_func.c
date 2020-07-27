/*
 * Copyright (C) 2017 - 2019 Xilinx, Inc.
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

/** Connection handle for a UDP Server session */

#include "udp_server_func.h"

extern struct netif server_netif;
/* Report interval in ms */
#define RECV_SIZE 2000
#define SEND_SIZE 2000

char UDPsendBuffer[SEND_SIZE];  //����buffer�����ڱ��ļ����ɵײ�����ʹ��
char UDPrecvBuffer[RECV_SIZE];  //����buffer�����ڱ��ļ����ɵײ�����ʹ��
extern UDP_User_Recv_Buf;  // ����buffer�� ��main�ж���Ͷ�ȡ
extern UDP_User_Send_Buf;  // ����buffer�� ��main�ж���͸�ֵ
extern u8 tcp_udp_server_flag;	 //Server ����ȫ��״̬��Ǳ���

struct udp_pcb *connected_UDP_pcb = NULL; //udp��ȫ��pcb
static u8_t first = 1;
int count_recv = 0;// �����ڲ���


// ��ӡudp��ʼ��״̬
void UDP_print_app_header(void)
{
	xil_printf("UDP server listening on port %d\r\n",
			UDP_CONN_PORT);
	xil_printf("On Host: Run $iperf -c %s -i %d -t 300 -u -b <bandwidth>\r\n",
			inet_ntoa(server_netif.ip_addr),
			INTERIM_REPORT_INTERVAL);
}

// ��ӡĿǰ�Ľ���buffer
void UDP_print_received_data(void){
	xil_printf("%s \r\n", UDPrecvBuffer);
}



/** Receive data on a udp session */
static void udp_recv_callback(void *arg, struct udp_pcb *upcb,
		struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	if (first) {
		connected_UDP_pcb->remote_ip = *addr;
		connected_UDP_pcb->remote_port = port;
		xil_printf("connected with address %s and port %d\r\n", inet_ntoa(*addr), port);
		first = 0;
	}

	u32 data_len = 0;
	struct pbuf *q;
	if(p!=NULL)	//���յ���Ϊ�յ�����ʱ
	{
		// ** ���UDP�Ѿ������� **
		tcp_udp_server_flag|=1<<2;
		memset(UDPrecvBuffer,0,RECV_SIZE);  //���ݽ��ջ���������
		for(q=p;q!=NULL;q=q->next)  //����������pbuf����
		{
			//�ж�Ҫ������RECV_SIZE�е������Ƿ����RECV_SIZE��ʣ��ռ䣬�������
			//�Ļ���ֻ����RECV_SIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
			if(q->len > (RECV_SIZE-data_len)) memcpy(UDPrecvBuffer+data_len,q->payload,(RECV_SIZE-data_len));//��������
			else memcpy(UDPrecvBuffer+data_len,q->payload,q->len);
			data_len += q->len;
			if(data_len > RECV_SIZE) {
				xil_printf("surpass the limite\r\n");
				break; //����TCP�ͻ��˽�������,����
			}
		}
		// ** ���ս���,��ǽ��յ� **
		tcp_udp_server_flag|=1<<3;
		//*******************���ս���***********************
		// ����recvbuffer�ĳ�����ȷ���ģ�����һ�η����������ݱ���С��������ȣ����򽫿��ܵ������ݶ�ʧ
		//*******************����******************
		//strcpy(&UDP_User_Recv_Buf, &UDPrecvBuffer); // example
		//count_recv++;
		//*****************��������*****************
		pbuf_free(p);//�ͷ��ڴ�

	}else{  //�Ͽ�������
		udp_disconnect(upcb);
		xil_printf("UDP connection closed\r\n");
		tcp_udp_server_flag &= ~(1<<2);	//������ӶϿ�
	}
	return;
}

void UDP_Send_data(const char a[]){
	//** ���UDP�������ݣ���main��while�������� **
	tcp_udp_server_flag|=1<<4;
	struct pbuf *temp_p;
	struct udp_pcb *upcb = connected_UDP_pcb;

	memcpy(UDPsendBuffer, a, SEND_SIZE);// sizeof ��Ӧ�ô���ָ����ָ�ı���
	temp_p=pbuf_alloc(PBUF_TRANSPORT,strlen((char*)UDPsendBuffer),PBUF_POOL); //�����ڴ�
	if(temp_p)
		{
			pbuf_take(temp_p,(char*)UDPsendBuffer,strlen((char*)UDPsendBuffer)); //��UDPsendBuffer�е����ݴ����pbuf�ṹ��
			udp_send(upcb,temp_p);	//udp��������
			if(temp_p != NULL) pbuf_free(temp_p);//�ͷ��ڴ�
		}
}


void udp_server_connection_close(void)
{
	udp_disconnect(connected_UDP_pcb);
	udp_remove(connected_UDP_pcb);			//�Ͽ�UDP����
	xil_printf("TCP server closed \r\n");
	//** ������ӶϿ� **
	tcp_udp_server_flag &= ~(1<<2);
}

void UDP_start_application(void)
{
	err_t err;
	static struct udp_pcb *pcb;

	/* Create Server PCB */
	pcb = udp_new();
	if (!pcb) {
		xil_printf("UDP server: Error creating PCB. Out of Memory\r\n");
		return;
	}

	err = udp_bind(pcb, IP_ADDR_ANY, UDP_CONN_PORT);
	if (err != ERR_OK) {
		xil_printf("UDP server: Unable to bind to port");
		xil_printf(" %d: err = %d\r\n", UDP_CONN_PORT, err);
		udp_remove(pcb);
		return;
	}
	//** ������� **
	tcp_udp_server_flag|=1<<2;

	/* specify callback to use for incoming connections */
	udp_recv(pcb, udp_recv_callback, NULL);

	// set defaut remote address and port
	ip_addr_t ipaddr_remote_defaut;
	IP4_ADDR(&ipaddr_remote_defaut, 192, 168, 1, 2);
	static unsigned remote_port_defaut = 8000;
	pcb->remote_ip = ipaddr_remote_defaut;
	pcb->remote_port = remote_port_defaut;


	connected_UDP_pcb = pcb;

	return;
}