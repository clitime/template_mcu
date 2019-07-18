#ifndef __LWIPOPTS__H__
#define __LWIPOPTS__H__

#if defined (__GNUC__)
	#define LWIP_TIMEVAL_PRIVATE 0
#else
	#define LWIP_TIMEVAL_PRIVATE 1
#endif

/**
 * The number of sys timeouts used by the core stack (not apps)
 * The default number of timeouts is calculated here for all enabled modules.
 */
#define IP_REASSEMBLY           1
/**
 * IP_FRAG==1: Fragment outgoing IP packets if their size exceeds MTU. Note
 * that this option does not affect incoming packet sizes, which can be
 * controlled via IP_REASSEMBLY.
 */
#define IP_FRAG                 1
/**
 * ARP_QUEUEING==1: Multiple outgoing packets are queued during hardware address
 * resolution. By default, only the most recent packet is queued per IP address.
 * This is sufficient for most protocols and mainly reduces TCP connection
 * startup time. Set this to 1 if you know your application sends more than one
 * packet in a row to an IP address that is not in the ARP cache.
 */
#define ARP_QUEUEING            0

#define LWIP_NETIF_LINK_CALLBACK		1

#define LWIP_COMPAT_SOCKETS         1
#define LWIP_POSIX_SOCKETS_IO_NAMES 1
/**
 * LWIP_SO_RCVTIMEO==1: Enable receive timeout for sockets/netconns and
 * SO_RCVTIMEO processing.
 */
#define LWIP_SO_RCVTIMEO            1
#define LWIP_TCP_KEEPALIVE          1 	//need for keepalive RTSP session
/**
 * LWIP_SO_SNDRCVTIMEO_NONSTANDARD==1: SO_RCVTIMEO/SO_SNDTIMEO take an int
 * (milliseconds, much like winsock does) instead of a struct timeval (default).
 */
#define LWIP_SO_SNDRCVTIMEO_NONSTANDARD 1

//NO_SYS==1: Provides VERY minimal functionality. Otherwise, use lwIP facilities.

#define NO_SYS                  0
/**
 * SYS_LIGHTWEIGHT_PROT==1: enable inter-task protection (and task-vs-interrupt
 * protection) for certain critical regions during buffer allocation, deallocation
 * and memory allocation and deallocation.
 * ATTENTION: This is required when using lwIP from more than one context! If
 * you disable this, you must be sure what you are doing!
 */
#define SYS_LIGHTWEIGHT_PROT    1

#define LWIP_SOCKET_SET_ERRNO 1
// ---------- Memory options ----------
/**
 * MEM_ALIGNMENT: should be set to the alignment of the CPU
 *    4 byte alignment -> \#define MEM_ALIGNMENT 4
 *    2 byte alignment -> \#define MEM_ALIGNMENT 2
 */
#define MEM_ALIGNMENT           4
/**
 * MEM_SIZE: the size of the heap memory. If the application will send
 * a lot of data that needs to be copied, this should be set high.
 */
#define MEM_SIZE                (12*1024)
/**
 * MEMP_OVERFLOW_CHECK: memp overflow protection reserves a configurable
 * amount of bytes before and after each memp element in every pool and fills
 * it with a prominent default value.
 *    MEMP_OVERFLOW_CHECK == 0 no checking
 *    MEMP_OVERFLOW_CHECK == 1 checks each element when it is freed
 *    MEMP_OVERFLOW_CHECK >= 2 checks each element in every pool every time
 *      memp_malloc() or memp_free() is called (useful but slow!)
 */
#define MEMP_OVERFLOW_CHECK             1
/**
 * MEMP_SANITY_CHECK==1: run a sanity check after each memp_free() to make
 * sure that there are no cycles in the linked lists.
 */
#define MEMP_SANITY_CHECK               1
/**
 * MEMP_NUM_SYS_TIMEOUT: the number of simultaneously active timeouts.
 * The default number of timeouts is calculated here for all enabled modules.
 * The formula expects settings to be either '0' or '1'.
 */
#define MEMP_NUM_SYS_TIMEOUT            5
/**
 * MEMP_NUM_NETBUF: the number of struct netbufs.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETBUF                 8
/**
 * MEMP_NUM_NETCONN: the number of struct netconns.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETCONN                16
/**
 * PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
 */
#define PBUF_POOL_SIZE                  32
/*
	 ------------------------------------------------
	 ---------- Internal Memory Pool Sizes ----------
	 ------------------------------------------------
*/
/**
 * MEMP_NUM_PBUF: the number of memp struct pbufs (used for PBUF_ROM and PBUF_REF).
 * If the application sends a lot of data out of ROM (or other static memory),
 * this should be set high.
 */
#define MEMP_NUM_PBUF           20
/**
 * MEMP_NUM_RAW_PCB: Number of raw connection PCBs
 * (requires the LWIP_RAW option)
 */
#define MEMP_NUM_RAW_PCB                4
/**
 * MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
 * per active UDP "connection".
 * (requires the LWIP_UDP option)
 */
#define MEMP_NUM_UDP_PCB                4
/**
 * MEMP_NUM_TCP_PCB: the number of simultaneously active TCP connections.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_PCB                5
/**
 * MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP connections.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_PCB_LISTEN         8
/**
 * MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP segments.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_SEG                16
/**
 * MEMP_NUM_TCPIP_MSG_INPKT: the number of struct tcpip_msg, which are used
 * for incoming packets.
 * (only needed if you use tcpip.c)
 */
