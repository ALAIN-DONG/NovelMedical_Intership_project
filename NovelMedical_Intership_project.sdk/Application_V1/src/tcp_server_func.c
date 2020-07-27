#include "tcp_server_func.h"

extern struct netif server_netif;
static struct tcp_pcb *connected_TCP_pcb = NULL; // connected_pcb c_pcb


extern u8 tcp_udp_server_flag;	 //Server ����ȫ��״̬��Ǳ���

#define RECV_SIZE 2000
#define SEND_SIZE 2000
char TCPsendBuffer[SEND_SIZE];//����buffer�����ڱ��ļ����ɵײ�����ʹ��
char TCPrecvBuffer[RECV_SIZE];//����buffer�����ڱ��ļ����ɵײ�����ʹ��
extern TCP_User_Send_Buf;//����buffer����main�ж���͸�д
extern TCP_User_Recv_Buf;//����buffer����main�ж���Ͷ�ȡ

// �ڿ���������ʹ�õı���
int tcp_poll_cnt = 0;
int tcp_sent_cnt = 0;
int count_for = 0;
int Test_counter = 0;
char test_char[] = "test! \r\n";

//********************************
void TCP_receive_commend(struct pbuf *p);

// ��ӡtcp��ʼ��״̬
void TCP_print_app_header(void)
{
	xil_printf("TCP server listening on port %d\r\n",
			TCP_CONN_PORT);
#if LWIP_IPV6==1
	xil_printf("On Host: Run $iperf -V -c %s%%<interface> -i %d -t 300 -w 2M\r\n",
			inet6_ntoa(server_netif.ip6_addr[0]),
			INTERIM_REPORT_INTERVAL);
#else
	xil_printf("On Host: Run $iperf -c %s -i %d -t 300 -w 2M\r\n",
			inet_ntoa(server_netif.ip_addr),
			INTERIM_REPORT_INTERVAL);
#endif /* LWIP_IPV6 */
}


//*****************Ӧ�ò� ����*******************************

// ��ӡrecvbuffer�е�����
void TCP_print_received_data(void){
	xil_printf("%s \r\n", TCPrecvBuffer);
}

//*******************RAW TCP ����***************************
//������ͺ����������и��Բ�ͬ�����뷽ʽ�͵��÷�ʽ����ʵ����ʹ�����أ�������Ϊֱ�ۺͱ��գ�����������
//��������ͺ��������𽥸��ӣ�TCP_send_data����Ϊ���հ汾��ǰ�ĸ�Ϊ̽��ʱд�������հ汾��Ӧɾ��
// send_a ����������������ֱ��ʹ�ã������κβ���
//�ڲ�ʹ����ȫ�ֱ���connected_pcb��Ϊwrite�е�������������޷�ָ��������tcp_pcb
void send_data_a()
{
	err_t err;

	itoa(Test_counter, TCPsendBuffer, 10);
	//strcat(Test_counter, " \0\r\n");

	struct tcp_pcb *tpcb = connected_TCP_pcb;

	if (!connected_TCP_pcb)
			return;

	err = tcp_write(tpcb, TCPsendBuffer, SEND_SIZE, 3);
	if (err != ERR_OK) {
		xil_printf("txperf: Error on tcp_write: %d\r\n", err);
		connected_TCP_pcb = NULL;
		return;
	}
	err = tcp_output(tpcb);
	if (err != ERR_OK) {
		xil_printf("txperf: Error on tcp_output: %d\r\n",err);
		return;
	}
	Test_counter++;
}


//send_b ʹ��pbuf* Ϊ������������͸������������Ѿ��洢������
//�ڲ�ʹ����ȫ�ֱ���connected_pcb��Ϊwrite�е�������������޷�ָ��������tcp_pcb
void send_data_b(struct pbuf* p)
{
	err_t err;
	struct tcp_pcb *tpcb = connected_TCP_pcb;

	if (!connected_TCP_pcb)
			return;

	err = tcp_write(tpcb, p->payload, p->len, 3);
	if (err != ERR_OK) {
		xil_printf("txperf: Error on tcp_write: %d\r\n", err);
		connected_TCP_pcb = NULL;
		return;
	}
	err = tcp_output(tpcb);
	if (err != ERR_OK) {
		xil_printf("txperf: Error on tcp_output: %d\r\n",err);
		return;
	}
}



//send_c ʹ��tcp_pcb * �� pbuf* Ϊ������������� pbuf*���������Ѿ��洢������
//���Ը��ݲ�ͬ�Ķ˿�ָ����ͬ�������send���²⣩
void send_data_c(struct tcp_pcb *tpcb, struct pbuf* p){
	struct pbuf *ptr;
	u16 plen;
	err_t wr_err=ERR_OK;
	 while((wr_err==ERR_OK)&&p&&(p->len<=tcp_sndbuf(tpcb)))
	 {
		ptr=p;
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1);
		if(wr_err==ERR_OK)
		{
			plen=ptr->len;
			p=ptr->next;			//ָ����һ��pbuf
			if(p) pbuf_ref(p);	//pbuf��ref��һ
			pbuf_free(ptr);
			tcp_recved(tpcb,plen); 		//����tcp���ڴ�С
		}else if(wr_err==ERR_MEM) p=ptr;//����Ͳ�������
	 }
}


