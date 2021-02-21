#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

#include <arpa/inet.h>
#include <lib/lwip/ip.h>
#include <lib/lwip/icmp.h>
#include <lib/lwip/netif.h>
#include <lib/lwip/sys.h>
#include <lib/lwip/timers.h>
#include <lib/lwip/inet_chksum.h>
#include <lib/lwip/sockets.h>
#include <lib/lwip/inet.h>
#include <lib/lwip/api.h>
#include <lib/lwip/err.h>

#include <lib/lwip/mqtt.h>

#ifndef P_FD_PENDING
#define P_FD_PENDING  1
#endif

#ifndef P_FD_ERR
#define P_FD_ERR  -1
#endif


static void dump_data(unsigned char *data,int size)
{
   int i;
   for(i=0;i<size;i++){
     printf("%x ",data[i]);

     if((i+1)%16 == 0)
       printf("\n");
   }
   printf("\n");

}

#define MQTT_USED_MBEDTLS

#ifndef MQTT_USED_MBEDTLS

int open_nb_socket(const char* addr, const char* port) {
    struct addrinfo hints = {0};

    hints.ai_family = AF_UNSPEC; /* IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Must be TCP */
    int sockfd = -1;
    int rv;
    struct addrinfo *p, *servinfo;

    /* get address information */
    rv = getaddrinfo(addr, port, &hints, &servinfo);
    if(rv != 0) {
        printf("error:failed to open socket (getaddrinfo)\n");
        return -1;
    }

    /* open the first possible socket */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;

        /* connect to server */
        rv = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
        if(rv == -1) continue;
        break;
    }  

    /* free servinfo */
    freeaddrinfo(servinfo);

    /* make non-blocking */
    if (sockfd != -1) 
       fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);

    /* return the new socket fd */
    printf("sockfd:%d\n",sockfd);

    return sockfd;  
}

ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void* buf, size_t len, int flags) {
     size_t cnts = 0;
/*
     printf("send: %d bytes\n",len);
     dump_data((unsigned char*)buf,(int)len);
*/

    while(cnts < len) {
        ssize_t tmp = send(fd, buf + cnts, len - cnts, flags);
        if (tmp < 1){
            return MQTT_ERROR_SOCKET_ERROR;
        }
        cnts += (size_t) tmp;
    }
    return cnts;
}

ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void* buf, size_t bufsz, int flags) {
    const void const *start = buf;
    ssize_t rv;
    do {
        rv = recv(fd, buf, bufsz, flags);
        if (rv > 0) {
/*
            printf("recv:%d bytes\n",rv);
            dump_data((unsigned char *)buf,(int)rv);
*/
            buf += rv;
            bufsz -= rv;

        }
    } while (rv > 0);

  return buf - start;
}


void close_nb_socket(int fd) 
{
  close(fd);
}

#else

#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/net_sockets.h>

#include <yodalite_autoconf.h>
#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif


struct mqtt_ssl_ctx {
    mbedtls_net_context      net;
    mbedtls_ssl_context      ssl;
    mbedtls_ssl_config       cfg;
    mbedtls_ctr_drbg_context drbg;
    mbedtls_entropy_context  etpy;
    mbedtls_x509_crt         x509;
    int last_read_ok;
};

struct mqtt_ssl_ctx* g_ctx = NULL;


struct mqtt_ssl_ctx * fd2ctx(int fd)
{
   struct mqtt_ssl_ctx *ctx = NULL;

   printf("%s->%d:fd:%d,g_ctx->net.fd:%d\n",__func__,__LINE__,fd,g_ctx->net.fd);
   if(fd == g_ctx->net.fd)
     ctx = g_ctx; 

   return ctx;
}

int mqtt_ssl_init(struct mqtt_ssl_ctx **ctx, int sock)
{
    struct mqtt_ssl_ctx *c = yodalite_malloc(sizeof(struct mqtt_ssl_ctx));

    if (!c) {
        printf("error:yodalite_malloc %d bytes failed\n",sizeof(struct mqtt_ssl_ctx));
        return -1;
    }

    mbedtls_net_init(&c->net);
    mbedtls_ssl_init(&c->ssl);
    mbedtls_ssl_config_init(&c->cfg);
    mbedtls_ctr_drbg_init(&c->drbg);
    mbedtls_x509_crt_init(&c->x509);

    mbedtls_entropy_init(&c->etpy);
    mbedtls_ctr_drbg_seed(&c->drbg, mbedtls_entropy_func, &c->etpy, NULL, 0);

    mbedtls_ssl_config_defaults(&c->cfg, MBEDTLS_SSL_IS_CLIENT,
        MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);

    mbedtls_ssl_conf_authmode(&c->cfg, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&c->cfg, &c->x509, NULL);
    mbedtls_ssl_conf_rng(&c->cfg, mbedtls_ctr_drbg_random, &c->drbg);

    mbedtls_ssl_set_bio(&c->ssl, &c->net, mbedtls_net_send,
        mbedtls_net_recv, mbedtls_net_recv_timeout);

    mbedtls_ssl_setup(&c->ssl, &c->cfg);


    c->net.fd = sock;
    printf("%s->%d:net.fd:%d,%d\n",__func__,__LINE__,sock,c->net.fd);
    *ctx = c;
    return 0;
}

