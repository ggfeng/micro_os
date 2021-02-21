/*
 * (C) Copyright 2019 Rokid Corp.
 * Zhu Bin <bin.zhu@rokid.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <yodalite_autoconf.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include <osal/semaphore.h>
#include <osal/pthread.h>
#include <osal/time.h>
#include <hapi/caps.h>
#include <lib/mem_ext/mem_ext.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif
#define CAPS_LOG    printf

static caps_t caps_goto_end(caps_t caps)
{
    caps_t ret = caps;
    if(!ret)
        return ret;
    while(ret) {
        if(!ret->next)
            break;
        ret = ret->next;
    }
    return ret;
}

static caps_t caps_sub_create(void)
{
    caps_t ret;
    ret = (flora_caps_t*)yodalite_malloc(sizeof(flora_caps_t));
    if(ret) {
        memset(ret, 0, sizeof(flora_caps_t));
    }
    return ret;
}

caps_t caps_create(void)
{
    caps_t ret;
    ret = (flora_caps_t*)yodalite_malloc(sizeof(flora_caps_t));
    if(ret) {
        memset(ret, 0, sizeof(flora_caps_t));
        ret->pmutex = (pthread_mutex_t*)yodalite_malloc(sizeof(pthread_mutex_t));
        if(!ret->pmutex) {
            yodalite_free(ret);
            CAPS_LOG("caps_create error:no enough memory\n");
            return NULL;
        }
        pthread_mutex_init(ret->pmutex, NULL);
    }
    return ret;
}

int32_t caps_parse(void* data, uint32_t length, caps_t* result)
{
    pthread_mutex_t* pmutex;
    int32_t caps_size = 0;
    caps_t trace;
    void* src = data;
    *result = caps_create();
    trace = *result;
    if(!trace)
        return -1;
    pmutex = trace->pmutex;
    while(caps_size < length && trace) {
        memcpy(trace, src, sizeof(flora_caps_t));
        if(trace == *result)
            trace->pmutex = pmutex;
        else
            trace->pmutex = NULL;
        trace->next = NULL;
        src += sizeof(flora_caps_t);
        caps_size += sizeof(flora_caps_t);
        if(trace->ext_size) {
            trace->ext = (void*)yodalite_malloc(trace->ext_size);
            if(!trace->ext)
                return -1;
            memcpy(trace->ext, src, trace->ext_size);
            src += trace->ext_size;
            caps_size += trace->ext_size;
        }
        if(caps_size < length) {
            trace->next = caps_sub_create();
            trace = trace->next;
        }
    }
    if(!trace)
        return -1;
    else
        return 0;

}

int32_t caps_serialize(caps_t caps, void* buf, uint32_t bufsize)
{
    pthread_mutex_t* pmutex;
    int32_t caps_size = 0;
    caps_t trace = caps;
    void* dest;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    while(trace) {
        caps_size += sizeof(flora_caps_t);
        caps_size += trace->ext_size;
        trace = trace->next;
    }
    if(caps_size <= bufsize) {
        trace = caps;
        dest = buf;
        while(trace) {
            memcpy(dest, trace, sizeof(flora_caps_t));
            dest += sizeof(flora_caps_t);
            if(trace->ext_size) {
                memcpy(dest, trace->ext, trace->ext_size);
                dest += trace->ext_size;
            }
            trace = trace->next;
        }
    }
    pthread_mutex_unlock(pmutex);
    return caps_size;
}

int32_t caps_write_integer(caps_t caps, int32_t v)
{
    pthread_mutex_t* pmutex;
    caps_t end;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    if(caps->mem_type) {
        end = caps_goto_end(caps);
        end->next = caps_sub_create();
        if(!end->next) {
            pthread_mutex_unlock(pmutex);
            return -1;
        }
        end->next->mem_type = CAPS_MEMBER_TYPE_INTEGER;
        end->next->mem.i32 = v;
    } else {
        caps->mem_type = CAPS_MEMBER_TYPE_INTEGER;
        caps->mem.i32 = v;
    }
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_write_long(caps_t caps, int64_t v)
{
    pthread_mutex_t* pmutex;
    caps_t end;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    if(caps->mem_type) {
        end = caps_goto_end(caps);
        end->next = caps_sub_create();
        if(!end->next) {
            pthread_mutex_unlock(pmutex);
            return -1;
        }
        end->next->mem_type = CAPS_MEMBER_TYPE_LONG;
        end->next->mem.i64 = v;
    } else {
        caps->mem_type = CAPS_MEMBER_TYPE_LONG;
        caps->mem.i64 = v;
    }
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_write_float(caps_t caps, float v)
{
    pthread_mutex_t* pmutex;
    caps_t end;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    if(caps->mem_type) {
        end = caps_goto_end(caps);
        end->next = caps_sub_create();
        if(!end->next) {
            pthread_mutex_unlock(pmutex);
            return -1;
        }
        end->next->mem_type = CAPS_MEMBER_TYPE_FLOAT;
        end->next->mem.f = v;
    } else {
        caps->mem_type = CAPS_MEMBER_TYPE_FLOAT;
        caps->mem.f = v;
    }
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_write_double(caps_t caps, double v)
{
    pthread_mutex_t* pmutex;
    caps_t end;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    if(caps->mem_type) {
        end = caps_goto_end(caps);
        end->next = caps_sub_create();
        if(!end->next) {
            pthread_mutex_unlock(pmutex);
            return -1;
        }
        end->next->mem_type = CAPS_MEMBER_TYPE_DOUBLE;
        end->next->mem.d = v;
    } else {
        caps->mem_type = CAPS_MEMBER_TYPE_DOUBLE;
        caps->mem.d = v;
    }
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_write_string(caps_t caps, const char* v)
{
    pthread_mutex_t* pmutex;
    caps_t end;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    if(caps->mem_type) {
        end = caps_goto_end(caps);
        end->next = caps_sub_create();
        if(!end->next) {
            pthread_mutex_unlock(pmutex);
            return -1;
        }
        end->next->mem_type = CAPS_MEMBER_TYPE_STRING;
        end->next->mem.s = v;
    } else {
        caps->mem_type = CAPS_MEMBER_TYPE_STRING;
        caps->mem.s = v;
    }
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_write_binary(caps_t caps, const void* data, uint32_t length)
{
    pthread_mutex_t* pmutex;
    caps_t end;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    if(caps->mem_type) {
        end = caps_goto_end(caps);
        end->next = caps_sub_create();
        if(!end->next) {
            pthread_mutex_unlock(pmutex);
            return -1;
        }
        end->next->mem_type = CAPS_MEMBER_TYPE_BINARY;
        end->next->mem.b.data = data;
        end->next->mem.b.length = length;
    } else {
        caps->mem_type = CAPS_MEMBER_TYPE_BINARY;
        caps->mem.b.data = data;
        caps->mem.b.length = length;
    }
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_write_object(caps_t caps, caps_t v)
{
    pthread_mutex_t* pmutex;
    caps_t dest;
    caps_t src;
    if(!caps || !v)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    if(caps->mem_type) {
        dest = caps_goto_end(caps);
        dest->next = caps_sub_create();
        if(!dest->next) {
            pthread_mutex_unlock(pmutex);
            return -1;
        }
        dest = dest->next;
    } else {
        dest = caps;
    }
    src = v;
    do {
        memcpy(dest, src, sizeof(flora_caps_t));
        dest->ext = NULL;
        dest->ext_size = 0;
        dest->read = 0;
        if(dest == caps)
            dest->pmutex = pmutex;
        else
            dest->pmutex = NULL;
        dest->next = NULL;
        src = src->next;
        if(src) {
            dest->next = caps_sub_create();
            if(!dest->next) {
                pthread_mutex_unlock(pmutex);
                return -1;
            }
            dest = dest->next;
        }
    } while(src);
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_write_void(caps_t caps)
{
    pthread_mutex_t* pmutex;
    caps_t trace = caps;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    while(trace) {
        if(trace->mem_type == CAPS_MEMBER_TYPE_STRING) {
            if(trace->ext_size != strlen(trace->mem.s)+1) {
                if(trace->ext) {
                    yodalite_free(trace->ext);
                    trace->ext = NULL;
                    trace->ext_size = 0;
                }
                trace->ext = (void*)yodalite_malloc(strlen(trace->mem.s)+1);
                if(!trace->ext) {
                    pthread_mutex_unlock(pmutex);
                    CAPS_LOG("caps_write_void ERROR: no enough memory\n");
                    return -1;
                }
                memset(trace->ext, 0, strlen(trace->mem.s)+1);
            } else {
                memset(trace->ext, 0, trace->ext_size);
            }
            strcpy(trace->ext, trace->mem.s);
            trace->ext_size = strlen(trace->mem.s)+1;
        } else if(trace->mem_type == CAPS_MEMBER_TYPE_BINARY) {
            if(trace->ext_size != trace->mem.b.length) {
                if(trace->ext) {
                    yodalite_free(trace->ext);
                    trace->ext = NULL;
                    trace->ext_size = 0;
                }
                trace->ext = (void*)yodalite_malloc(trace->mem.b.length);
                if(!trace->ext) {
                    pthread_mutex_unlock(pmutex);
                    CAPS_LOG("caps_write_void ERROR: no enough memory\n");
                    return -1;
                }
                memset(trace->ext, 0, trace->mem.b.length);
            } else {
                memset(trace->ext, 0, trace->ext_size);
            }
            memcpy(trace->ext, trace->mem.b.data, trace->mem.b.length);
            trace->ext_size = trace->mem.b.length;
        }
        trace = trace->next;
    }
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_read_integer(caps_t caps, int32_t* r)
{
    pthread_mutex_t* pmutex;
    caps_t trace = caps;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    while(trace) {
        if(!trace->read)
            break;
        trace = trace->next;
    }
    if(!trace) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    if(trace->mem_type != CAPS_MEMBER_TYPE_INTEGER) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    *r = trace->mem.i32;
    trace->read = 1;
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_read_long(caps_t caps, int64_t* r)
{
    pthread_mutex_t* pmutex;
    caps_t trace = caps;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    while(trace) {
        if(!trace->read)
            break;
        trace = trace->next;
    }
    if(!trace) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    if(trace->mem_type != CAPS_MEMBER_TYPE_LONG) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    *r = trace->mem.i64;
    trace->read = 1;
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_read_float(caps_t caps, float* r)
{
    pthread_mutex_t* pmutex;
    caps_t trace = caps;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    while(trace) {
        if(!trace->read)
            break;
        trace = trace->next;
    }
    if(!trace) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    if(trace->mem_type != CAPS_MEMBER_TYPE_FLOAT) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    *r = trace->mem.f;
    trace->read = 1;
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_read_double(caps_t caps, double* r)
{
    pthread_mutex_t* pmutex;
    caps_t trace = caps;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    while(trace) {
        if(!trace->read)
            break;
        trace = trace->next;
    }
    if(!trace) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    if(trace->mem_type != CAPS_MEMBER_TYPE_DOUBLE) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    *r = trace->mem.d;
    trace->read = 1;
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_read_string(caps_t caps, const char** r)
{
    pthread_mutex_t* pmutex;
    caps_t trace = caps;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    while(trace) {
        if(!trace->read)
            break;
        trace = trace->next;
    }
    if(!trace) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    if(trace->mem_type != CAPS_MEMBER_TYPE_STRING) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    *r = trace->mem.s;
    trace->read = 1;
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_read_binary(caps_t caps, const void** r, uint32_t* length)
{
    pthread_mutex_t* pmutex;
    caps_t trace = caps;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    while(trace) {
        if(!trace->read)
            break;
        trace = trace->next;
    }
    if(!trace) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    if(trace->mem_type != CAPS_MEMBER_TYPE_BINARY) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    *r = trace->mem.b.data;
    *length = trace->mem.b.length;
    trace->read = 1;
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_read_object(caps_t caps, caps_t* r)
{
    pthread_mutex_t* pmutex;
    pthread_mutex_t* r_pmutex;
    caps_t trace = caps;
    caps_t dest;
    if(!caps || !r)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    while(trace) {
        if(!trace->read)
            break;
        trace = trace->next;
    }
    if(!trace) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    *r = caps_create();
    if(!(*r)) {
        pthread_mutex_unlock(pmutex);
        return -1;
    }
    dest = *r;
    r_pmutex = dest->pmutex;
    do {
        memcpy(dest, trace, sizeof(flora_caps_t));
        dest->read = 0;
        if(dest == *r)
            dest->pmutex = r_pmutex;
        else
            dest->pmutex = NULL;
        dest->next = NULL;
        trace->read = 1;
        trace = trace->next;
        if(trace) {
            dest->next = caps_sub_create();
            if(!dest->next) {
                pthread_mutex_unlock(pmutex);
                return -1;
            }
            dest = dest->next;
        }
    } while(trace);
    pthread_mutex_unlock(pmutex);
    return 0;
}

int32_t caps_read_void(caps_t caps)
{
    pthread_mutex_t* pmutex;
    caps_t trace = caps;
    if(!caps)
        return -1;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    while(trace) {
        if(trace->mem_type == CAPS_MEMBER_TYPE_STRING) {
            trace->mem.s = trace->ext;
        } else if(trace->mem_type == CAPS_MEMBER_TYPE_BINARY) {
            trace->mem.b.data = trace->ext;
            trace->mem.b.length = trace->ext_size;
        }
        trace = trace->next;
    }
    pthread_mutex_unlock(pmutex);
    return 0;
}

void caps_destroy(caps_t caps)
{
    pthread_mutex_t* pmutex;
    caps_t trace = caps;
    if(!caps)
        return;
    pmutex = caps->pmutex;
    pthread_mutex_lock(pmutex);
    while(trace) {
        caps_t tmp = trace;
        if(trace->ext)
            yodalite_free(trace->ext);
        trace = trace->next;
        yodalite_free(tmp);
    }
    pthread_mutex_unlock(pmutex);
    pthread_mutex_destroy(pmutex);
    yodalite_free(pmutex);
}

