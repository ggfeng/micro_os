/*****************************************************************************************
*	BM.C
*
*	Buffer Manager API functions
*
*	coded by K. H. Yoon
*	2004. 3. 29
*
******************************************************************************************/
#include <yodalite_autoconf.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <hardware/platform.h>

#include "Basic_typedefs.h"

#include <lib/bm/SH_BM_api.h>
#include <lib/bm/BM_api.h>

BM_FREEPOOL gBmMsgPool;
VOID 	*msgAddr;
static int bm_init = 0;

/*****************************************************************************************
*
*	FUNCTION NAME : BM_initialize
*
*	initialize free buffer pool for Msg queue
*
*	API Function
*
*	input argument : 	None
*
*	return value : RET_STATUS : error condition or no error
*
******************************************************************************************/
RET_STATUS BM_initialize (VOID)
{
	RET_STATUS status;

    if(bm_init)
        return BM_NOERROR;

	#ifndef _BM_TEST_
	msgAddr = BM_MALLOC( 16 * 50 );
	memset(msgAddr, 0, 16 * 50 );

	#else
	msgAddr = 0x1500000;
	#endif

	if (msgAddr == NULL)
		return BM_ILLEGAL_POOL_PT;

	if ((status = BML_initialize()) != BM_NOERROR)
		return status;
#if _BM_ADD_FREEPOOL
	if ((status = BML_initializeFreePool( &gBmMsgPool, (char *)msgAddr, 16, 50, FALSE, NULL, 0, FALSE)) != BM_NOERROR)
		return status;
#else
	// free buffer pool init local function
	if ((status = BML_initializeFreePool( &gBmMsgPool, (char *)msgAddr, 16, 50, FALSE, NULL, 0)) != BM_NOERROR)
		return status;
#endif
    bm_init = 1;
	return BM_NOERROR;
}


/*****************************************************************************************
*
*	FUNCTION NAME : BM_initializeFreePool
*
*	initialize free buffer pool
*
*	API Function
*
*	input argument : 	BM_FREEPOOL *pool : free buffer pool control block for initialization
*					char *start : buffer start address
*					int size  : one buffer size
*
*	return value : RET_STATUS : error condition or no error
*
******************************************************************************************/
RET_STATUS BM_initializeFreePool 	(BM_FREEPOOL *pool, char *start, unsigned int size, unsigned int count, BOOL initExInfo, VOID *exStart, UINT exSize)
{
	RET_STATUS status;
	//SysInterruptLevel_t old_level;

	// check error case
	if (pool == NULL) 	return BM_ILLEGAL_POOL_PT; // illegal free pool pointer
	if (pool->valid == STATUS_VALID) return BM_ILLEGAL_POOL_PT;
	if (start == NULL ) return BM_ILLEGAL_START; // illegal start point
	if (size == 0 || (size % 4) != 0 ) return BM_ILLEGAL_SIZE; // word align
	if (count == 0 ) return BM_ILLEGAL_COUNT; // illegal buffer count

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

	pool->msg_filled.valid = STATUS_INVALID;
#if _BM_ADD_FREEPOOL
	if ((status = BML_initializeFreePool(pool, start, size, count, initExInfo, (char*)exStart, exSize, FALSE)) != BM_NOERROR){
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return status;
	}
#else
	// free buffer pool init local function
	if ((status = BML_initializeFreePool(pool, start, size, count, initExInfo, (char*)exStart, exSize)) != BM_NOERROR){
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return status;
	}
#endif
	// msg queue init
	pool->msg_free = (struct bm_freepool *)(&gBmMsgPool);

	if ((status = BM_initializeFilledList(&(pool->msg_filled))) != BM_NOERROR) {
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return status;
	}

	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return BM_NOERROR;
}


