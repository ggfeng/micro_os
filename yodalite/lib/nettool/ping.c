#include <inttypes.h>
#include <stdlib.h>
#include <sys/time.h>


//#include <lib/lwip/opt.h>
//#include <lib/lwip/mem.h>
//#include <lib/lwip/raw.h>
# if 1 
#include <lwip/ip.h>
#include <lwip/icmp.h>
#include <lwip/netif.h>
#include <lwip/sys.h>
//#include <lwip/timers.h>
#include <lwip/inet_chksum.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <lwip/api.h>
#else
#include "ip.h"
#include "icmp.h"
#include "netif.h"
#include "sys.h"
#include "timers.h"
#include "inet_chksum.h"
#include "sockets.h"
#include "inet.h"
#include "api.h"
#endif

#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

static uint16_t  ping_seq_num=0;

static void  ping_prepare_echo( struct icmp_echo_hdr *iecho,uint16_t len)
{
  size_t i;
  size_t data_len = len - sizeof(struct icmp_echo_hdr);

  ICMPH_TYPE_SET(iecho, ICMP_ECHO);
  ICMPH_CODE_SET(iecho, 0);
  iecho->chksum = 0;
  iecho->id     = PING_ID;
  iecho->seqno  = htons(++ping_seq_num);

  /* fill the additional data buffer with some data */
  for(i = 0; i < data_len; i++) {
    ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
  }

  iecho->chksum = inet_chksum(iecho, len);
}

static int ping_send(int s, ip_addr_t *addr)
{
  int iret= 0;
  struct icmp_echo_hdr *iecho;
  struct sockaddr_in to;
  size_t ping_size = sizeof(struct icmp_echo_hdr) + 32;

  iecho = (struct icmp_echo_hdr *)malloc(ping_size);

  if (!iecho) {
    printf("error:malloc %d fail\n",ping_size);
    return -1;
  }

  ping_prepare_echo(iecho, (uint16_t)ping_size);

  to.sin_len = sizeof(to);
  to.sin_family = AF_INET;

  #ifdef __CONFIG_LWIP_V1
	    inet_addr_from_ipaddr(&to.sin_addr,addr);
  #else /*  now only for IPv4 */
	    inet_addr_from_ip4addr(&to.sin_addr, ip_2_ip4(addr));
  #endif

//  inet_addr_from_ipaddr(&to.sin_addr, ip_2_ip4(addr));

  if(sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to)) == -1)
  {
    iret = -1;
  }

  free (iecho);

  return iret;
}

static int ping_recv(int s)
{
  char buf[64]= {0};
  int len;
  struct sockaddr_in from;
  struct ip_hdr *iphdr;
  struct icmp_echo_hdr *iecho;
  int fromlen = sizeof(from);

  while((len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0) {
    if (len >= (int)(sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr))) {
//        ip4_addr_t fromaddr;
 //       inet_addr_to_ipaddr(&fromaddr, &from.sin_addr);
        ip_addr_t fromaddr;

  #ifdef __CONFIG_LWIP_V1
	    inet_addr_to_ipaddr(&fromaddr,&from.sin_addr);
  #else   /*  now only for IPv4 */
	    inet_addr_to_ip4addr(ip_2_ip4(&fromaddr),&from.sin_addr);
  #endif
        iphdr = (struct ip_hdr *)buf;
        iecho = (struct icmp_echo_hdr *)(buf + (IPH_HL(iphdr) * 4));

        if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) {
//          printf("ping: alive,seqno:%d\n",ping_seq_num);
          return 0;
        } else {
 //         printf("ping: drop\n");
        }
    }
  }
 
//  printf("warning:recvfrom timeout\n");
  return -1;
}

int ping(char*ip,int timeout_ms,int count)
{
  int s;
  int retry = count;
  err_t err;
  ip_addr_t ipaddr;

#if (CONFIG_OPT_RCVTIMEO_INT == 1)
  int timeout = timeout_ms;
#else
  struct timeval timeout;

  timeout.tv_sec = timeout_ms/1000;
  timeout.tv_usec = (timeout_ms%1000)*1000;
#endif

  err = netconn_gethostbyname(ip, &ipaddr);
  if (err != ERR_OK) {
 //   printf("netconn_gethostbyname(%s) failed, err=%d\n", ip, err);
    return -1;
  }
//  printf("Get host %s ipaddr %s\n", ip, inet_ntoa(ipaddr));

  if ((s = socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0) {
 //   printf("error:socket() fail\n");
    return -1;
  }

  if(setsockopt(s,SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))){
     printf("error:setting receive timeout failed\n");
     close(s);
     return -1;
  }

  while(retry)
  {
    uint32_t addr;
    int ret;

    if ((ret = ping_send(s, &ipaddr)) == 0) {
      if(ping_recv(s)== 0){
       close(s);
       return 0;
      }
   }
   else{
//    printf("error:ping_send fail %d\n",ret);
  } 
  
  retry --;
//  printf("ping %s fail %d times\n",ip,count-retry);

 }
 close(s);
 return  -1;
}

