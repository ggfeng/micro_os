/*
 * =====================================================================================
 *
 *       Filename:  auth_test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/22/2019 07:59:16 PM
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
#include <rsa.h>
#include <sign_verify.h>

int main(int argc, char **argv)
{
#if 0
	FILE *fp = NULL;
	char buf[1024];
	printf("start to test auth:");
	printf("sn+chipid = %s\n", argv[1]);
	printf("signature = %s\n", argv[2]);
	if(sign_verify(argv[1], argv[2])) {
		printf("Auth check failed!\n");
	} else {
		printf("Auth check success!\n");
	}
#endif
	auth_check();
	return 0;
}