/* <<< @: 5010-xxx.HoJune_Byun 20060519  : MPVD Bug Avoid a certain address */
RET_STATUS BM_InitializeVBVFreePool 	(BM_FREEPOOL *pool, char *start,
					unsigned int size, unsigned int count, BOOL initExInfo, VOID *exStart, UINT exSize)
{
	RET_STATUS status;
	//SysInterruptLevel_t old_level;
#if _BM_ADD_FREEPOOL
	BOOL skip_2m_boundary;
#endif

	// check error case
	if (pool == NULL) 	return BM_ILLEGAL_POOL_PT; // illegal free pool pointer
	if (pool->valid == STATUS_VALID) return BM_ILLEGAL_POOL_PT;
	if (start == NULL ) return BM_ILLEGAL_START; // illegal start point
	if (size == 0 || (size % 4) != 0 ) return BM_ILLEGAL_SIZE; // word align
	if (count == 0 ) return BM_ILLEGAL_COUNT; // illegal buffer count

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

	pool->msg_filled.valid = STATUS_INVALID;
#if _BM_ADD_FREEPOOL
	#if _API_CAP_GLB_MPVD_FRAME_MODE
	skip_2m_boundary = TRUE;
	#else
	skip_2m_boundary = FALSE;
	#endif

	if ((status = BML_initializeFreePool(pool, start, size, count, initExInfo, (char*)exStart, exSize, skip_2m_boundary)) != BM_NOERROR){
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return status;
	}
#else
	// free buffer pool init local function
	if ((status = BML_InitializeVBVFreePool(pool, start, size, count, initExInfo, (char*)exStart, exSize)) != BM_NOERROR){
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return status;
	}
#endif
	// msg queue init
	pool->msg_free = (struct bm_freepool *)(&gBmMsgPool);

	if ((status = BM_initializeFilledList(&(pool->msg_filled))) != BM_NOERROR) {
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return status;
	}

	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return BM_NOERROR;
}
/* <<< @: 5010-xxx.HoJune_Byun 20060519  : MPVD Bug Avoid a certain address */


#if _BM_ADD_FREEPOOL
RET_STATUS BM_addFreePool 	(BM_FREEPOOL *pool, char *start, unsigned int size, unsigned int count, BOOL initExInfo, VOID *exStart, UINT exSize)
{
	RET_STATUS status;
	//SysInterruptLevel_t old_level;

	// check error case
	if (pool == NULL) 	return BM_ILLEGAL_POOL_PT; // illegal free pool pointer

	/* Pool should be valid because this should be called after BM_InitializeVBVFreePool */
	if (pool->valid == STATUS_INVALID) return BM_ILLEGAL_POOL_PT;

	if (start == NULL ) return BM_ILLEGAL_START; // illegal start point
	if (size == 0 || (size % 4) != 0 ) return BM_ILLEGAL_SIZE; // word align
	if (count == 0 ) return BM_ILLEGAL_COUNT; // illegal buffer count

	/* size should be same as the previous size */
	if ( pool->bSize != size ) return BM_ILLEGAL_SIZE;

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

	//pool->msg_filled.valid = STATUS_INVALID; //==> This should be deleted because it will cause freecallback not occur when buffer full ck.lee

	// free buffer pool init local function
	if ((status = BML_addFreePool(pool, start, size, count, initExInfo, (char*)exStart, exSize, FALSE)) != BM_NOERROR){
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return status;
	}

	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return BM_NOERROR;
}

