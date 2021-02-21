/*
 * =====================================================================================
 *
 *       Filename:  auth_test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/01/2019 01:47:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <lib/libaes/aes.h>
#include <hapi/auth/auth.h>
#include <lib/shell/shell.h>

static int auth_aes_test(int argc, int8_t *const argv[])
{
	uint8_t *src = NULL;
	uint8_t *key = NULL;
	uint8_t dst[128] = {0};
	//uint8_t *dst = NULL;
	uint8_t aes_blocks;
	uint8_t len = 0;
	
	if (argc < 3) {
		printf("Too less args,'aes_test src key iv' or 'aes_test src key \n");
		return -1;
	}
//	strcpy(src, argv[1]);
//	strcpy(key, argv[2]);
	src = argv[1];
	key = argv[2];

//	len = strlen(src);
//	printf("src len = %d\n", len);
//    aes_blocks = DIV_ROUND_UP(len, AES_KEY_LENGTH);
//	printf("strlen src is %d aes_blocks = %d\n", strlen(src), aes_blocks);
//	dst = (uint8_t*)malloc(aes_blocks * AES_KEY_LENGTH);
//	memset(dst, 0, aes_blocks * AES_KEY_LENGTH);
	auth_rokid_aes(src, key, dst);
	
	printf("%s\n", dst);
//	free(dst);
	return 0;
}

static int auth_rsa_test(int argc, int8_t *const argv[])
{
	uint8_t sn[64] = {0};
	uint8_t chipid[64] = {0};
	uint8_t sign[360] = {0};
	
	if (argc < 4) {
		printf("Too less args,'rsa_test sn chipid sign' \n");
		return -1;
	}
	strcpy(sn, argv[1]);
	strcpy(chipid, argv[2]);
	strcpy(sign, argv[3]);
	
	sign_verify(sn, chipid, sign);
	return 0;
}


#define max_auth_aes_args      (3)
#define auth_aes_test_help     "auth_aes_test src key"

int cmd_auth_aes_test(void)
{
  YODALITE_REG_CMD(auth_aes_test, max_auth_aes_args, auth_aes_test, auth_aes_test_help);

  return 0;
}

#define max_auth_rsa_args      (4)
#define auth_rsa_test_help     "auth_rsa_test sn chipid sign"

int cmd_auth_rsa_test(void)
{
  YODALITE_REG_CMD(auth_rsa_test, max_auth_rsa_args, auth_rsa_test, auth_rsa_test_help);

  return 0;
}




