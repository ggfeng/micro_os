/**
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef _YODALITE_LIBC_H_
#define _YODALITE_LIBC_H_

#include <yodalite_autoconf.h>

#include <lib/yodalite_types.h>

#ifdef CONFIG_YODALITE_PRINTF_ENABLE 
typedef int(*put_handler_t)(int8_t ch);
typedef int(*get_handler_t)(void);

extern  int  yodalite_register_putchar(put_handler_t func);
extern  int  yodalite_register_getchar(get_handler_t func);
extern  void yodalite_unregister_putchar(void);
extern void yodalite_unregister_getchar(void);

extern int yodalite_putchar(int c);
extern int yodalite_getchar(void);

extern int32_t yodalite_printf(char *fmt, ...);

#ifdef printf
#undef printf
#endif

#ifdef putchar
#undef putchar 
#endif

#ifdef getchar
#undef getchar
#endif

#define getchar yodalite_getchar
#define putchar yodalite_putchar
#define printf yodalite_printf

#endif


#ifdef CONFIG_YODALITE_STRING_ENABLE 
extern int8_t* yodalite_strcpy(int8_t *dest, const int8_t *src);
extern int8_t* yodalite_strncpy(int8_t *dest, const int8_t *src, size_t n);
extern int32_t yodalite_strcmp(const int8_t *s1, const int8_t *s2);
extern int32_t yodalite_strncmp(const int8_t *s1, const int8_t *s2, size_t n);
extern size_t  yodalite_strlen(const int8_t *s);
extern int8_t* yodalite_strrchr(const int8_t *s, int32_t c);
extern void *  yodalite_memset(void *s, int32_t c, size_t n);
extern void *  yodalite_memcpy(void *dest, const void *src, size_t n);
extern int32_t yodalite_memcmp(const void *s1, const void *s2, size_t n);
extern void   *yodalite_memmove(void *dest, const void *src, size_t count);

extern size_t yodalite_strnlen(const char *s, size_t count);
extern char *yodalite_strncat(char *dest, const char *src, size_t count);
extern char *yodalite_strcat(char *dest, const char *src);
extern char *yodalite_strstr(const char *s1, const char *s2);

extern char *yodalite_strchr(const char *s, int c);
extern char *yodalite_strpbrk(const char *cs, const char *ct);
extern char *yodalite_strsep(char **s, const char *ct);

#ifdef strcpy
#undef strcpy
#endif

#ifdef strncpy
#undef strncpy
#endif

#ifdef strcmp
#undef strcmp
#endif

#ifdef strncmp
#undef strncmp
#endif

#ifdef strlen
#undef strlen
#endif

#ifdef strrchr
#undef strrchr
#endif

#ifdef memset
#undef memset
#endif

#ifdef memcpy
#undef memcpy
#endif

#ifdef memcmp
#undef memcmp
#endif

#ifdef memmove
#undef memmove
#endif

#ifdef strnlen
#undef strnlen
#endif

#ifdef strncat
#undef strncat
#endif

#ifdef strcat
#undef strcat
#endif

#ifdef strstr
#undef strstr
#endif

#ifdef strchr
#undef strchr
#endif

#ifdef strpbrk
#undef strpbrk
#endif

#ifdef strsep
#undef strsep
#endif


#define  strcpy   yodalite_strcpy
#define  strncpy  yodalite_strncpy
#define  strcmp   yodalite_strcmp 
#define  strncmp  yodalite_strncmp
#define  strlen   yodalite_strlen
#define  strrchr  yodalite_strrchr
#define  memset   yodalite_memset
#define  memcpy   yodalite_memcpy
#define  memcmp   yodalite_memcmp
#define  memmove  yodalite_memmove 

#define  strnlen yodalite_strnlen
#define  strncat yodalite_strncat
#define  strcat  yodalite_strcat
#define  strstr  yodalite_strstr


#define strchr  yodalite_strchr
#define strpbrk yodalite_strpbrk
#define strsep  yodalite_strsep

#endif

#ifdef CONFIG_YODALITE_MALLOC_ENABLE
#ifdef malloc
#undef malloc
#endif

#ifdef calloc
#undef calloc
#endif

#ifdef free
#undef free
#endif

#ifdef realloc
#undef realloc
#endif

#define malloc   yoda_malloc
#define calloc   yoda_calloc
#define realloc  yoda_realloc
#define free     yoda_free

#endif

#endif /* _STRING_H_ */