RET_STATUS BM_addVBVFreePool 	(BM_FREEPOOL *pool, char *start,
					unsigned int size, unsigned int count, BOOL initExInfo, VOID *exStart, UINT exSize)
{
	RET_STATUS status;
	//SysInterruptLevel_t old_level;
	BOOL skip_2m_boundary;

	// check error case
	if (pool == NULL) 	return BM_ILLEGAL_POOL_PT; // illegal free pool pointer

	/* Pool should be valid because this should be called after BM_InitializeVBVFreePool */
	if (pool->valid == STATUS_INVALID) return BM_ILLEGAL_POOL_PT;

	if (start == NULL ) return BM_ILLEGAL_START; // illegal start point
	if (size == 0 || (size % 4) != 0 ) return BM_ILLEGAL_SIZE; // word align
	if (count == 0 ) return BM_ILLEGAL_COUNT; // illegal buffer count

	/* size should be same as the previous size */
	if ( pool->bSize != size ) return BM_ILLEGAL_SIZE;

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

#if _API_CAP_GLB_MPVD_FRAME_MODE
	skip_2m_boundary = TRUE;
#else
	skip_2m_boundary = FALSE;
#endif

	// free buffer pool init local function
	if ((status = BML_addFreePool(pool, start, size, count, initExInfo, (char*)exStart, exSize, skip_2m_boundary)) != BM_NOERROR){
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return status;
	}

	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return BM_NOERROR;
}
#endif

RET_STATUS BM_deinitializeFreePool(BM_FREEPOOL *pool)
{
	RET_STATUS status;
	//SysInterruptLevel_t old_level;

	// check error case
	if (pool == NULL) 	return BM_ILLEGAL_POOL_PT; // illegal free pool pointer 
	if (pool->valid == STATUS_INVALID) return BM_ILLEGAL_POOL_PT; // illegal free pool pointer
	if (pool->bTotalCount != pool->bCount) return BM_ILLEGAL_COUNT;

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

	status = BM_flushFilledBuffer(/*pool->msg_free, */&(pool->msg_filled));
	BM_deinitializeFilledList(&(pool->msg_filled));

	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return BML_deinitializeFreePool(pool);
}

/*****************************************************************************************
*
*	FUNCTION NAME : BM_requestFreeBuffer
*
*	request a free buffer
*
*	API Function
*
*	input argument : BM_FREEPOOL *pool : free buffer pool control block
*
*	return value : BM_NODE * : free buffer pointer or NULL<ERROR CONDITION>
*
******************************************************************************************/
//RET_STATUS BM_registFreeCallback(BM_FREEPOOL *pool, BM_FREECALLBACK function, int para1, int para2)
BM_NODE * BM_requestFreeBuffer(BM_FREEPOOL *pool, BM_FREECALLBACK function, int para1, int para2)
{
	BM_LINK * pTempLink;
	BM_NODE *pTempNode;
	BM_NODE *				callbackMsg;
	unsigned int *				addr;
	RET_STATUS		status;
	//SysInterruptLevel_t old_level;
//	BM_LINK *pTemp;

	// check error case
	if (pool == NULL) 	return NULL; // illegal free pool pointer
	if (pool->valid == STATUS_INVALID) return NULL; // illegal free pool pointer

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);
	// get free buffer node
	if ((pTempLink = BML_getLink(&(pool->list))) == NULL) {

		if (function != NULL) {
			callbackMsg = BM_requestFreeBuffer(pool->msg_free, NULL, 0, 0);
			addr = (unsigned int*)BM_getBaseAddr(callbackMsg);

			// regist function
			*addr = (unsigned int)function;
			*(((int *)addr) + 1) = para1;
			*(((int *)addr) + 2) = para2;
			if ((status = BM_putFilledBuffer(&(pool->msg_filled), callbackMsg)) != BM_NOERROR) {
				//SysControlInterrupt(old_level, &old_level, NULL);
				pal_irq_enable(ALL_IRQ);
				return NULL;
			}
			BM_releaseFreeBuffer(callbackMsg);
		}

		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return NULL;
	}
	pTempNode = pTempLink->node;
	pool->bCount--;

	pTempNode->status = STATUS_ALLOC;
	//pTempNode->extraInfo = NULL;
	pTempNode->refCount = 1;

	BML_putLink(&link_list, pTempLink);  // change with free
	//SysFree((VOID *) pTempLink);
	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return pTempNode;
}