#define MEMP_NUM_TCPIP_MSG_INPKT        8
/**
 * MEMP_NUM_FRAG_PBUF: the number of IP fragments simultaneously sent
 * (fragments, not whole packets!).
 * This is only used with LWIP_NETIF_TX_SINGLE_PBUF==0 and only has to be > 1
 * with DMA-enabled MACs where the packet is not yet sent when netif->output
 * returns.
 */
#define MEMP_NUM_FRAG_PBUF              15
/*
	 ------------------------------------------------
	 ---------- Internal Memory Pool Sizes ----------
	 ------------------------------------------------
*/
/**
 * TCP_WND: The size of a TCP window.  This must be at least
 * (2 * TCP_MSS) for things to work well.
 * ATTENTION: when using TCP_RCV_SCALE, TCP_WND is the total size
 * with scaling applied. Maximum window value in the TCP header
 * will be TCP_WND >> TCP_RCV_SCALE
 */
#define TCP_WND                         (4 * TCP_MSS)
/**
 * TCP_MSS: TCP Maximum segment size. (default is 536, a conservative default,
 * you might want to increase this.)
 * For the receive side, this MSS is advertised to the remote side
 * when opening a connection. For the transmit size, this MSS sets
 * an upper limit on the MSS advertised by the remote host.
 */
#define TCP_MSS                         1400
/**
 * TCP_SND_BUF: TCP sender buffer space (bytes).
 * To achieve good performance, this should be at least 2 * TCP_MSS.
 */
#define TCP_SND_BUF                     (2 * TCP_MSS)
/**
 * TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
 * as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work.
 */
#define TCP_SND_QUEUELEN                ((4 * (TCP_SND_BUF) + (TCP_MSS - 1))/(TCP_MSS))
/**
 * TCP_LISTEN_BACKLOG: Enable the backlog option for tcp listen pcb.
 */
#define TCP_LISTEN_BACKLOG      1


/**
 * PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. The default is
 * designed to accommodate single full size TCP frame in one pbuf, including
 * TCP_MSS, IP header, and link header.
 */
#define PBUF_POOL_BUFSIZE               LWIP_MEM_ALIGN_SIZE(TCP_MSS+40+PBUF_LINK_ENCAPSULATION_HLEN+PBUF_LINK_HLEN)

#define LWIP_RAW								1
// ---------- TCP options -----------
#define LWIP_TCP                1
#define TCP_TTL                 255

// ---------- ICMP options ----------
#define LWIP_ICMP               1

// ---------- DHCP options ----------
#define LWIP_DHCP               0

// ---------- UDP options ----------
#define LWIP_UDP                1
#define UDP_TTL                 255

#define LWIP_IGMP				0


// ---------- Checksum options ----------
//The STM32F4x7 allows computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
#define CHECKSUM_BY_HARDWARE

#ifdef CHECKSUM_BY_HARDWARE
	//CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.
	#define CHECKSUM_GEN_IP                 0
	//CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.
	#define CHECKSUM_GEN_UDP                0
	//CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.
	#define CHECKSUM_GEN_TCP                0
	//CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.
	#define CHECKSUM_CHECK_IP               0
	//CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.
	#define CHECKSUM_CHECK_UDP              0
	//CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.
	#define CHECKSUM_CHECK_TCP              0
	//CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.
	#define CHECKSUM_GEN_ICMP               0
#else
	//CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.
	#define CHECKSUM_GEN_IP                 1
	//CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.
	#define CHECKSUM_GEN_UDP                1
	//CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.
	#define CHECKSUM_GEN_TCP                1
	//CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.
	#define CHECKSUM_CHECK_IP               1
	//CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.
	#define CHECKSUM_CHECK_UDP              1
	//CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.
	#define CHECKSUM_CHECK_TCP              1
	//CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.
	#define CHECKSUM_GEN_ICMP               1
#endif

/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN                    1
/** LWIP_TCPIP_TIMEOUT==1: Enable tcpip_timeout/tcpip_untimeout to create
 * timers running in tcpip_thread from another thread.
 */
#define LWIP_TCPIP_TIMEOUT              1
/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET                     1

// ---------- DEBUG options ----------
#define LWIP_DEBUG                      0

#define TCP_RST_DEBUG           LWIP_DBG_OFF
#define MEM_DEBUG 				LWIP_DBG_OFF
#define PBUF_DEBUG              LWIP_DBG_OFF
#define API_LIB_DEBUG           LWIP_DBG_OFF
#define SOCKETS_DEBUG           LWIP_DBG_OFF
#define UDP_DEBUG               LWIP_DBG_OFF
#define IP_DEBUG	LWIP_DBG_OFF
#define ICMP_DEBUG	LWIP_DBG_ON
// ---------- Statistics options ----------
#define LWIP_STATS 							1

#if LWIP_STATS
	#define MEM_STATS 							1
	#define MEMP_STATS 							1

	#define LINK_STATS              1
	#define IP_STATS                1
	#define UDP_STATS               1

	#define SYS_STATS								1
	#define IP_STATS								1
#endif

// ---------- OS options ----------
#define TCPIP_THREAD_STACKSIZE          1024
#define TCPIP_MBOX_SIZE                 20
#define DEFAULT_UDP_RECVMBOX_SIZE       50
#define DEFAULT_TCP_RECVMBOX_SIZE       20
#define DEFAULT_ACCEPTMBOX_SIZE         10
#define DEFAULT_THREAD_STACKSIZE        1024
#define TCPIP_THREAD_PRIO               (configMAX_PRIORITIES - 3)


#endif //__LWIPOPTS__H_H
