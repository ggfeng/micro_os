#ifndef _AUTH_H_
#define _AUTH_H_
int auth_rokid_aes(char *src, char *key, char *dst);
int sign_verify(int8_t *sn, int8_t *chipid, int8_t *sign);
int auth_check(void);
#endif

