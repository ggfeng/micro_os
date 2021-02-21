#ifndef _FLORA_CAPS_H_
#define _FLORA_CAPS_H_

#include <yodalite_autoconf.h>
#include <inttypes.h>
#include <osal/pthread.h>

#define CAPS_MEMBER_TYPE_INTEGER 'i'
#define CAPS_MEMBER_TYPE_FLOAT 'f'
#define CAPS_MEMBER_TYPE_LONG 'l'
#define CAPS_MEMBER_TYPE_DOUBLE 'd'
#define CAPS_MEMBER_TYPE_STRING 'S'
#define CAPS_MEMBER_TYPE_BINARY 'B'
#define CAPS_MEMBER_TYPE_OBJECT 'O'
#define CAPS_MEMBER_TYPE_VOID 'V'

typedef struct {
    const void* data;
    uint32_t length;
} flora_binary_t;

typedef union {
    int32_t i32;
    int64_t i64;
    float f;
    double d;
    const char* s;
    flora_binary_t b;
} misc_container_t;

struct flora_caps{
    char mem_type;
    misc_container_t mem;
    void* ext;
    int ext_size;
    int read;
    pthread_mutex_t* pmutex;
    struct flora_caps* next;
};
typedef struct flora_caps flora_caps_t;

#ifndef caps_t
#define caps_t flora_caps_t*
#endif

extern caps_t caps_create(void);

// parse创建的caps_t对象可读，不可写
extern int32_t caps_parse(void* data, uint32_t length, caps_t* result);

// 如果serialize生成的数据长度大于'bufsize'，将返回所需的buf size，
// 但'buf'不会写入任何数据，需外部重新分配更大的buf，再次调用serialize
extern int32_t caps_serialize(caps_t caps, void* buf, uint32_t bufsize);

extern int32_t caps_write_integer(caps_t caps, int32_t v);
extern int32_t caps_write_long(caps_t caps, int64_t v);
extern int32_t caps_write_float(caps_t caps, float v);
extern int32_t caps_write_double(caps_t caps, double v);
extern int32_t caps_write_string(caps_t caps, const char* v);
extern int32_t caps_write_binary(caps_t caps, const void* data, uint32_t length);
extern int32_t caps_write_object(caps_t caps, caps_t v);
extern int32_t caps_write_void(caps_t caps);
extern int32_t caps_read_integer(caps_t caps, int32_t* r);
extern int32_t caps_read_long(caps_t caps, int64_t* r);
extern int32_t caps_read_float(caps_t caps, float* r);
extern int32_t caps_read_double(caps_t caps, double* r);
extern int32_t caps_read_string(caps_t caps, const char** r);
extern int32_t caps_read_binary(caps_t caps, const void** r, uint32_t* length);
extern int32_t caps_read_object(caps_t caps, caps_t* r);
extern int32_t caps_read_void(caps_t caps);
extern void caps_destroy(caps_t caps);

#endif  /*_FLORA_CAPS_H_*/
