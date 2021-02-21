#ifndef _PAL_AUTH_H_
#define _PAL_AUTH_H_

#include <stdint.h>

#define PAL_RSANUMBYTES 256           /* 2048 bit key length */
#define PAL_RSANUMWORDS (PAL_RSANUMBYTES / sizeof(uint32_t))

struct rsapubkey {
    int len;                  /* Length of n[] in number of uint32_t */
    uint32_t n0inv;           /* -1 / n[0] mod 2^32 */
    uint32_t n[PAL_RSANUMWORDS];  /* modulus as little endian array */
    uint32_t rr[PAL_RSANUMWORDS]; /* R^2 as little endian array */
    int exponent;             /* 3 or 65537 */
};

struct auth_lapi {
    int (*yodalite_auth_get_sn)(unsigned char *sn);
    int (*yodalite_auth_get_chipid)(unsigned char *chipid);
    int (*yodalite_auth_get_signature)(unsigned char *signature);
    int (*yodalite_auth_get_pubkey)(struct rsapubkey *pubkey);
};

extern int pal_auth_init(struct auth_lapi *platform_auth_lapi);
extern int pal_auth_get_sn(unsigned char *sn);
extern int pal_auth_get_chipid(unsigned char *chipid);
extern int pal_auth_get_signature(unsigned char *signature);
extern int pal_auth_get_pubkey(struct rsapubkey *pubkey);

#endif  /*_PAL_AUTH_H_*/
