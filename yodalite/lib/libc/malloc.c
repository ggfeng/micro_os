#include <lib/libc/yodalite_libc.h>

#ifdef CONFIG_YODALITE_MALLOC_ENABLE

#define  YODALITE_MALLOC_INIT_FLAG (1)

typedef struct Block Block;

struct Block {
    void *addr;
    Block *next;
    size_t size;
};


typedef struct heap
{
    Block *free;   // first free block
    Block *used;   // first used block
    Block *fresh;  // first available blank block
    size_t top;    // top free addr
    int flag;
    Block blocks[CONFIG_MALLOC_BLOCK_COUNTS];

} Heap;

#ifdef CONFIG_ABSOLUT
static Heap *heap = (Heap *)CONFIG_MALLOC_POOL_BASE;
#endif

#ifdef CONFIG_GLOBAL
static Heap g_malloc_heap; 
static Heap *heap= (Heap*)&g_malloc_heap;
#endif 

static void insert_block(Block *block) 
{
    Block *ptr  = heap->free;
    Block *prev = NULL;
    while (ptr != NULL) {
        if ((size_t)block->addr <= (size_t)ptr->addr) {
            break;
        }
        prev = ptr;
        ptr  = ptr->next;
    }
    if (prev != NULL) {
        prev->next = block;
    } else {
        heap->free = block;
    }
    block->next = ptr;
}

static void release_blocks(Block *scan, Block *to) 
{
    Block *scan_next;
    while (scan != to) {
        scan_next   = scan->next;
        scan->next  = heap->fresh;
        heap->fresh = scan;
        scan->addr  = 0;
        scan->size  = 0;
        scan        = scan_next;
    }
}

static void compact(void) 
{
    Block *ptr = heap->free;
    Block *prev;
    Block *scan;
    while (ptr != NULL) {
        prev = ptr;
        scan = ptr->next;
        while (scan != NULL &&
               (size_t)prev->addr + prev->size == (size_t)scan->addr) {
            prev = scan;
            scan = scan->next;
        }
        if (prev != ptr) {
            size_t new_size =(size_t)prev->addr - (size_t)ptr->addr + prev->size;
            ptr->size   = new_size;
            Block *next = prev->next;
            // make merged blocks available
            release_blocks(ptr->next, prev->next);
            // relink
            ptr->next = next;
        }
        ptr = ptr->next;
    }
}

static bool yoda_init(void) 
{
    size_t i;

    heap->free   = NULL;
    heap->used   = NULL;
    heap->fresh  = heap->blocks;
    heap->top    = (uint32_t)heap + sizeof(Heap);
    Block *block = heap->blocks;
    i = CONFIG_MALLOC_BLOCK_COUNTS- 1;
    while (i--) {
        block->next = block + 1;
        block++;
    }
    block->next = NULL;
    return TRUE;
}

void yoda_free(void *free) 
{
    Block *block = heap->used;
    Block *prev  = NULL;
    while (block != NULL) {
        if (free == block->addr) {
            if (prev) {
                prev->next = block->next;
            } else {
                heap->used = block->next;
            }
            insert_block(block);
            compact();
            break;
        }
        prev  = block;
        block = block->next;
    }
    return;
}

static Block *alloc_block(size_t num) 
{
    size_t new_top;
    Block *ptr  = heap->free;
    Block *prev = NULL;
    size_t top  = heap->top;
    num         = (num + CONFIG_MALLOC_ALIGN_BYTES - 1) & -CONFIG_MALLOC_ALIGN_BYTES;

   if(heap->flag != YODALITE_MALLOC_INIT_FLAG){
        if(yoda_init() == TRUE){
            heap->flag = YODALITE_MALLOC_INIT_FLAG;
        }
        else
          return NULL; 
    }

    while (ptr != NULL) {
        const int is_top = ((size_t)ptr->addr + ptr->size >= top) && ((size_t)ptr->addr + num <= CONFIG_MALLOC_POOL_SIZE);
        if (is_top || ptr->size >= num) {
            if (prev != NULL) {
                prev->next = ptr->next;
            } else {
                heap->free = ptr->next;
            }
            ptr->next  = heap->used;
            heap->used = ptr;
            if (is_top) {
                ptr->size = num;
                heap->top = (size_t)ptr->addr + num;
            } else if (heap->fresh != NULL) {
                size_t excess = ptr->size - num;
                if (excess >= CONFIG_MALLOC_SPLIT_THRESH) {
                    ptr->size    = num;
                    Block *split = heap->fresh;
                    heap->fresh  = split->next;
                    split->addr  = (void *)((size_t)ptr->addr + num);
                    split->size = excess;
                    insert_block(split);
                    compact();
                }
            }
            return ptr;
        }
        prev = ptr;
        ptr  = ptr->next;
    }

    new_top = top + num;

    if (heap->fresh != NULL && new_top <= CONFIG_MALLOC_POOL_SIZE) {
        ptr         = heap->fresh;
        heap->fresh = ptr->next;
        ptr->addr   = (void *)top;
        ptr->next   = heap->used;
        ptr->size   = num;
        heap->used  = ptr;
        heap->top   = new_top;
        return ptr;
    }
    return NULL;
}

void *yoda_malloc(size_t num) 
{
    Block *block = alloc_block(num);
    if (block != NULL) {
        return block->addr;
    }
    return NULL;
}

static void memclear(void *ptr, size_t num) 
{
    uint8_t *ptrb;
    size_t *ptrw = (size_t *)ptr;
    size_t numw  = (num & -sizeof(size_t)) / sizeof(size_t);
    while (numw--) {
        *ptrw++ = 0;
    }
    num &= (sizeof(size_t) - 1);
    ptrb = (uint8_t *)ptrw;
    while (num--) {
        *ptrb++ = 0;
    }
}

void *yoda_calloc(size_t num, size_t size) 
{
    num *= size;
    Block *block = alloc_block(num);
    if (block != NULL) {
        memclear(block->addr, num);
        return block->addr;
    }
    return NULL;
}


static size_t get_malloc_size(void *ptr) 
{
    Block *prev;
    Block *block = heap->used;

    while (block != NULL) {
        if (ptr == block->addr) {
           return block->size;
        }
        prev  = block;
        block = block->next;
    }
    return 0;
}

void *yoda_realloc(void *ptr, size_t size)
{
    size_t counts;
    Block *block = alloc_block(size);

    if (block != NULL) 
    {
        if( (counts= get_malloc_size(ptr)) >0)
        {
           counts = counts > size ? size:counts;
           memcpy(block->addr,ptr,counts);

           free(ptr); 
           return block->addr;
        }
    }

    return NULL;
 }

#endif