/*****************************************************************************************
*
*	FUNCTION NAME : BM_requestFreeBufferOverlap ( added by hspark to allow overlapping of callback )
*
*	request a free buffer & if there is previously registered msg, then skip registering again
*
*	API Function
*
*	input argument : BM_FREEPOOL *pool : free buffer pool control block
*
*	return value : BM_NODE * : free buffer pointer or NULL<ERROR CONDITION>
*
******************************************************************************************/
//RET_STATUS BM_registFreeCallback(BM_FREEPOOL *pool, BM_FREECALLBACK function, int para1, int para2)
BM_NODE * BM_requestFreeBufferOverlap(BM_FREEPOOL *pool, BM_FREECALLBACK function, int para1, int para2)
{
	BM_LINK * pTempLink;
	BM_NODE *pTempNode;
	BM_NODE *				callbackMsg;
	unsigned int *				addr;
	RET_STATUS		status;
	//SysInterruptLevel_t old_level;
//	BM_LINK *pTemp;

	// check error case
	if (pool == NULL) 	return NULL; // illegal free pool pointer 
	if (pool->valid == STATUS_INVALID) return NULL; // illegal free pool pointer

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);
	// get free buffer node
	if ((pTempLink = BML_getLink(&(pool->list))) == NULL) {

		if ((pool->msg_filled.bCount == 0) && function != NULL) {
			/*
				if there is no previous callbackMsg
			*/
			callbackMsg = BM_requestFreeBuffer(pool->msg_free, NULL, 0, 0);
			addr = (unsigned int*)BM_getBaseAddr(callbackMsg);

			// regist function
			*addr = (unsigned int)function;
			*(((int *)addr) + 1) = para1;
			*(((int *)addr) + 2) = para2;
			if ((status = BM_putFilledBuffer(&(pool->msg_filled), callbackMsg)) != BM_NOERROR) {
				//SysControlInterrupt(old_level, &old_level, NULL);
				pal_irq_enable(ALL_IRQ);
				return NULL;
			}
			BM_releaseFreeBuffer(callbackMsg);
		}

		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return NULL;
	}
	pTempNode = pTempLink->node;
	pool->bCount--;

	pTempNode->status = STATUS_ALLOC;
	//pTempNode->extraInfo = NULL;
	pTempNode->refCount = 1;

	BML_putLink(&link_list, pTempLink);  // change with free
	//SysFree((VOID *) pTempLink);
	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return pTempNode;
}

/*****************************************************************************************
*
*	FUNCTION NAME : BM_releaseFreeBuffer
*
*	deallocate a free buffer
*
*	API Function
*
*	input argument : 	BM_FREEPOOL *pool : free buffer pool control block
*					BM_NODE *node : free buffer control block
*
*	return value : RET_STATUS : error condition or no error
*
******************************************************************************************/
RET_STATUS BM_releaseFreeBuffer(BM_NODE *node)
{
	BM_FREEPOOL *				pool;
	BM_LINK *					pTempLink;
	BM_NODE *					pTempNode;
	VOID **						addr;
	int 							para1, para2;
	//SysInterruptLevel_t			old_level;

	if (node == NULL) return BM_ILLEGAL_NODE_PT;
	if (node->status == STATUS_FREE) return BM_ILLEGAL_NODE_PT;

	pool = node->where;

	// check error case
	if (pool == NULL) 	return BM_ILLEGAL_POOL_PT; // illegal free pool pointer
	if (pool->valid == STATUS_INVALID) return BM_ILLEGAL_POOL_PT; // illegal free pool pointer

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);
	// decrement refernce count
	if (node->refCount > 0) node->refCount--;
	if (node->refCount != 0) {
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return BM_NOERROR;
	}

	// put into free buffer list
	//pTempLink = BML_getLink(&link_list);	// change with malloc
	//pTempLink = (BM_LINK *) SysMalloc(sizeof(BM_LINK));
	if ((pTempLink = BML_getLink(&link_list)) == NULL) {
		//ASSERT(0);
		return BM_FREE_LINK_EMPTY;
	}

	pTempLink->node = node;
	node->status = STATUS_FREE;

	BML_putLink(&(pool->list), pTempLink);
	pool->bCount++;
