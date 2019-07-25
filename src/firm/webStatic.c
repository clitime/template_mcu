#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include <lwip/sockets.h>
#include "lwip/stats.h"

#include "webStatic.h"


//-----------------------------------------------------------------------------
#define TCP_THREAD_PRIO  				( tskIDLE_PRIORITY + 1 )

#define BUF_RCV		2048
static uint32_t nPageHits = 0;

static char recv_buffer[BUF_RCV];	//buffer for http_stat (dynamic)

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nConnection: close\r\n\r\n\
                                    <!DOCTYPE html>\n\
                                    <html>\n<head>\n<title>B-419-stat</title>\n\
                                    <meta http-equiv='Content-Type' content='text/html; charset=windows-1251'>\n\
                                    </head>\n<body bgcolor='#8dbdef' text='black'>\n";

const static char http_html_ftr[]="\n</body>\n</html>";

void DynWebPage(int conn);

#define LBFSZ  	1400
static char PAGE_BODY[LBFSZ];

#define FREERTOS_STATS_BUFLEN 	LBFSZ
static char freertos_stats[FREERTOS_STATS_BUFLEN];

#if LWIP_STATS
static char * memp_names[] =
	{
	#define LWIP_MEMPOOL(name,num,size,desc) desc,
	#include "lwip/priv/memp_std.h"
	};
#endif

static void http_server_stat(int conn)
{
	int ret;
	size_t buflen = BUF_RCV;
	int opt = 0, retval;

	opt = 1;
	retval = setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, (char *)&opt, sizeof(opt));
	if(retval == -1) {
		// SYS_DEBUGF(SYS_STATISTIC_DBG, ("Error setsockopt!\r\n"));
	}

	//Read in the request
	ret = lwip_read(conn, recv_buffer, buflen);
	if(ret <= 0) {
		lwip_close(conn);
		return;
	}

	if (strncmp((char *)recv_buffer,"GET /",5) == 0) {
		ret = lwip_send(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);
		// if(ret == -1) SYS_DEBUGF(SYS_STATISTIC_DBG, ("Error send socket dyn header!\r\n"));

		DynWebPage(conn);
		ret = lwip_send(conn, http_html_ftr, sizeof(http_html_ftr)-1, NETCONN_NOCOPY);
		// if(ret == -1) SYS_DEBUGF(SYS_STATISTIC_DBG, ("Error send socket dyn footer!\r\n"));
	}

	lwip_close(conn);
}
//-------------------------------------------------------------------------------------------------