int open_nb_socket(const char* addr, const char* port) 
{
    int iret  = -1;
    struct mqtt_ssl_ctx *ctx;
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC; /* IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Must be TCP */
    int sockfd = -1;
    int rv;
    struct addrinfo *p, *servinfo;

    /* get address information */
    rv = getaddrinfo(addr, port, &hints, &servinfo);
    if(rv != 0) {
        printf("error:failed to open socket (getaddrinfo)\n");
        return -1;
    }

    /* open the first possible socket */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;

        /* connect to server */
        rv = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
        if(rv == -1) continue;
        break;
    }  

    /* free servinfo */
    freeaddrinfo(servinfo);

    /* make non-blocking */
    if (sockfd != -1) 
       fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);

    printf("%s->%d:sockfd:%d\n",__func__,__LINE__,sockfd);

   if(mqtt_ssl_init(&ctx,sockfd) == 0){
       g_ctx = ctx;
     // iret  = sockfd;
   }

   printf("Performing the SSL/TLS handshake...\n");

   while((iret = mbedtls_ssl_handshake(&ctx->ssl)) != 0)
   {
     if (iret != MBEDTLS_ERR_SSL_WANT_READ && iret != MBEDTLS_ERR_SSL_WANT_WRITE)
     {
          iret  = -iret;
          printf("mbedtls_ssl_handshake returned -0x%x\n", -iret);
          break;
      }
   }
 
    printf("mbedtls_ssl_handshake() = %d\n",iret);

    if(iret == 0)
      iret = sockfd;
    /* return the new socket fd */
    printf("%s->%d:sockfd:%d\n",__func__,__LINE__,iret);

    return iret;  
}

void close_nb_socket(int fd)
{
  struct mqtt_ssl_ctx * ctx = fd2ctx(fd);

  if(ctx == NULL){
    printf("error:ctx is not exsist\n");
    return; 
  }

   mbedtls_net_free( &ctx->net );
   mbedtls_x509_crt_free(&ctx->x509);
   mbedtls_ssl_free(&ctx->ssl);
   mbedtls_ssl_config_free(&ctx->cfg);
   mbedtls_ctr_drbg_free(&ctx->drbg);
   mbedtls_entropy_free(&ctx->etpy);
   yodalite_free(ctx);
}

ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void* buf, size_t len, int flags) 
{
  int ret = 0;
  size_t bytes = 0;
  struct mqtt_ssl_ctx * ctx = fd2ctx(fd);

  if(ctx == NULL){
    printf("error:ctx is not exsist\n");
    return -1;
  }

  do {
     printf("mbedtls_ssl_write: %d bytes\n",len);
     dump_data((unsigned char*)buf,(int)len);

    ret = mbedtls_ssl_write(&ctx->ssl,(const unsigned char *)buf + bytes,len-ret);
    if (ret >= 0) {
      printf("%d bytes written\n", ret);
      bytes += ret;
    }
    else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ) {
        printf("mbedtls_ssl_write returned -0x%x", -ret);
        break;
     }
     printf("mbedtls_ssl_write ret:%d,bytes::%d\n",ret,bytes);
   }while(bytes < len);

   return bytes;
}

ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void* buf, size_t bufsz, int flags) 
{
   int ret = 0;
   size_t bytes = 0;
   const void const *start = buf;
   struct mqtt_ssl_ctx * ctx = fd2ctx(fd);

   if(ctx == NULL){
     printf("error:ctx is not exsist\n");
     return -1;
   }

   do{
       printf("mbedtls_ssl_read\n");

     if (mbedtls_ssl_get_bytes_avail(&ctx->ssl) <= 0) {
       break;
      }
      ret = mbedtls_ssl_read(&ctx->ssl,(unsigned char *)buf,bufsz);

       printf("mbedtls_ssl_read: %d bytes\n",ret);
       dump_data((unsigned char*)buf,(int)ret);

      if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE){
         continue;
         printf("wait read or write:0x%x\n",ret);
      }

      if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
                ret = 0;
                break;
      }

      if (ret > 0) {
         buf += ret;
         bufsz -= ret;
     }
  } while(ret > 0);
    printf("mbedtls_ssl_read exit\n"); 
   return buf -start;
}
#endif

