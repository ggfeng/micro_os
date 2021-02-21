/*
 * =====================================================================================
 *
 *       Filename:  auth.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/01/2019 01:28:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lib/libaes/aes.h>
#include <lib/libbase64/base64.h>
#include <hardware/pal_auth.h>
#include <lib/librsa/rsa_public_key.h> 
#include <lib/librsa/rsa.h>

int auth_rokid_aes(char *src, char *key, char *dst)
{
	uint8_t *cipher = NULL;
	uint8_t *iv = "Rokid";
	uint8_t aes_blocks;
	uint8_t len;
	FILE *fp = NULL;
	
    aes_blocks = DIV_ROUND_UP(strlen(src), AES_KEY_LENGTH);
	cipher = (uint8_t*)malloc(aes_blocks * AES_KEY_LENGTH);
	memset(cipher, 0, aes_blocks * AES_KEY_LENGTH);
	len = aes_blocks * AES_KEY_LENGTH;
	aes_do_encrypt(src, key, iv, cipher);
	base64_enc(cipher, len, dst);
	free(cipher);
	return 0;
}

static int get_sn(uint8_t *sn)
{
    int ret;
    ret = pal_auth_get_sn(sn);
    return ret;
}

static int get_chipid(uint8_t *chipid)
{
    int ret;
    ret = pal_auth_get_chipid(chipid);
    return ret;
}

static int get_sign(uint8_t *sign)
{
    int ret;
    ret = pal_auth_get_signature(sign);
    return ret;
}

#define SN_LEN_MAX 32
#define CHIPID_LEN_MAX 32
#define SIGN_LEN_MAX 512
int sign_verify(int8_t *sn, int8_t *chipid, int8_t *sign)
{
	int8_t *buf = NULL;
    uint8_t sign_buf[SIGN_LEN_MAX] = {0};
    int sign_len = 0;

	buf = strcat(sn, chipid);
#ifdef DEBUG_AUTH_RSA
	printf("sn+chipid = %s\n", buf);
	printf("sign = %s\n", sign);
#endif
	if (NULL == buf) {
		printf("Connect sn and chipid failed!\n");
		return -1;
	}
/*decode base64 string*/
    if (base64_dec(sign, sign_buf, &sign_len) < 0) {
        printf("base64 decode failed!\n");
        return -1;
    }
	if (rsa_with_sha1_verify(buf, strlen(buf), &factory_rsa_pk, sign_buf)) {
		printf("RSA verify failed!\n");
		return -1;
	}
	printf("RSA verify OK!\n");
	return 0;

}

int auth_sign_check(void)
{
	char sign[SIGN_LEN_MAX] = {0};
	char sn[SN_LEN_MAX + CHIPID_LEN_MAX] = {0};
	char chipid[CHIPID_LEN_MAX] = {0};
	char *buf = NULL;
#if 0
	if (get_sn(sn) < 0) {
		printf("Get serial number failed!\n");
		return -1;
	}
#ifdef AUTH_DEBUG
	printf("Get sn  = %s\n", sn);
#endif
	if (get_chipid(chipid) < 0) {
		printf("Get chipid failed!\n");
		return -1;
	}
#ifdef AUTH_DEBUG
	printf("Get chipid = %s\n", chipid);
#endif
	if (get_sign(sign) < 0) {
		printf("Get signature failed!\n");
		return -1;
	}
#ifdef AUTH_DEBUG
	printf("Get sign = %s\n",sign);
#endif
#endif
	if (sign_verify(sn, chipid, sign) < 0) {
		printf("RSA verify failed!\n");
		return -1;
	}
	printf("RSA verify success!\n");
	return 0;
}