void send_data_c1(const char a[]){
	struct tcp_pcb *tpcb = connected_TCP_pcb;
	struct pbuf* temp_p;

	memcpy(TCPsendBuffer, a, sizeof(TCPrecvBuffer));
	temp_p = pbuf_alloc(PBUF_TRANSPORT,strlen((char*) TCPsendBuffer),PBUF_POOL);
	pbuf_take(temp_p,(char*)TCPsendBuffer,strlen((char*)TCPsendBuffer));
	send_data_c(tpcb, temp_p);
	if(temp_p != NULL) pbuf_free(temp_p);
}
//send_c �� send_d ��ʹ��ʾ��������recv��callback�У��ڽ�����ɺ�
//***********************��ʼ���ͣ���buffer�ж�ȡ***********************

//			struct pbuf* temp_p;
//			temp_p = pbuf_alloc(PBUF_TRANSPORT,strlen((char*) sendBuffer),PBUF_POOL);
//			pbuf_take(temp_p,(char*)sendBuffer,strlen((char*)sendBuffer));
//			send_data_c(tpcb, temp_p);
//			if(temp_p != NULL) pbuf_free(temp_p);

//��ѡ����
//			es->p = pbuf_alloc(PBUF_TRANSPORT,strlen((char*) sendBuffer),PBUF_POOL);
//			pbuf_take(es->p,(char*)sendBuffer,strlen((char*)sendBuffer));
//			send_data_d(tpcb,es);
//			//tcp_server_flag&=~(1<<7);  			//������ݷ��ͱ�־λ
//			if(es->p != NULL) pbuf_free(es->p); //�ͷ��ڴ�

//***********************�������ͣ���buffer�ж�ȡ���***********************

//send_d ʹ��tcp_pcb * �� tcp_server_struct * Ϊ�������������tcp_server_struct * �� pbuf*���������Ѿ��洢������
//��send_c ��ͬС�죬ֻ������pbuf* ���ɽ���tcp_server_struct *�У��������˸����ж�����ǿ��ȫ�ԣ���ȻҲ�����ʹ��
//���Ը��ݲ�ͬ�Ķ˿�ָ����ͬ�������send���²⣩
void send_data_d(struct tcp_pcb *tpcb, struct tcp_server_struct *es){
	struct pbuf *ptr;
		u16 plen;
		err_t wr_err=ERR_OK;
		 while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb)))
		 {
			ptr=es->p;
			wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1);
			if(wr_err==ERR_OK)
			{
				plen=ptr->len;
				es->p=ptr->next;			//ָ����һ��pbuf
				if(es->p) pbuf_ref(es->p);	//pbuf��ref��һ
				pbuf_free(ptr);
				tcp_recved(tpcb,plen); 		//����tcp���ڴ�С
			}else if(wr_err==ERR_MEM) es->p=ptr;
		 }
}


// ʹ��tcp���������ݣ������κ�λ��ʹ�ã�����Ϊ�ַ���
void TCP_send_data(const char a[]){
	//** ��Ƿ������� **
	tcp_udp_server_flag|=1<<7;
	struct tcp_pcb *tpcb = connected_TCP_pcb;
	u16 plen;
	err_t wr_err=ERR_OK;
	struct pbuf* temp_p;
	struct pbuf *ptr;
	memcpy(TCPsendBuffer, a, RECV_SIZE); //������������ַ���д��sendbuffer��
	temp_p = pbuf_alloc(PBUF_TRANSPORT,strlen((char*) TCPsendBuffer),PBUF_POOL);
	pbuf_take(temp_p,(char*)TCPsendBuffer,strlen((char*)TCPsendBuffer));

	 while((wr_err==ERR_OK)&&temp_p&&(temp_p->len<=tcp_sndbuf(tpcb)))
	 {
		ptr=temp_p;
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1);
		if(wr_err==ERR_OK)
		{
			plen=ptr->len;
			temp_p=ptr->next;			//ָ����һ��pbuf
			if(temp_p) pbuf_ref(temp_p);	//pbuf��ref��1
			pbuf_free(ptr);
			tcp_recved(tpcb,plen); 		//����tcp���ڴ�С
		}else if(wr_err==ERR_MEM) temp_p=ptr;//����Ͳ�������
	 }
	if(temp_p != NULL) pbuf_free(temp_p); //�ͷ��ڴ�
}