static void http_server_stat_thread(void *arg) {
	(void)arg;
	int sock;
	struct sockaddr_in local;
	struct sockaddr_in from;
	int fromlen;
	int newconn;
	int opt = 0;

	sock = lwip_socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		// SYS_DEBUGF(SYS_STATISTIC_DBG, ("HttpStat SocketRecv: create filed!\r\n"));
		return;
	}

	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons(8080);
	local.sin_addr.s_addr = INADDR_ANY;

	if (lwip_bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
		lwip_close(sock);
		// SYS_DEBUGF(SYS_STATISTIC_DBG, ("HttpStat SocketRecv: bind filed!\r\n"));
		return;
	}

	//listen for incoming connections (TCP listen backlog = 1 for ONE peer !!!!!! Not more!!!!!!)
	lwip_listen(sock, 1); //1 peer for speed
	fromlen = sizeof(from);

	opt = 1;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&opt, sizeof(opt));

	for (;;) {
		newconn = lwip_accept(sock, (struct sockaddr *)&from, (socklen_t *)&fromlen);
		if (newconn > 0) http_server_stat(newconn);
		else vTaskDelay(10);
	}
}
//-------------------------------------------------------------------------------------------------
void http_stat_init(void) {
	xTaskCreate (http_server_stat_thread, "HttpSt", configMINIMAL_STACK_SIZE * 3, NULL, TCP_THREAD_PRIO, NULL);
}
//-------------------------------------------------------------------------------------------------
void DynWebPage(int conn) {
	portCHAR pagehits[100] = {0};
	uint32_t size=0, i, z;
	int ret;

  memset(PAGE_BODY, 0,LBFSZ);
	size=xPortGetFreeHeapSize();

  nPageHits++;
	sprintf(pagehits, "RtosHeap=%lu     MinimumFreeHeap=%u<br>", size, xPortGetMinimumEverFreeHeapSize());
  strcat(PAGE_BODY, pagehits);

	strcat((char *)PAGE_BODY, "<table width='100%' border='1' cellpadding='6' cellspacing='2'><th>RTOS threads</th>\n<th>RTOS time</th>\n<th>LwIP</th>\n<th>LwIP proto</th><tr>");

	strcat((char *)PAGE_BODY, "<td style='vertical-align: top'>");
	strcat((char *)PAGE_BODY, "<pre>");
	strcat((char *)PAGE_BODY, "<b>Name          State  Priority  Stack   Num</b>\n" );
  vTaskList((char *)(PAGE_BODY + strlen(PAGE_BODY)));
  strcat((char *)PAGE_BODY, "<br>B : Blocked, R : Ready, D : Deleted, S : Suspended<br>\n");
	strcat((char *)PAGE_BODY, "</pre></td>\n");
	ret = lwip_send(conn, PAGE_BODY, strlen(PAGE_BODY), NETCONN_COPY);
	if(ret == -1){goto end_dyn;}
	if(ret == 0){goto end_dyn;}

	memset(PAGE_BODY, 0,LBFSZ);
	strcat((char *)PAGE_BODY, "<td style='vertical-align: top'>");
	strcat((char *)PAGE_BODY, "<pre>");
	strcat((char *)PAGE_BODY, "<b>Name          Abs Time\t\tTime</b>\n" );
	vTaskGetRunTimeStats(freertos_stats);		//for this modify FreeRTOSConf.h and def timer functions
	strcat(PAGE_BODY, (const char *)freertos_stats);
	strcat((char *)PAGE_BODY, "</pre></td>\n");
	ret = lwip_send(conn, PAGE_BODY, strlen(PAGE_BODY), NETCONN_COPY);
	if(ret == -1){goto end_dyn;}
	if(ret == 0){goto end_dyn;}

	memset(PAGE_BODY, 0,LBFSZ);
	strcat((char *)PAGE_BODY, "<td style='vertical-align: top'>");
	strcat((char *)PAGE_BODY, "<pre>");
	for(i=0;i<100;i++){pagehits[i]=0x00;}
	sprintf(pagehits,"<b>%-15s %-5s %-5s %-5s %-5s %s</b><br>\n","Name","Cur","Size","Max","Err","Usage");
	strcat(PAGE_BODY, pagehits);
	for(i=0;i<100;i++){pagehits[i]=0x00;}

#if LWIP_STATS
	sprintf(pagehits,"%-15s %-5d %-5d %-5d %-5u %u%%\n","Heap",lwip_stats.mem.used,lwip_stats.mem.avail,lwip_stats.mem.max,lwip_stats.mem.err,lwip_stats.mem.used*100/lwip_stats.mem.avail);
	strcat(PAGE_BODY, pagehits);
	for (z = 0; z < 8; z++)
		{
		for(i=0;i<100;i++){pagehits[i]=0x00;}
		sprintf(pagehits,"%-15s %-5d %-5d %-5d %-5u %u%%\n",memp_names[z],lwip_stats.memp[z]->used,lwip_stats.memp[z]->avail,lwip_stats.memp[z]->max,lwip_stats.memp[z]->err,lwip_stats.memp[z]->used*100/lwip_stats.memp[z]->avail);
		strcat(PAGE_BODY, pagehits);
		}
	ret = lwip_send(conn, PAGE_BODY, strlen(PAGE_BODY), NETCONN_COPY);
	if(ret == -1){goto end_dyn;}
	if(ret == 0){goto end_dyn;}

	memset(PAGE_BODY, 0,LBFSZ);
	//for (z = 8; z < 15; z++)
	for (z = 8; z < 13; z++)		//becouse remove RAW_PCB in lwipopts.h
		{
		for(i=0;i<100;i++){pagehits[i]=0x00;}
		sprintf(pagehits,"%-15s %-5d %-5d %-5d %-5u %u%%\n",memp_names[z],lwip_stats.memp[z]->used,lwip_stats.memp[z]->avail,lwip_stats.memp[z]->max,lwip_stats.memp[z]->err,lwip_stats.memp[z]->used*100/lwip_stats.memp[z]->avail);
		strcat(PAGE_BODY, pagehits);
		}
#endif

	strcat(PAGE_BODY, "<br>\n");
	strcat((char *)PAGE_BODY, "</pre></td>\n");
	ret = lwip_send(conn, PAGE_BODY, strlen(PAGE_BODY), NETCONN_COPY);
	if(ret == -1){goto end_dyn;}
	if(ret == 0){goto end_dyn;}

	memset(PAGE_BODY, 0,LBFSZ);
	strcat((char *)PAGE_BODY, "<td style='vertical-align: top'>");
	strcat((char *)PAGE_BODY, "<pre>");
	for(i=0;i<100;i++){pagehits[i]=0x00;}
	sprintf(pagehits,"<b>%-15s %-5s %-5s %-5s %-5s %-5s</b><br>\n","Name","Recv","Trsmt","Drop","Mem","Err");
	strcat(PAGE_BODY, pagehits);
	for(i=0;i<100;i++){pagehits[i]=0x00;}

#if LWIP_STATS
	sprintf(pagehits, "%-15s %-5d %-5d %-5d %-5d %-5d\n", "Icmp", lwip_stats.icmp.recv, lwip_stats.icmp.xmit ,lwip_stats.icmp.drop, lwip_stats.icmp.memerr, lwip_stats.icmp.err);
	strcat(PAGE_BODY, pagehits);
	for(i=0;i<100;i++){pagehits[i]=0x00;}
	sprintf(pagehits, "%-15s %-5d %-5d %-5d %-5d %-5d\n", "Udp", lwip_stats.udp.recv, lwip_stats.udp.xmit ,lwip_stats.udp.drop, lwip_stats.udp.memerr, lwip_stats.udp.err);
	strcat(PAGE_BODY, pagehits);
	for(i=0;i<100;i++){pagehits[i]=0x00;}
	sprintf(pagehits, "%-15s %-5d %-5d %-5d %-5d %-5d\n", "Tcp", lwip_stats.tcp.recv, lwip_stats.tcp.xmit ,lwip_stats.tcp.drop, lwip_stats.tcp.memerr, lwip_stats.tcp.err);
	strcat(PAGE_BODY, pagehits);
	for(i=0;i<100;i++){pagehits[i]=0x00;}
	sprintf(pagehits, "%-15s %-5d %-5d %-5d %-5d %-5d\n", "Link", lwip_stats.link.recv, lwip_stats.link.xmit ,lwip_stats.link.drop, lwip_stats.link.memerr, lwip_stats.link.err);
	strcat(PAGE_BODY, pagehits);
	for(i=0;i<100;i++){pagehits[i]=0x00;}
	sprintf(pagehits, "%-15s %-5d %-5d %-5d %-5d %-5d\n", "Ip", lwip_stats.ip.recv, lwip_stats.ip.xmit ,lwip_stats.ip.drop, lwip_stats.ip.memerr, lwip_stats.ip.err);
	strcat(PAGE_BODY, pagehits);
	for(i=0;i<100;i++){pagehits[i]=0x00;}

	sprintf(pagehits,"<br><b>%-15s %-5s %-5s %-5s</b><br>\n","Name","Used","Max","Err");
	strcat(PAGE_BODY, pagehits);
	for(i=0;i<100;i++){pagehits[i]=0x00;}
	sprintf(pagehits, "%-15s %-5d %-5d %-5d\n", "Mbox", lwip_stats.sys.mbox.used, lwip_stats.sys.mbox.max, lwip_stats.sys.mbox.err);
	strcat(PAGE_BODY, pagehits);
	for(i=0;i<100;i++){pagehits[i]=0x00;}
	sprintf(pagehits, "%-15s %-5d %-5d %-5d\n", "Mutex", lwip_stats.sys.mutex.used, lwip_stats.sys.mutex.max, lwip_stats.sys.mutex.err);
	strcat(PAGE_BODY, pagehits);
	for(i=0;i<100;i++){pagehits[i]=0x00;}
	sprintf(pagehits, "%-15s %-5d %-5d %-5d\n", "Semph", lwip_stats.sys.sem.used , lwip_stats.sys.sem.max, lwip_stats.sys.sem.err);
	strcat(PAGE_BODY, pagehits);
	for(i=0;i<100;i++){pagehits[i]=0x00;}
#endif

	strcat((char *)PAGE_BODY, "</pre></td>\n</tr>\n</table>\n");
	ret = lwip_send(conn, PAGE_BODY, strlen(PAGE_BODY), NETCONN_COPY);
	if(ret == -1){goto end_dyn;}
	if(ret == 0){goto end_dyn;}

//	memset(PAGE_BODY, 0,LBFSZ);
//	strcat((char *)PAGE_BODY, "<pre>");
//	strcat((char *)PAGE_BODY, "<br>-------------------Local----------------------<br>\n");
//	for(i=0;i<100;i++){pagehits[i]=0x00;}
//	sprintf(pagehits, "RTP Pkt Rec=%-10d\n", Ses.RtpPcktRcv);
//	strcat(PAGE_BODY, pagehits);
//	sprintf(pagehits, "RTP Pkt Drop=%-10d\n", Ses.RtpPcktDrp);
//	strcat(PAGE_BODY, pagehits);
//	strcat(PAGE_BODY, "</pre>\n");
//	ret = lwip_send(conn, PAGE_BODY, strlen(PAGE_BODY), NETCONN_COPY);
end_dyn:
	return;
// 	if(ret == -1){SYS_DEBUGF(SYS_STATISTIC_DBG, ("Error sending dyn state page!\r\n"));}
// 	if(ret == 0){SYS_DEBUGF(SYS_STATISTIC_DBG, ("Error, close socket while sending dyn state page!\r\n"));}
}
