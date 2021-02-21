/*
 * =====================================================================================
 *
 *       Filename:  keygen.c
 *
 *    Description:  Generate the public key which contained len,n0_inv,modulus,rr,and
 *                  exponent
 *        Version:  1.0
 *        Created:  04/03/2019 09:21:59 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  glhou, 
 *   Organization:  Rokid
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include "keygen.h"

//#define KEYGEN_DEBUG
/*
const char* publicKey = "-----BEGIN PUBLIC KEY-----\n"\
"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDXAyGXbKEDLKaZg8sFKeZq9KIL\n"\
"uZchsXXdPYfEQKFTyuxAgR32obkJbkpskAMRyKXm6348DPyvnpNV1r+euXCJudFr\n"\
"nGtbcXuA02fpyYU3GTDWwmHJWJVJKkmQbyHcssV+wCpK+MXw/ht1OI/TsPKypOwa\n"\
"TKdptOXItURVGlvuTQIDAQAB\n"\
"-----END PUBLIC KEY-----\n";
*/
const char* publicKey = "-----BEGIN PUBLIC KEY-----\n"\
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsuzCHRNUXr/HAtDPgHlT\n"\
"GRfheANSRjrAh7lCdqsny2byQ3JnkQu2L5bmErXYPnbUdw7VYz1Fuia1udhjMP6x\n"\
"RMj0eCW0wF9p7N8IKV3r2uoXI4XB1tXDaPEyqER6pt+ddPTcQY2QbxnYwazxoanp\n"\
"W1ikxeWmKVcfuQdkE7B27bu1wXXR6F3XEcLIjYagMHooEgsbwcih9+B4EtyCcVq2\n"\
"ngjv91NpJ30b6Wr/YyXn271O868jRlkn5Qy5sybWwsvT0XySElwKfB2vPQyj3KUZ\n"\
"4tl57Yk3rCYMrOcFGJq2C1ZBgtFxMiyvH5puseRjpQiTa+ZwDOegSnL9nsa+WdKt\n"\
"FwIDAQAB\n"\
"-----END PUBLIC KEY-----\n";

/**
 * @brief: create Public key from public key pem file
*/
RSA* createPublicRSA(const char* key)
{
    RSA* rsa = NULL;
    BIO* keybio;
    keybio = BIO_new_mem_buf((void*)key, -1);
    if(keybio == NULL)
        return NULL;
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
    BIO_set_close(keybio, BIO_CLOSE);
    BIO_free(keybio);
    return rsa;
}

/*
 * rsa_get_exponent(): - Get the public exponent from an RSA key
 */
static int rsa_get_exponent(RSA *key, uint64_t *e)
{
	int ret;
	BIGNUM *bn_te;
	uint64_t te;

	ret = -EINVAL;
	bn_te = NULL;

	if (!e)
		goto cleanup;

	if (BN_num_bits(key->e) > 64)
		goto cleanup;

	*e = BN_get_word(key->e);

	if (BN_num_bits(key->e) < 33) {
		ret = 0;
		goto cleanup;
	}

	bn_te = BN_dup(key->e);
	if (!bn_te)
		goto cleanup;

	if (!BN_rshift(bn_te, bn_te, 32))
		goto cleanup;

	if (!BN_mask_bits(bn_te, 32))
		goto cleanup;

	te = BN_get_word(bn_te);
	te <<= 32;
	*e |= te;
	ret = 0;

cleanup:
	if (bn_te)
		BN_free(bn_te);

	return ret;
}

/*
 * rsa_get_params(): - Get the important parameters of an RSA public key
 */
