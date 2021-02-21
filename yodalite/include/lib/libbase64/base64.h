#ifndef _BASE_64_
#define _BASE_64_
int base64_dec(const char *src, unsigned char *dst, int *dst_len);

int base64_enc(const unsigned char *s, int src_len, char *dst);
#endif