#if _API_CAP_GLB_BAT_MODE
	if (/*pool->bCount == 1 && */(pTempNode = BM_getFilledBuffer(&(pool->msg_filled), NULL, 0, 0, BM_CALL_NULL)) != NULL) {
		// suspend block exist
		addr = (VOID **)BM_getBaseAddr(pTempNode);

		para1 = *(((int *)addr) + 1);
		para2 = *(((int *)addr) + 2);

		/* function call */
		(*((BM_FREECALLBACK)*addr))(para1, para2);

		// msg queue free
		BM_releaseFreeBuffer(pTempNode);
	}
#else
	if (pool->bCount == 1 && (pTempNode = BM_getFilledBuffer(&(pool->msg_filled), NULL, 0, 0, BM_CALL_NULL)) != NULL) {
		// suspend block exist
		addr = (VOID **)BM_getBaseAddr(pTempNode);

		para1 = *(((int *)addr) + 1);
		para2 = *(((int *)addr) + 2);

		/* function call */
		(*((BM_FREECALLBACK)*addr))(para1, para2);

		// msg queue free
		BM_releaseFreeBuffer(pTempNode);
	}
#endif
	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return BM_NOERROR;
}

/* Filled Buffer APIs */
/*****************************************************************************************
*
*	FUNCTION NAME : BM_initializeFilledList
*
*	initialize filled buffer list
*
*	API Function
*
*	input argument : BM_FILLEDLIST *list : Filled Buffer List control block for initialization
*
*	return value : RET_STATUS : error condition or no error
*
******************************************************************************************/
RET_STATUS BM_initializeFilledList (BM_FILLEDLIST *list)
{
	RET_STATUS				status;
	//SysInterruptLevel_t		old_level;

	if (list == NULL) return BM_ILLEGAL_FILLED_LIST_PT;
	if (list->valid == STATUS_VALID) return BM_ILLEGAL_FILLED_LIST_PT;

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

	if ( (status = BML_initializeList(&(list->list))) != BM_NOERROR) {
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return status;
	}

	list->valid = STATUS_VALID;
	list->eBindStatus = STATUS_UNBIND;
	list->bCount = 0;
	list->function = NULL;

	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return BM_NOERROR;
}

RET_STATUS BM_deinitializeFilledList (BM_FILLEDLIST *list)
{
	//SysInterruptLevel_t 		old_level;

	if (list == NULL) return BM_ILLEGAL_FILLED_LIST_PT;
	if (list->valid == STATUS_INVALID) return BM_ILLEGAL_FILLED_LIST_PT;
	if (list->bCount != 0) return BM_ILLEGAL_COUNT;

	//BM_flushFilledBuffer(BM_FREEPOOL * pool, BM_FILLEDLIST * list)

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

	list->valid = STATUS_INVALID;
	list->bCount = 0;
	list->function = NULL;

	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return BM_NOERROR;
}

