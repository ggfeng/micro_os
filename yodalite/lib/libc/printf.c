/**
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 *
 */

#include <stdarg.h>
#include <lib/libc/yodalite_libc.h>

#define BUF_SZ (128)

#ifdef CONFIG_YODALITE_PRINTF_ENABLE 

static put_handler_t hard_putchar = NULL;
static get_handler_t hard_getchar = NULL;

int yodalite_register_putchar(put_handler_t func)
{
    if(func)
     hard_putchar = func;

    return 0;
}

void  yodalite_unregister_putchar(void)
{
      hard_putchar = NULL;
}


int yodalite_register_getchar(get_handler_t func)
{
    if(func)
     hard_getchar = func;

    return 0;
}

void  yodalite_unregister_getchar(void)
{
      hard_getchar = NULL;
}


int yodalite_putchar(int c)
{
    if(hard_putchar)
      return  hard_putchar(c); 

}

int yodalite_getchar(void)
{
   if(hard_getchar)
    return hard_getchar();
}


static int32_t yodalite_vsnprintf(int8_t *buf, uint32_t size, const int8_t *fmt, va_list ap)
{
    int8_t * p;
    int32_t  i, base, idx, width, sign = 0, lead, uc = 0;
    int32_t  val, flag;
    uint32_t uval;
    int8_t * s, *digit;
    int8_t   tmp[20];

    p = (int8_t *)fmt;
    /* keep one char for '\0' */
    size--;

    for (i = 0; i < size && *p; p++) {
        if ('%' != *p) {
            buf[i++] = *p;
            continue;
        }
        p++;
        if ('0' == *p) {
            lead = 1;
            p++;
        } else {
            lead = 0;
        }
        width = 0;
        while (*p >= '0' && *p <= '9') {
            width *= 10;
            width += (*p - '0');
            p++;
        }
        /* skip 'l' */
        if ('l' == *p) {
            p++;
        }
        sign = uc = 0;
        switch (*p) {
            case '%':
                buf[i++] = '%';
                continue;
            case 'c':
                val      = va_arg(ap, int32_t);
                buf[i++] = val & 0xFF;
                continue;
            case 's':
                s = va_arg(ap, int8_t *);
                if (NULL == s) {
                    s = (int8_t *)"NULL";
                }
                while (i < size && *s) {
                    buf[i++] = *s++;
                }
                continue;
            case 'd':
                sign = 1;
            case 'u':
                base = 10;
                break;
            case 'X':
                uc = 1;
            case 'x':
            case 'p':
                base = 16;
                break;
            default:
                buf[i++] = '.';
                continue;
        }
        if (uc) {
            digit = (int8_t *)"0123456789ABCDEF";
        } else {
            digit = (int8_t *)"0123456789abcdef";
        }
        flag = 0;
        val  = va_arg(ap, int32_t);
        if (sign && val < 0) {
            val  = -val;
            flag = 1;
        }

        uval = (uint32_t)val;
        idx  = 0;
        do {
            tmp[idx++] = digit[uval % base];
            uval /= base;
        } while (uval);

        if (width && flag) {
            width--;
        }

        width -= idx;

        if (lead) {
            while (width-- > 0) {
                tmp[idx++] = '0';
            }
        }
        if (flag) {
            tmp[idx++] = '-';
        }
        while (width-- > 0) {
            tmp[idx++] = ' ';
        }
        while (i < size && idx > 0) {
            buf[i++] = tmp[--idx];
        }
    }
    buf[i] = '\0';
    return i;
}

int32_t yodalite_printf(char *fmt, ...)
{
    va_list args;
    int8_t  buf[BUF_SZ];
    int8_t *ptr;

    va_start(args, fmt);
    yodalite_vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);

    buf[BUF_SZ - 1] = '\0';

    ptr = buf;
    while (*ptr) {
        if(hard_putchar)
          hard_putchar(*ptr);
        ++ptr;
    }

    return strlen(buf);
}

#endif
