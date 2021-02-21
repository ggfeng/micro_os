/*
 * =====================================================================================
 *
 *       Filename:  base64.c
 *
 *    Description:  base64 encode and decode
 *
 *        Version:  1.0
 *        Created:  04/01/2019 08:38:42 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  glhou (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int base64_dec(const char *src, unsigned char *dst, int *dst_len)
{
    int x, y;
    unsigned char *p;
    unsigned char b64_decode[] = {
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
         52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,255,255,255,
        255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
         15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
        255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255
    };
/*    
    if (!(*dst = malloc((3 * (strlen(src) / 4) + 1) * sizeof(char)))) {
        return ENOMEM;
    }
*/
	p = dst;
    
    while ((x = (int)((unsigned char)(*src++))) != 0) {
        if (x > 127 || (x = b64_decode[x]) == 255) {
            goto out;
        }
        if ((y = (int)((unsigned char)(*src++))) == 0
            || (y = b64_decode[y]) == 255) {
            goto out;
        }
        *p++ = (char)(x << 2) | (y >> 4);
        if ((x = (int)((unsigned char)(*src++))) == '=') {
            if (*src++ != '=' || *src != 0) {
                goto out;
            }
        } else {
            if (x > 127 || (x = b64_decode[x]) == 255) {
                goto out;
            }
            *p++ = (char)(y << 4) | (x >> 2);
            if ((y = (int)((unsigned char)(*src++))) == '=') {
                if (*src != '\0') {
                    goto out;
                }
            } else {
                if (y > 127 || (y = b64_decode[y]) == 255) {
                    goto out;
                }
                *p++ = (char)(x << 6) | y;
            }
        }
    }
    *p = '\0';
    *dst_len = p - dst;
    return 0;

out:
    return -1;
}

int base64_enc(const unsigned char *s, int src_len, char *dst)
{
    char b64chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *p;
    int x, y;
    
#if 0
    if (!(*dst = malloc((4 * ((src_len + 2) / 3) + 1) * sizeof(char)))) {
        return -1;
    }
#endif
    p = dst;
    while (src_len-- > 0) {
        x = *s++;
        *p++ = b64chars[(x >> 2) & 63];
        if (src_len-- <= 0) {
            *p++ = b64chars[(x << 4) & 63];
            *p++ = '=';
            *p++ = '=';
            break;
        }
        y = *s++;
        *p++ = b64chars[((x << 4) | ((y >> 4) & 15)) & 63];
        if (src_len-- <= 0) {
            *p++ = b64chars[(y << 2) & 63];
            *p++ = '=';
            break;
        }
        x = *s++;
        *p++ = b64chars[((y << 2) | ((x >> 6) & 3)) & 63];
        *p++ = b64chars[x & 63];
    }
    *p = '\0';
    
    return 0;
}