/*****************************************************************************************
*
*	FUNCTION NAME : BM_putFilledBuffer
*
*	insert filled buffer into filled list
*
*	API Function
*
*	input argument : 	BM_FILLEDLIST *list : Filled Buffer List control block
*					BM_NODE *node : buffer control block to insert
*
*	return value : RET_STATUS : error condition or no error
*
******************************************************************************************/
RET_STATUS BM_putFilledBuffer (BM_FILLEDLIST *list, BM_NODE *node)
{
	RET_STATUS 				status;
	BM_LINK *				pTempLink;
	BM_FILLEDCALLBACK		pTemp;
	UINT					para1, para2;
	//SysInterruptLevel_t 		old_level;

	if (list == NULL) return BM_ILLEGAL_FILLED_LIST_PT;
	if (list->valid == STATUS_INVALID) return BM_ILLEGAL_FILLED_LIST_PT;
	if (node == NULL) return BM_ILLEGAL_NODE_PT;

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

	node->refCount++;

		// change with malloc
	if ((pTempLink = BML_getLink(&link_list)) == NULL) {
		//ASSERT(0);
		return BM_FREE_LINK_EMPTY;
	}
	//pTempLink = (BM_LINK *) SysMalloc (sizeof(BM_LINK));
	pTempLink->node = node;

	if ((status = BML_putLink(&(list->list), pTempLink)) != BM_NOERROR) {
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return status;
	}
	list->bCount++;

	if (/*list->bCount == 1 &&*/ list->function != NULL) {
		/* function call */
		pTemp = list->function;

		if (list->callbackType == BM_CALL_AT_ONCE)
			list->function = (BM_FILLEDCALLBACK)NULL;

		para1 = list->para1;
		para2 = list->para2;

		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);

		(*(pTemp))(para1, para2);

		//return BM_NOERROR;
	}
	else {
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
	}

	return BM_NOERROR;
}

/****************************************************************************************
*
*	FUNCTION NAME : BM_getFilledBuffer
*
*	get filled buffer from filled list
*
*	API Function
*
*	input argument : 	BM_FILLEDLIST *list : Filled Buffer List control block
*
*	return value : BM_NODE * : filled buffer pointer or NULL<ERROR Case>
*
******************************************************************************************/
//RET_STATUS BM_registFilledCallback(BM_FILLEDLIST *list, BM_FILLEDCALLBACK function, int para1, int para2, BM_CallbackType_et type)
BM_NODE * BM_getFilledBuffer(BM_FILLEDLIST *list, BM_FILLEDCALLBACK function, int para1, int para2, BM_CallbackType_et type)
{
	BM_LINK *pTempLink;
	BM_NODE *pTempNode;
	RET_STATUS status;
	//SysInterruptLevel_t old_level;

	if (list == NULL) return NULL;
	if (list->valid == STATUS_INVALID) return NULL;

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

	if ((pTempLink = BML_getLink(&(list->list))) == NULL) {
		if (function != NULL) {
			list->function = function;
			list->para1 = para1;
			list->para2 = para2;
			list->callbackType = type;
		}
		//SysControlInterrupt(old_level, &old_level, NULL);
		pal_irq_enable(ALL_IRQ);
		return NULL;
	}
	list->bCount--;

	pTempNode = pTempLink->node;
	//pTempNode->refCount--;

	if ((status = BML_putLink(&link_list, pTempLink)) != BM_NOERROR) return NULL;	// change with free
	//SysFree ((VOID *) pTempLink);
	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return pTempNode;
}

/* Get the first node without removing from the list */
BM_NODE * BM_peekFilledBuffer(BM_FILLEDLIST *list)
{
	BM_LINK *pTempLink;
	BM_NODE *pTempNode;
	//RET_STATUS status;
	//SysInterruptLevel_t old_level;

	if (list == NULL) return NULL;
	if (list->valid == STATUS_INVALID) return NULL;

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

	pTempLink = list->list.head;
	if ( pTempLink != NULL )
		pTempNode = pTempLink->node;
	else
		pTempNode = NULL;

	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return pTempNode;
}

RET_STATUS BM_flushFilledBuffer(BM_FILLEDLIST *list)
{
	//BM_LINK *pTempLink;
	BM_NODE *pTempNode;
	RET_STATUS status;
	//SysInterruptLevel_t old_level;

	/*for (pTempLink = list->list.head; pTempLink != NULL; pTempLink = pTempLink->next) {
		if ((status = BM_releaseFreeBuffer(pool, pTempLink->node)) != BM_NOERROR) return status;
		//if ((status = BML_putLink(&link_list, pTempLink)) != BM_NOERROR) return status;
		SysFree ((VOID *) pTempLink);
	}*/
	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

	for (pTempNode = BM_getFilledBuffer(list, NULL, 0, 0, BM_CALL_NULL); \
		pTempNode != NULL; pTempNode = BM_getFilledBuffer(list, NULL, 0, 0, BM_CALL_NULL)) {
		if ((status = BM_releaseFreeBuffer(pTempNode)) != BM_NOERROR) {
			//SysControlInterrupt(old_level, &old_level, NULL);
			pal_irq_enable(ALL_IRQ);
			return status;
		}
	}

	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return BM_NOERROR;
}

