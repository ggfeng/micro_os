#ifndef _AES_REF_H_
#define _AES_REF_H_
#include <stdint.h>

/*
 * AES encryption library, with small code size, supporting only 128-bit AES
 *
 * AES is a stream cipher which works a block at a time, with each block
 * in this case being AES_KEY_LENGTH bytes.
 */
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
enum {
	AES_STATECOLS	= 4,	/* columns in the state & expanded key */
	AES_KEYCOLS	= 4,	/* columns in a key */
	AES_ROUNDS	= 10,	/* rounds in encryption */

	AES_KEY_LENGTH	= 128 / 8,
	AES_EXPAND_KEY_LENGTH	= 4 * AES_STATECOLS * (AES_ROUNDS + 1),
};

/**
 * Apply chain data to the destination using EOR
 *
 * Each array is of length AES_KEY_LENGTH.
 *
 * @cbc_chain_data	Chain data
 * @src			Source data
 * @dst			Destination data, which is modified here
 */
void aes_apply_cbc_chain_data(uint8_t *cbc_chain_data, uint8_t *src, uint8_t *dst);

/**
 * aes_cbc_encrypt_blocks() - Encrypt multiple blocks of data with AES CBC.
 *
 * @key_exp		Expanded key to use
 * @src			Source data to encrypt
 * @dst			Destination buffer
 * @num_aes_blocks	Number of AES blocks to encrypt
 */
void aes_cbc_encrypt_blocks(uint8_t *key_exp, uint8_t *src, uint8_t *iv, uint8_t *dst, uint32_t num_aes_blocks);

/**
 * Decrypt multiple blocks of data with AES CBC.
 *
 * @key_exp		Expanded key to use
 * @src			Source data to decrypt
 * @dst			Destination buffer
 * @num_aes_blocks	Number of AES blocks to decrypt
 */
void aes_cbc_decrypt_blocks(uint8_t *key_exp, uint8_t *src, uint8_t *iv, uint8_t *dst, uint32_t num_aes_blocks);

void aes_do_encrypt(uint8_t *src, uint8_t *key, uint8_t *iv, uint8_t *cipher);
#endif /* _AES_REF_H_ */
