#ifndef __KEYGEN_H_
#define __KEYGEN_H_
typedef struct rsa_key
{
	int len;
	unsigned int n0_inv;
	unsigned int modulus[64];
	unsigned int rr[64];
	int exponent;
} rsa_key_t;
#endif

