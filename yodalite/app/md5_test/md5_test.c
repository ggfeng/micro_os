/*
 * =====================================================================================
 *
 *       Filename:  md5_test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/30/2019 05:16:42 PM
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
#include <lib/libmd5/md5.h>
#include <lib/shell/shell.h>

static int md5_test(int argc, int8_t *const argv[])
{
	uint8_t in[128] = {0};
	uint8_t out[16] = {0};
	uint8_t i = 0;

	strcpy(in, argv[1]);
	md5(in, strlen(in), out);
	for (i = 0; i < 16; i++) {
		printf("%02X", out[i]);
	}
	printf("\n");
	return 0;
}

#define max_md5_args      (2)
#define md5_test_help     "md5_test input"

int cmd_md5_test(void)
{
  YODALITE_REG_CMD(md5_test, max_md5_args, md5_test, md5_test_help);

  return 0;
}