int rsa_get_params(RSA *key, uint64_t *exponent, uint32_t *n0_invp,
		   BIGNUM **modulusp, BIGNUM **r_squaredp)
{
	BIGNUM *big1, *big2, *big32, *big2_32;
	BIGNUM *n, *r, *r_squared, *tmp;
	BN_CTX *bn_ctx = BN_CTX_new();
	int ret = 0;

	/* Initialize BIGNUMs */
	big1 = BN_new();
	big2 = BN_new();
	big32 = BN_new();
	r = BN_new();
	r_squared = BN_new();
	tmp = BN_new();
	big2_32 = BN_new();
	n = BN_new();
	if (!big1 || !big2 || !big32 || !r || !r_squared || !tmp || !big2_32 ||
	    !n) {
		printf("Out of memory (bignum)\n");
		return -ENOMEM;
	}

	if (0 != rsa_get_exponent(key, exponent))
		ret = -1;

	if (!BN_copy(n, key->n) || !BN_set_word(big1, 1L) ||
	    !BN_set_word(big2, 2L) || !BN_set_word(big32, 32L))
		ret = -1;

	/* big2_32 = 2^32 */
	if (!BN_exp(big2_32, big2, big32, bn_ctx))
		ret = -1;

	/* Calculate n0_inv = -1 / n[0] mod 2^32 */
	if (!BN_mod_inverse(tmp, n, big2_32, bn_ctx) ||
	    !BN_sub(tmp, big2_32, tmp))
		ret = -1;
	*n0_invp = BN_get_word(tmp);

	/* Calculate R = 2^(# of key bits) */
	if (!BN_set_word(tmp, BN_num_bits(n)) ||
	    !BN_exp(r, big2, tmp, bn_ctx))
		ret = -1;

	/* Calculate r_squared = R^2 mod n */
	if (!BN_copy(r_squared, r) ||
	    !BN_mul(tmp, r_squared, r, bn_ctx) ||
	    !BN_mod(r_squared, tmp, n, bn_ctx))
		ret = -1;

	*modulusp = n;
	*r_squaredp = r_squared;

	BN_free(big1);
	BN_free(big2);
	BN_free(big32);
	BN_free(r);
	BN_free(tmp);
	BN_free(big2_32);
	if (ret) {
		printf("Bignum operations failed\n");
		return -ENOMEM;
	}

	return ret;
}

void bn_hex_printf(BIGNUM * a)
{
	char *p = BN_bn2hex(a);
	printf("0x%s\n", p);
	OPENSSL_free(p);
}

void bn_printf(BIGNUM * a, int n)
{
	printf("0x");
	BN_print_fp(stdout, a);
	if (n)
		printf("\n");
}

int bignum_printf(BIGNUM *num, const char *name, int off)
{
	char num_buf[512];
	BIO *bp = NULL;
	BIO *bfile = NULL;
	int n = 0;
	int i = 0;
	int ret = 0;
	char *buf = num_buf;
	const char *neg;

	bp = BIO_new(BIO_s_mem());
	bfile = BIO_new_file("key_file", "wb");
    neg = (BN_is_negative(num)) ? "-" : "";
	buf[0] = 0;
	if (BIO_printf(bp, "%s%s", name,(neg[0] == '-') ? " (Negative)" : "") <= 0) {
		ret = -1;
		goto err;
	}

	n = BN_bn2bin(num, &buf[1]);
	if (buf[1] & 0x80)
		n++;
	else
		buf++;

	for (i = 0; i < n; i++) {
		if ((i % 15) == 0) {
			if (BIO_puts(bp, "\n") <= 0 || !BIO_indent(bp, off + 4, 128)) {
				ret = -1;
				goto err;
			}
        }
		BIO_write(bfile, &buf[i], 1);
        if (BIO_printf(bp, "%02x%s", buf[i], ((i + 1) == n) ? "" : ":")<= 0) {
            ret = -1;
			goto err;
		}
    }

    if (BIO_write(bp, "\n", 1) <= 0) {
		ret = -1;
		goto err;
	}

err:
	BIO_free(bfile);
	BIO_free(bp);
	return ret;
}

#define BIN_MOVE_BIT(a) ((3 - (a % 4)) * 8) 
/* Transfor bignum to 32bit array little endian*/
void bignum2array(BIGNUM *num, unsigned int *array)
{
	int bin_n = 0;
	int i = 0;
	int j = 0;
	unsigned char bin_buf[512] = {0};
	unsigned int  int_buf[64] = {0};
	unsigned char *buf = bin_buf;

	bin_n = BN_bn2bin(num, buf);
	for (i = 0; i < bin_n; i++) {
#ifdef KEYGEN_DEBUG
		printf("buf[%d] = %x\n", i, buf[i]);
#endif
		int_buf[j] |= buf[i] << BIN_MOVE_BIT(i);
		if (i % 4 == 3) {
			j++;
		}
	}
	for (i = 0; i < j; i++) {
		array[i] = int_buf[j - 1 - i];
#ifdef KEYGEN_DEBUG
		printf("array[%d] = 0x%x\n", i, array[i]);
#endif
	}
}

