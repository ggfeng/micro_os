#ifndef _YODALITE_TYPES_H_
#define _YODALITE_TYPES_H_

#ifndef int8_t
#define int8_t char
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef int16_t
#define int16_t  short
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef int32_t
#define int32_t   int
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef int64_t
#define int64_t long long
#endif

#ifndef uint64_t
#define uint64_t unsigned long long
#endif

#ifndef ssize_t
#define ssize_t  signed long int
#endif

#ifndef size_t
#define size_t unsigned long
#endif

#ifndef  long_t
#define long_t   long
#endif

#ifndef ulong_t
#define ulong_t unsigned long
#endif

#ifndef bool
#define bool unsigned char
#endif

#ifndef FALSE 
#define FALSE (0)
#endif

#ifndef TRUE 
#define TRUE (1)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef ERROR
#define ERROR -1
#endif

#ifndef OK
#define OK 0
#endif

#ifndef intptr_t
#define intprt_t void*
#endif

#ifndef caps_t
#define caps_t char*
#endif

#endif
