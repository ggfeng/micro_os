#ifndef _BM_H_
#define _BM_H_

#define BM_LOG		printf
#define BM_MALLOC   malloc
#define BM_FREE     free

#ifndef __UINT
#define __UINT
typedef unsigned int    UINT;
#endif

#ifndef __VOID
#define __VOID
typedef void    VOID;
#endif

#ifndef __SINT
#define __SINT
typedef signed int    SINT;
#endif

#ifndef __BOOL
#define __BOOL
typedef SINT    BOOL;

#define TRUE    1
#define FALSE    0
#endif

/********************************************************
 *
 *	Enable to create the free buffer pool over discontinuous memory
 *	region.
 *
 *	If _BM_ADD_FREEPOOL is defined, the free buffer pool can be
 *	created over multiple discontinuous memory regions.
 *	First it is created with BM_initializeFreePool() and added by calling
 *	BM_addFreePool() as many times as necessary.
 *
 *	This should be enabled if _API_CAP_GLB_SR_BY_UNIT is enabled.
 *	In other case, it can be optionally turned on or off.
 *
 *	(20061011,ishan) 
 ********************************************************/

typedef enum {
	STATUS_FREE = 0,				/* in free list */
	STATUS_ALLOC,					/* allocate buffer */
	STATUS_LAST_ALLOC
#if _API_CAP_GLB_BM_SUPPORT_TRICK_LAST
	,
	STATUS_LAST_TRICK_ALLOC		/* indicates that current is the last of PLAY UNIT (at MP3/WMA TRICK PLAY Mode only )*/
#endif
#if _API_CAP_GLB_DIVX_SLOW_REV
	,
	STATUS_LAST_CHUNK_ALLOC		/* indicates the last track node for every play unit in DIVX slow reverse mode -- zb 070227*/
#endif
} BM_STATUS;

typedef enum {
	STATUS_INVALID = 0,
	STATUS_VALID
} BM_VALID;

typedef enum {
	STATUS_UNBIND = 0,
	STATUS_PREPARE_UNBIND,
	STATUS_BIND
} BM_BIND_STATUS;


// Buffer Manager return value
typedef enum {
	BM_ILLEGAL_POOL_PT = 0,		// illegal pool pointer
	BM_ILLEGAL_FILLED_LIST_PT,	// illegal filled list pointer
	BM_ILLEGAL_LINK_LIST_PT,		// illegal link list pointer
	BM_ILLEGAL_LINK_PT,			// illegal link pointer
	BM_ILLEGAL_NODE_PT,			// illegal node pointer
	BM_ILLEGAL_START,				// illegal buffer start address
	BM_ILLEGAL_SIZE,				// illegal buffer size
	BM_ILLEGAL_COUNT,				// illegal buffer count
	BM_ILLEGAL_CALLBACK,			// illegal callback function
	BM_FREE_LINK_EMPTY,			// illegal free link
	BM_FREE_NODE_EMPTY,
	BM_NOERROR					// no error
}RET_STATUS;

typedef enum {
	BM_CALL_EVERY_TIME = 0,
	BM_CALL_AT_ONCE, 
	BM_CALL_NULL
} BM_CallbackType_et;

typedef enum {
	STATUS_FULL = 0,
	STATUS_PARTIAL
} BM_BOUNDARY_STATUS;

/*typedef struct bm_boundaryInfo {
	VOID *			start;
	VOID *			end;
	unsigned int		status;
	struct bm_boundaryInfo *next;
}BM_BOUNDARY;*/

typedef enum {
	BM_ALLOC_MSG = 0,
	BM_ALLOC_SDIN,
	BM_ALLOC_SCLR,
	BM_ALLOC_PREP,
	BM_ALLOC_VE,
	BM_ALLOC_PSM,
	BM_ALLOC_SAIU,
	BM_ALLOC_ADM
} BM_ALLOC_STATE;

typedef struct bm_freepool BM_FREEPOOL;
typedef struct bm_filledlist BM_FILLEDLIST;

/* free buffer node control block */
typedef struct bm_node {
	BM_STATUS		status;				/* buffer status */
	BM_FREEPOOL *	where;				/* pool pointer which node belongs to. */
	UINT			who_alloc;
	unsigned int		refCount;			/* reference count */
	VOID *			bAddress;			/* buffer start address */
	VOID *			extraInfo;
}BM_NODE;

/* node link data structure */
typedef struct bm_link BM_LINK;
struct bm_link {
	BM_NODE *		node;
	BM_LINK *		next;
};

/* node list head & tail pointer */
typedef struct bm_list BM_LIST;
struct bm_list {
	UINT			count;
	BM_LINK *		head;
	BM_LINK *		tail;
};