int rsa_get_pubkey_params(const char* pub_key, rsa_key_t *rsa_key)
{
	BIGNUM *modulus, *r_squared;
	uint64_t exponent;
	uint32_t n0_inv;
	RSA *pub_rsa;
	int ret = 0;
	int bits;

	pub_rsa = createPublicRSA(pub_key);
	ret = rsa_get_params(pub_rsa, &exponent, &n0_inv, &modulus, &r_squared);
	if (ret)
		return ret;
	bits = BN_num_bits(modulus);
	rsa_key->exponent = exponent;
	rsa_key->n0_inv = n0_inv;
	rsa_key->len = bits/32;
	/* Transfor bignum to 32bit array little endian*/
	bignum2array(modulus, rsa_key->modulus);
	bignum2array(r_squared, rsa_key->rr);

#ifdef KEYGEN_DEBUG
	printf("bits = 0x%x, exponent = 0x%llx, n0_inv = 0x%x\n", bits, exponent, n0_inv);
	printf("modulus = \n");
	{
		int i = 0;
		for (i = 0; i < rsa_key->len; i++) {
			printf("0x%08x ", rsa_key->modulus[i]);
			if (i % 8 == 7) {
				printf("\n");
			}
		}
			
	}
	//for (i = 0; i < bits/32; i++) {
	//	printf("%x, ", modulus[i]);
	//	if ((i+1) % 8 == 0) {
	//		printf("\n");
	//	}
	//}
	//BN_print_fp(stdout, modulus);
	//bn_hex_printf(modulus);
	bignum_printf(modulus, "modulus", 4);
#endif
	return ret;
}

int write_key_to_file(rsa_key_t *rsa_key, char* file)
{
	FILE *fp = NULL;
	int i = 0;
	fp = fopen(file, "w+");
	if (NULL == fp){
		printf("create file %s failed!\n", file);
		return -1;
	}
	fprintf(fp, "#ifndef __RSA_PUBKEY_H_\n");
	fprintf(fp, "#define __RSA_PUBKEY_H_\n");
	fprintf(fp, "#include <stdint.h> \n");
	fprintf(fp, "#include <hapi/rsa.h> \n");
	fprintf(fp, "static struct RSAPublicKey factory_rsa_pk = {\n");
	fprintf(fp, "    .len = %d,\n", rsa_key->len);
	fprintf(fp, "    .n0inv = 0x%x,\n", rsa_key->n0_inv);
	fprintf(fp, "    .exponent = 0x%x,\n", rsa_key->exponent);
	fprintf(fp, "    .n = {\n");
	for (i = 0; i < rsa_key->len; i++) {
		if (i % 8 == 0) {
			fprintf(fp, "            ");
		}
		fprintf(fp, "0x%08x, ", rsa_key->modulus[i]);
		if (i % 8 == 7) {
			fprintf(fp, "\n");
		}
	}
	fprintf(fp, "    },\n");
	fprintf(fp, "    .rr = {\n");
	for (i = 0; i < rsa_key->len; i++) {
		if (i % 8 == 0) {
			fprintf(fp, "            ");
		}
		fprintf(fp, "0x%08x, ", rsa_key->rr[i]);
		if (i % 8 == 7) {
			fprintf(fp, "\n");
		}
	}
	fprintf(fp, "    },\n");
	fprintf(fp, "};\n");
	fprintf(fp, "#endif");
	return 0;
}

//#define PUBKEY_OUT_FILE "rsa_pubkey.c"
int main(int argc, char **argv)
{
	rsa_key_t rsa_key;
	FILE *fp = NULL;
	char pub_key[512];

	fp = fopen(argv[1], "r");
	if (!fp) {
		printf("Open %s failed!\n", argv[1]);
		return -1;
	}
	fread(pub_key, 1, 512, fp);

	memset(&rsa_key, 0, sizeof(rsa_key_t));
	rsa_get_pubkey_params(pub_key, &rsa_key);
	write_key_to_file(&rsa_key, argv[2]);
	return 0;
}

