/*
 * =====================================================================================
 *
 *       Filename:  base64_test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/31/2019 10:14:37 PM
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
#include "lib/libbase64/base64.h"
#include <lib/shell/shell.h>

static int base64_test(int argc, int8_t *const argv[])
{
	uint8_t src[32] = {0};
	uint8_t dst[32] = {0};
	strcpy(src, argv[1]);
	base64_enc(src, strlen(src), dst);
	printf("base64 src %s, dst %s\n", src, dst);
	return 0;
}

#define max_base64_args      (4)
#define base64_test_help     "base64_test src"

int cmd_base64_test(void)
{
  YODALITE_REG_CMD(base64_test, max_base64_args, base64_test, base64_test_help);

  return 0;
}