// ������tcp������ָ�������ָ��
void TCP_receive_commend(struct pbuf *p){
	xil_printf("\n**reading commend\r\n");
	tcp_udp_server_flag|=1<<0;

	u16_t pos1;
	pos1 = pbuf_memfind(p, "start", 5, 0);
	if (pos1 == 0xFFFF){
		xil_printf("start commend not found ! \r\n");
	}
	else{
		tcp_udp_server_flag|=1<<1;
		xil_printf("yes! start commend received \r\n");
	}
	u16_t pos2;
	pos2 = pbuf_memfind(p, "stop", 4, 0);
	if (pos2 == 0xFFFF){
		xil_printf("stop commend not found ! \r\n");
	}
	else{
		tcp_udp_server_flag &= ~(1<<1);
		xil_printf("yes! stop commend received \r\n");
	}
}

//���ջص�����
err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	err_t ret_err;
	u32 data_len = 0;
	struct pbuf *q;
	struct tcp_server_struct *tcp_state;
	LWIP_ASSERT("arg != NULL",arg != NULL);
	tcp_state=(struct tcp_server_struct *)arg;

	/* ����δ����״̬��Ҫ��ȡ���ݰ� */
	if (p==NULL) {
		tcp_state->state=TCPSERVER_CLOSING;//��Ҫ�ر�TCP ������
		tcp_state->p=p;
		tcp_close(tpcb);//�ر�����
		xil_printf("tcp connection closed\r\n");
		tcp_recv(tpcb, NULL); // ����ص�����������
		return ERR_OK;
	}

	if(err!=ERR_OK){//�ӿͻ��˽��յ�һ���ǿ�����,��������ĳ��ԭ��err!=ERR_OK
		if(p)pbuf_free(p);	//�ͷŽ���pbuf
		ret_err=err;
	}else if(tcp_state->state==TCPSERVER_ACCEPTED){//��������״̬
		if(p!=NULL)  //����������״̬���ҽ��յ������ݲ�Ϊ��ʱ�����ӡ����
		{
			//******************* ��ʼ���ղ�����buffer**********************
			TCP_receive_commend(p); //ֱ�Ӵ�pbuf�з�����ȡָ���ʵҲ�ɴ�TCP_User_Recv_Buf�з�����ȡָ��
			memset(TCPrecvBuffer,0,RECV_SIZE);  //���ݽ��ջ���������
			for(q=p;q!=NULL;q=q->next)  //����������pbuf����
			{
				//�ж�Ҫ������RECV_SIZE�е������Ƿ����RECV_SIZE��ʣ��ռ䣬�������
				//�Ļ���ֻ����RECV_SIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
				if(q->len > (RECV_SIZE-data_len)) memcpy(TCPrecvBuffer+data_len,q->payload,(RECV_SIZE-data_len));//��������
				else memcpy(TCPrecvBuffer+data_len,q->payload,q->len);
				data_len += q->len;
				if(data_len > RECV_SIZE){
					break; //����TCP�ͻ��˽�������,����
				}
			}
			//** ��ǽ��յ������� **
			tcp_udp_server_flag|=1<<6;
			tcp_recved(tpcb, p->len);   //���յ����ݰ������´���
			//***********************���ս������Ѵ���recvbuffer************************
			//**************���������봦�����յ�����******************
			strcpy(TCP_User_Recv_Buf, TCPrecvBuffer);
			pbuf_free(p);    //�ͷ�pbuf
			ret_err=ERR_OK;
		}
	}else//�������ر���
	{
		tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
		tcp_state->p=NULL;
		pbuf_free(p); //�ͷ��ڴ�
		ret_err=ERR_OK;
	}
	return ret_err;
}

//�ر�tcp����
void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	tcp_close(tpcb);
	tcp_arg(tpcb,NULL);
	tcp_sent(tpcb,NULL);
	tcp_recv(tpcb,NULL);
	tcp_err(tpcb,NULL);
	tcp_poll(tpcb,NULL,0);
	if(es)mem_free(es);
	tcp_udp_server_flag&=~(1<<5);//������ӶϿ���
	xil_printf("TCP server closed \r\n");
}

