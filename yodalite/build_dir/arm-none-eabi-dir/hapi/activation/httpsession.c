#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "yodalite_autoconf.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// #include "freertos/event_groups.h"
#include "lib/cjson/cJSON.h"

#include "mbedtls/platform.h"

#include "mbedtls/net_sockets.h"

#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "login.h"
#include "common.h"
#include "http_parser.h"

static const char *TAG = "login";

#define WEB_SERVER "device-account.rokid.com"
#define WEB_PORT "443"
#define WEB_URL "https://www.howsmyssl.com/a/check"

// extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
// extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

static char request[1024];

static http_parser *parser;
static char *resp_body = NULL;

int on_message_begin(http_parser* _) {
  (void)_;
  printf("\n***MESSAGE BEGIN***\n\n");
  return 0;
}

int on_headers_complete(http_parser* _) {
  (void)_;
  printf("\n***HEADERS COMPLETE***\n\n");
  return 0;
}

int on_message_complete(http_parser* _) {
  (void)_;
  printf("\n***MESSAGE COMPLETE***\n\n");
  return 0;
}

int on_url(http_parser* _, const char* at, size_t length) {
  (void)_;
  printf("Url: %.*s\n", (int)length, at);
  return 0;
}

int on_header_field(http_parser* _, const char* at, size_t length) {
  (void)_;
  printf("Header field: %.*s\n", (int)length, at);
  return 0;
}

int on_header_value(http_parser* _, const char* at, size_t length) {
  (void)_;
  printf("Header value: %.*s\n", (int)length, at);
  return 0;
}

int on_body(http_parser* _, const char* at, size_t length) {
  (void)_;
  printf("Body: length:%d %s\n", (int)length, at);
  if(length >= 0) {
        resp_body = (char *)yodalite_malloc((int)length + 1);
        memset(resp_body, 0, (int)length + 1);
        memcpy(resp_body, at, length);
  }
  return 0;
}

static http_parser_settings settings_null =
  {.on_message_begin = on_message_begin
  ,.on_header_field = on_header_field
  ,.on_header_value = on_header_value
  ,.on_url = on_url
  ,.on_status = 0
  ,.on_body = on_body
  ,.on_headers_complete = on_headers_complete
  ,.on_message_complete = on_message_complete
};

static int https_resp_parse(const char *buf)
{
    size_t parsed;
    parser = malloc(sizeof(http_parser)); //分配一个http_parser
    http_parser_init(parser, HTTP_RESPONSE);  //初始化parser为Response类型
    parsed = http_parser_execute(parser, &settings_null, buf, strlen(buf));  //执行解析过程
    
    free(parser);
    parser = NULL;
    printf("https response parse end\n");
}

char *https_post(const char* header, const char* https_contents)
{ 
    char buf[2048];
    int ret, flags, len;

    resp_body = NULL;
    memset(request, 0, sizeof(request));
    sprintf(request, "%sContent-Length: %u\r\n\r\n", header, strlen(https_contents));
    strcat(request, https_contents);
    printf("lenth:%d  request:\n\r%s\n\r",strlen(request), request);

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

    mbedtls_ssl_init(&ssl);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    printf("Seeding the random number generator");

    mbedtls_ssl_config_init(&conf);

    mbedtls_entropy_init(&entropy);
    if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                    NULL, 0)) != 0)
    {
        printf("mbedtls_ctr_drbg_seed returned %d", ret);
        abort();
    }

    printf("Loading the CA root certificate...");

    // ret = mbedtls_x509_crt_parse(&cacert, server_root_cert_pem_start,
    //                              server_root_cert_pem_end-server_root_cert_pem_start);

    // if(ret < 0)
    // {
    //     printf("mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
    //     abort();
    // }

    printf("Setting hostname for TLS session...");

     /* Hostname set here should match CN in server certificate */
    if((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
    {
        printf("mbedtls_ssl_set_hostname returned -0x%x", -ret);
        abort();
    }

    printf("Setting up the SSL/TLS structure...");

    if((ret = mbedtls_ssl_config_defaults(&conf,
                                          MBEDTLS_SSL_IS_CLIENT,
                                          MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        printf("mbedtls_ssl_config_defaults returned %d", ret);
        goto exit;
    }

    /* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
       a warning if CA verification fails but it will continue to connect.

       You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
    */
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
#ifdef CONFIG_MBEDTLS_DEBUG
    mbedtls_esp_enable_debug_log(&conf, 4);
#endif

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        printf("mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        goto exit;
    }

        mbedtls_net_init(&server_fd);

        printf("Connecting to %s:%s...", WEB_SERVER, WEB_PORT);

        if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
                                      WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
        {
            printf("mbedtls_net_connect returned -%x", -ret);
            goto exit;
        }

        printf("Connected.");

        mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

        printf("Performing the SSL/TLS handshake...");

        while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                printf("mbedtls_ssl_handshake returned -0x%x", -ret);
                goto exit;
            }
        }

        printf("Verifying peer X.509 certificate...");

        if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
        {
            /* In real life, we probably want to close connection if ret != 0 */
            printf("Failed to verify peer certificate!");
            bzero(buf, sizeof(buf));
            mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
            printf("verification info: %s", buf);
        }
        else {
            printf("Certificate verified.");
        }

        printf("Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));

        printf("Writing HTTP request...");

        printf("\r\n request:%u\r\n%s\r\n",strlen(request),request);

        size_t written_bytes = 0;
        do {
            ret = mbedtls_ssl_write(&ssl,
                                    (const unsigned char *)request + written_bytes,
                                    strlen(request) - written_bytes);
            if (ret >= 0) {
                printf("%d bytes written", ret);
                written_bytes += ret;
            } else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ) {
                printf("mbedtls_ssl_write returned -0x%x", -ret);
                goto exit;
            }
        } while(written_bytes < strlen(request));

        printf("Reading HTTP response...");
    
        bzero(buf, sizeof(buf));
        int rep_len = 0;
        do
        {
            
            ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf + rep_len , sizeof(buf) - rep_len);
            //printf("Reading HTTP responseaa... 0x%x\n",ret);
            if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
                continue;

            if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
                ret = 0;
                break;
            }

            if(ret < 0)
            {
                printf("mbedtls_ssl_read returned -0x%x", -ret);
                break;
            }

            if(ret == 0)
            {
                //printf("connection closed");
                break;
            }
            rep_len += ret;
        } while(1);

        printf("=====================\n");
        printf("buf:%s\n",buf);
        printf("=====================\n");

        https_resp_parse(buf);

        printf("response body:%s\n",resp_body);

        mbedtls_ssl_close_notify(&ssl);

    exit:
        mbedtls_ssl_session_reset(&ssl);
        mbedtls_net_free(&server_fd);

        if(ret != 0)
        {
            //mbedtls_strerror(ret, buf, 100);
            printf("Last error was: -0x%x - %s", -ret, buf);
        }
        printf("https post end\n");
        return resp_body;
}