typedef int (*BM_FREECALLBACK)(int Parameter1, int Parameter2);
typedef int (*BM_FILLEDCALLBACK)(int Parameter1, int Parameter2);

/* Filled buffer list control block */
struct bm_filledlist {
	BM_VALID			valid;
	BM_BIND_STATUS		eBindStatus;
	unsigned int 			bCount;			/* filled buffer count */
	BM_LIST 				list;				/* filled list pointer */
	BM_FILLEDCALLBACK	function;	/* input callback function */
	int					para1;			/* callback function para1 */
	int					para2;			/* callback function para2 */
	BM_CallbackType_et	callbackType;
};

/* free buffer pool control block */
struct bm_freepool {
	BM_VALID			valid;
	unsigned int 			bSize;			/* buffer size */
	unsigned int			bTotalCount;		
	unsigned int 			bCount;			/* free buffer count */
	VOID *				start;			/* buffer start address */
	BM_LIST 				list;				/* free list pointer */
	BM_FREEPOOL 		*msg_free;
	BM_FILLEDLIST 	 	msg_filled;
};



extern RET_STATUS BM_initialize (VOID);
/* FREE Buffer APIs */
extern RET_STATUS BM_initializeFreePool 	(BM_FREEPOOL *pool, char *start, unsigned int size, unsigned int count, BOOL initExInfo, VOID *exStart, UINT exSize);
#if _BM_ADD_FREEPOOL
extern RET_STATUS BM_addFreePool 	(BM_FREEPOOL *pool, char *start, unsigned int size, unsigned int count, BOOL initExInfo, VOID *exStart, UINT exSize);
extern RET_STATUS BM_addVBVFreePool 	(BM_FREEPOOL *pool, char *start, 
					unsigned int size, unsigned int count, BOOL initExInfo, VOID *exStart, UINT exSize);
#endif

extern RET_STATUS BM_deinitializeFreePool (BM_FREEPOOL *pool);
extern BM_NODE * BM_requestFreeBuffer(BM_FREEPOOL *pool, BM_FREECALLBACK function, int para1, int para2);
extern BM_NODE * BM_requestFreeBufferOverlap(BM_FREEPOOL *pool, BM_FREECALLBACK function, int para1, int para2);

extern RET_STATUS BM_releaseFreeBuffer (BM_NODE *node);
extern RET_STATUS BM_registFreeCallback (BM_FREEPOOL *pool, BM_FREECALLBACK function, int para1, int para2);

/* Filled Buffer APIs */
extern RET_STATUS BM_initializeFilledList (BM_FILLEDLIST *list);
extern RET_STATUS BM_deinitializeFilledList (BM_FILLEDLIST *list);
extern RET_STATUS BM_putFilledBuffer (BM_FILLEDLIST *list, BM_NODE *node);
extern BM_NODE * BM_getFilledBuffer(BM_FILLEDLIST *list, BM_FILLEDCALLBACK function, int para1, int para2, BM_CallbackType_et type);
extern BM_NODE * BM_peekFilledBuffer(BM_FILLEDLIST *list);


extern RET_STATUS BM_flushFilledBuffer(BM_FILLEDLIST *list);
extern RET_STATUS BM_registFilledCallback(BM_FILLEDLIST *list, BM_FILLEDCALLBACK function, int para1, int para2, BM_CallbackType_et type);


/* Node APIs */
extern unsigned int BM_getReferenceCount(BM_NODE *node);
extern RET_STATUS BM_setReferenceCount(BM_NODE *node, unsigned int count);
extern VOID * BM_getBaseAddr(BM_NODE *node);
extern RET_STATUS BM_setLastNode (BM_NODE *node);
#if _API_CAP_GLB_BM_SUPPORT_TRICK_LAST
extern RET_STATUS BM_setLastTrickNode (BM_NODE *node);
#endif
#if _API_CAP_GLB_DIVX_SLOW_REV
extern RET_STATUS BM_setLastChunkNode (BM_NODE *node);
#endif
extern UINT BM_getFreeBufferCount (BM_FREEPOOL *pool);
extern UINT BM_getFilledBufferCount (BM_FILLEDLIST *list);

/* 
	All Global Buffer Manager variables will be defined below header file
		-- hspark 050119
*/


/* <<< @: 5010-xxx.HoJune_Byun 20060519  : MPVD Bug Avoid a certain address */
RET_STATUS BM_InitializeVBVFreePool 	(BM_FREEPOOL *pool, char *start, 
					unsigned int size, unsigned int count, BOOL initExInfo, VOID *exStart, UINT exSize);
/* <<< @: 5010-xxx.HoJune_Byun 20060519  : MPVD Bug Avoid a certain address */

#endif