// tcp��ѯ����ѡ���Ƿ�������accetp��
err_t tcp_poll_callback(void * arg, struct tcp_pcb * tpcb)
{
	err_t ret_err;
	struct tcp_server_struct *tcp_state;
	tcp_state=(struct tcp_server_struct *)arg;

	tcp_poll_cnt++;     //ͳ�Ʒ������ݵĴ���
	xil_printf("poll int:%d\r\n", tcp_poll_cnt);
	if(tcp_state!=NULL)
	{
		if(tcp_udp_server_flag&(1<<7))	//�ж��Ƿ�������Ҫ����
		{
			//���´������ʵ�������պ�������ʹ�ã����跢�����flag
			xil_printf("polling test \r\n");
			memcpy(TCPsendBuffer, test_char , sizeof(test_char));
			tcp_state->p = pbuf_alloc(PBUF_TRANSPORT,strlen((char*) TCPsendBuffer),PBUF_POOL);
			pbuf_take(tcp_state->p,(char*)TCPsendBuffer,strlen((char*)TCPsendBuffer));
			send_data_d(tpcb,tcp_state);
			//tcp_server_flag&=~(1<<7);  			//������ݷ��ͱ�־λ
			if(tcp_state->p != NULL) pbuf_free(tcp_state->p); //�ͷ��ڴ�
		}else if(tcp_state->state==TCPSERVER_CLOSING)//��Ҫ�ر�����?ִ�йرղ���
		{
			tcp_server_connection_close(tpcb,tcp_state);//�ر�����
		}
		ret_err = ERR_OK;
	}else
	{
		tcp_abort(tpcb);//��ֹ����,ɾ��pcb���ƿ�
		ret_err=ERR_ABRT;
	}

	return ret_err;
}

// ����֮��ĵ��õĻص�����
static err_t tcp_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	struct tcp_server_struct *es;
	LWIP_UNUSED_ARG(len);
	es = (struct tcp_server_struct *) arg;
	if(es->p) send_data_d(tpcb,es);//�������ݣ�������������ܻ���û������ģ�

	tcp_sent_cnt++; //ͳ�Ʒ������ݵĴ���
	//xil_printf("sent counte : %d \r\n", tcp_sent_cnt);
	return ERR_OK;
}


//tcp_err�����Ļص�����
void tcp_server_error_callback(void *arg,err_t err)
{
	LWIP_UNUSED_ARG(err);
	printf("tcp error \r\n");
	if(arg!=NULL)mem_free(arg);//�ͷ��ڴ�
}

// ���ӻص�����
err_t tcp_server_accept_callback(void *arg,struct tcp_pcb *tpcb,err_t err){
	xil_printf("tcp_server: Connection Accepted\r\n");
	err_t ret_err;
	struct tcp_server_struct *tcp_state;
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	tcp_setprio(tpcb,TCP_PRIO_MIN);//�����´�����pcb���ȼ�
	tcp_state=(struct tcp_server_struct*)mem_malloc(sizeof(struct tcp_server_struct)); //�����ڴ�

	if(tcp_state!=NULL) //�ڴ����ɹ�
	{
		tcp_state->state=TCPSERVER_ACCEPTED;  	//��������
		tcp_state->pcb=tpcb;
		tcp_state->p=NULL;

		connected_TCP_pcb = tpcb;   //�洢���ӵ�TCP״̬
		tcp_nagle_disable(connected_TCP_pcb); // ͣ��nagle�� ������ջ���ŷ���//connected_pcb
		tcp_arg(connected_TCP_pcb,tcp_state);  //���ò���
		tcp_recv(connected_TCP_pcb, tcp_recv_callback);  //���ý��յĻص�����
		tcp_err(connected_TCP_pcb,tcp_server_error_callback); 	//��ʼ��err�Ļص�����
#if USE_TCP_POLL
		tcp_poll(connected_TCP_pcb, tcp_poll_callback, 2);//������ѯ�Ļص����������� 2 -->> 1 ��/��
#endif
		tcp_sent(connected_TCP_pcb, tcp_sent_callback); //���÷��ͺ�Ļص�����

		//** ����пͻ��������� **
		tcp_udp_server_flag|=1<<5;
		ret_err=ERR_OK;
	}else{
		ret_err=ERR_MEM;
	}
	return ret_err;
}


// ��ʼtcp��server���ã���main����
int TCP_start_application()
{
	struct tcp_pcb *TCP_server_pcb;
	err_t err;

	/*  �����µ�TCP PCB  */
	TCP_server_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!TCP_server_pcb) {
		xil_printf("txperf: Error creating PCB. Out of Memory\r\n");
		return -1;
	}
	/*  �󶨱��ض˿�  */
	err = tcp_bind(TCP_server_pcb, IP_ADDR_ANY, TCP_CONN_PORT);
	if (err != ERR_OK) {
	    xil_printf("tcp_server: Unable to bind to port %d: err = %d\r\n", TCP_CONN_PORT, err);
	    tcp_close(TCP_server_pcb);
	    return -2;
	}
    /*  ��������  */
	TCP_server_pcb = tcp_listen(TCP_server_pcb);
	if (!TCP_server_pcb) {
		xil_printf("tcp_server: Out of memory while tcp_listen\r\n");
		tcp_close(TCP_server_pcb);
		return -3;
	}

	tcp_arg(TCP_server_pcb, NULL);

	/*  ����accept�ص�����  */
	tcp_accept(TCP_server_pcb, tcp_server_accept_callback);

	return 0;
}