/*****************************************************************************************
*
*	FUNCTION NAME : BM_registFilledCallback
*
*	regist delayed execution function to execute when a new filled buffer is inserted
*
*	API Function
*
*	input argument : 	BM_FILLEDLIST *list : filled buffer list control block
*					BM_FREECALLBACK function : function pointer
*					int para1 : function paramenter 1
*					int para2 : function paramenter 2
*
*	return value : RET_STATUS : error condition or no error
*
******************************************************************************************/
RET_STATUS BM_registFilledCallback(BM_FILLEDLIST *list, BM_FILLEDCALLBACK function, int para1, int para2, BM_CallbackType_et type)
{
	//SysInterruptLevel_t old_level;

	if (list == NULL) 	return BM_ILLEGAL_POOL_PT; // illegal free pool pointer
	if (list->valid == STATUS_INVALID) return BM_ILLEGAL_FILLED_LIST_PT;
	if (function == NULL) return BM_ILLEGAL_CALLBACK;

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

	list->function = function;
	list->para1 = para1;
	list->para2 = para2;
	list->callbackType = type;

	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return BM_NOERROR;
}

/* Node APIs */
unsigned int BM_getReferenceCount(BM_NODE *node)
{
	if (node == NULL) return -1;
	return node->refCount;
}

RET_STATUS BM_setReferenceCount(BM_NODE *node, unsigned int count)
{
	//SysInterruptLevel_t old_level;

	if (node == NULL) return BM_ILLEGAL_NODE_PT;

	//SysControlInterrupt(INTERRUPT_DISABLE, &old_level, NULL);
	pal_irq_disable(ALL_IRQ);

	node->refCount = count;

	//SysControlInterrupt(old_level, &old_level, NULL);
	pal_irq_enable(ALL_IRQ);

	return BM_NOERROR;
}

VOID * BM_getBaseAddr(BM_NODE *node)
{
	if (node == NULL) return NULL;
	return node->bAddress;
}

RET_STATUS BM_setLastNode (BM_NODE *node)
{
	if (node == NULL) return BM_ILLEGAL_NODE_PT;
	node->status = STATUS_LAST_ALLOC;

	return BM_NOERROR;
}

#if _API_CAP_GLB_BM_SUPPORT_TRICK_LAST
RET_STATUS BM_setLastTrickNode (BM_NODE *node)
{
	if (node == NULL) return BM_ILLEGAL_NODE_PT;

	if (node->status != STATUS_LAST_ALLOC)
	{
		/* 
			if it isn't LAST_ALLOC, then write
		*/
		node->status = STATUS_LAST_TRICK_ALLOC;
	}

	return BM_NOERROR;
}
#endif

#if _API_CAP_GLB_DIVX_SLOW_REV
RET_STATUS BM_setLastChunkNode (BM_NODE *node)
{
	if (node == NULL) return BM_ILLEGAL_NODE_PT;

	if (node->status != STATUS_LAST_ALLOC)
	{
		/* 
			if it isn't LAST_ALLOC, then write
		*/
		node->status = STATUS_LAST_CHUNK_ALLOC;
	}

	return BM_NOERROR;
}
#endif

UINT BM_getFreeBufferCount (BM_FREEPOOL *pool)
{
	if (pool == NULL) return BM_ILLEGAL_POOL_PT;

	return pool->bCount;
}

UINT BM_getFilledBufferCount (BM_FILLEDLIST *list)
{
	if (list == NULL) return BM_ILLEGAL_FILLED_LIST_PT;

	return list->bCount;
}

