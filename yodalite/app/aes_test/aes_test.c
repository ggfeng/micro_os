/*
 * =====================================================================================
 *
 *       Filename:  aes_test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/28/2019 09:11:25 PM
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
#include <lib/libaes/aes.h>
#include <lib/shell/shell.h>

static int aes_test(int argc, int8_t *const argv[])
{
	uint8_t src[256] = {0};
	uint8_t key[256] = {0};
	uint8_t iv[256] = {0};
	uint8_t cipher[256] = {0};
	uint8_t aes_blocks;
	uint8_t i;
	FILE *fp = NULL;
	
	if (argc < 3) {
		printf("Too less args,'aes_test src key iv' or 'aes_test src key \n");
		return -1;
	}
	strcpy(src, argv[1]);
	strcpy(key, argv[2]);
	if (argc >3) {
		strcpy(iv, argv[3]);
	}
	
    aes_blocks = DIV_ROUND_UP(strlen(src), AES_KEY_LENGTH);
	aes_do_encrypt(src, key, iv, cipher);

	for (i = 0; i < aes_blocks * 16; i++){
		if ((i % 16) == 0) {
			printf("\n");
		}
		printf("%02x ", cipher[i]);
	}
	printf("\n");
	return 0;
}

#define max_aes_args      (4)
#define aes_test_help     "aes_test src key iv"

int cmd_aes_test(void)
{
  YODALITE_REG_CMD(aes_test, max_aes_args, aes_test, aes_test_help);

  return 0;
}

