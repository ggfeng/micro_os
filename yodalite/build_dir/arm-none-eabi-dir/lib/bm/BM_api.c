/*
*	BM_LOCAL.C
*
*	local functions for Buffer Manager list control
*
*	coded by K. H. Yoon
*	2004. 3. 29
*
*/
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

BM_LIST		link_list;
BM_LIST		node_list;
BM_LINK		bm_link[BM_MAX_LINK];
BM_NODE		bm_node[BM_MAX_NODE];

RET_STATUS BML_initialize(VOID)
{
	UINT	i;
	RET_STATUS	status;
	BM_LINK *pTempLink;

	BML_initializeList(&link_list);
	BML_initializeList(&node_list);

	for (i=0; i<BM_MAX_LINK; i++) {
		if ((status = BML_putLink(&link_list, &(bm_link[i]))) != BM_NOERROR)
			return status;
	}

	for (i=0; i < BM_MAX_NODE; i++) {
		if ((pTempLink = BML_getLink(&link_list)) == NULL) {
			BM_LOG("BML_getLink fail\n");
			return BM_FREE_LINK_EMPTY;
		}
		pTempLink->node = &(bm_node[i]);

		if (( status = BML_putLink(&node_list, pTempLink)) != BM_NOERROR)
			return status;
	}

	return BM_NOERROR;
}
#if _BM_ADD_FREEPOOL
/* <<< @: 5010-xxx.HoJune_Byun 20060519  : MPVD Bug : Avoid a certain address */
RET_STATUS _BML_addFreeNode(BM_FREEPOOL *pool, char *start,
				unsigned int size, unsigned int count, BOOL initExInfo, char *exStart, UINT exSize, BOOL skip_2m_boundary)
{
	BM_LINK * pTempLink;
	BM_NODE * pTempNode;
	RET_STATUS status;
	int i;
	char *current, *currentEx;

	CHAR *Start_Addr;
	CHAR *End_Addr;
	INT	NodeCnt_ommitted = 0;
	BOOL	BML_puLinkFlag = TRUE;

	current = start;
	currentEx = exStart;

	for (i=0; i<count; i++)
	{
		if ((pTempLink = BML_getLink(&node_list)) == NULL)
		{
			BM_LOG("BML_getLink fail\n");
			return BM_FREE_NODE_EMPTY;
		}
		pTempNode = pTempLink->node;

		pTempNode->where = pool;
		pTempNode->status = STATUS_FREE;
		pTempNode->refCount = 0;
		pTempNode->bAddress = current;

		Start_Addr	=	current;
		current = (char *) ( current + size );
		End_Addr = current;

		if (initExInfo == TRUE)
		{
			pTempNode->extraInfo = currentEx;
			currentEx = (char *) ( currentEx + exSize );
		}
		else
			pTempNode->extraInfo = NULL;

		if ( skip_2m_boundary )
		{
			/*	This is how MPVD set 'end address' of a node */
			if( (UINT)End_Addr == (((UINT)Start_Addr & 0xFFE00000) | ((UINT)End_Addr & 0x001FFFFF)))
			{
				if ((status = BML_putLink(&(pool->list), pTempLink)) != BM_NOERROR)
					return status;
			}
			else
			{
				pTempNode->where	=	(BM_FREEPOOL *)NULL;
				pTempNode->status	=	STATUS_FREE;
				pTempNode->refCount	=	0;
				pTempNode->bAddress	=	(VOID *)NULL;
				pTempNode->extraInfo	=	(VOID *)NULL;

				NodeCnt_ommitted++;
				BML_puLinkFlag = FALSE;
			}
		}
		else
		{
			if ((status = BML_putLink(&(pool->list), pTempLink)) != BM_NOERROR)
				return status;
		}
	}

	pool->bCount += (count - NodeCnt_ommitted);
	pool->bTotalCount += (count - NodeCnt_ommitted);

	return BM_NOERROR;
}

RET_STATUS BML_initializeFreePool 	(BM_FREEPOOL *pool, char *start, unsigned int size, unsigned int count, BOOL initExInfo,
									char *exStart, UINT exSize, BOOL skip_2m_boundary)
{
	RET_STATUS status;

	if ((status = BML_initializeList(&(pool->list))) != BM_NOERROR) return status;

	pool->valid = STATUS_VALID;
	pool->bSize = size;
	pool->bCount = 0;
	pool->bTotalCount = 0;
	pool->start = (VOID *)start;

	return _BML_addFreeNode(pool, start, size, count, initExInfo, exStart, exSize, skip_2m_boundary);
}

RET_STATUS BML_addFreePool 	(BM_FREEPOOL *pool, char *start, unsigned int size, unsigned int count, BOOL initExInfo,
									char *exStart, UINT exSize, BOOL skip_2m_boundary)
{
	return _BML_addFreeNode(pool, start, size, count, initExInfo, exStart, exSize, skip_2m_boundary);
}
#else
RET_STATUS BML_initializeFreePool 	(BM_FREEPOOL *pool, char *start, unsigned int size, unsigned int count, BOOL initExInfo,
									char *exStart, UINT exSize)
{
	BM_LINK * pTempLink;
	BM_NODE * pTempNode;
	RET_STATUS status;
	int i;
	char *current, *currentEx;

	if ((status = BML_initializeList(&(pool->list))) != BM_NOERROR) return status;

	pool->valid = STATUS_VALID;
	pool->bSize = size;
	pool->bCount = count;
	pool->bTotalCount = count;
	pool->start = (VOID *)start;

	current = start;
	currentEx = exStart;

	for (i=0; i<count; i++) {
		if ((pTempLink = BML_getLink(&node_list)) == NULL) {
			BM_LOG("BML_getLink fail\n");
			return BM_FREE_NODE_EMPTY;
		}
		pTempNode = pTempLink->node;

		pTempNode->where = pool;
		pTempNode->status = STATUS_FREE;
		pTempNode->refCount = 0;
		pTempNode->bAddress = current;
		current = (char *) ( current + size );

		if (initExInfo == TRUE)
		{
			pTempNode->extraInfo = currentEx;
			currentEx = (char *) ( currentEx + exSize );
		}
		else
			pTempNode->extraInfo = NULL;

		if ((status = BML_putLink(&(pool->list), pTempLink)) != BM_NOERROR)
			return status;
	}

	return BM_NOERROR;
}
#endif
RET_STATUS BML_deinitializeFreePool(BM_FREEPOOL *pool)
{
	UINT	i;
	BM_LINK * pTempLink;

	if (pool == NULL) return BM_ILLEGAL_POOL_PT;
	if (pool->bCount != pool->bTotalCount) return BM_ILLEGAL_COUNT;

	for (i=0; i < pool->bTotalCount; i++) {
		pTempLink = BML_getLink(&(pool->list));
		if (pTempLink != NULL) {
			BML_putLink(&node_list, pTempLink);
		}
	}

	pool->valid = STATUS_INVALID;
	pool->bSize = 0;
	pool->bCount = 0;
	pool->bTotalCount = 0;
	pool->start = (VOID *)NULL;

	return BM_NOERROR;
}

RET_STATUS BML_initializeList(BM_LIST *list)
{
	if (list == NULL) return BM_ILLEGAL_LINK_LIST_PT;

	list->head = NULL;
	list->tail = NULL;
	list->count = 0;

	return BM_NOERROR;
}

RET_STATUS	BML_putLink(BM_LIST *list, BM_LINK *link)
{
	if (list == NULL) return BM_ILLEGAL_LINK_LIST_PT;
	if (link == NULL) return BM_ILLEGAL_LINK_PT;
	link->next = NULL;

	if (list->head == NULL){	// list is empty

		list->head = link;
		list->tail = link;
	}
	else {	// else
		list->tail->next = link;
		list->tail = link;
	}

	list->count++;

	return BM_NOERROR;
}

BM_LINK * BML_getLink(BM_LIST *list)
{
	BM_LINK *pLink;

	if (list == NULL) return NULL;

	if (list->head == NULL) { // list is empty
		pLink = NULL;
	}
	else if (list->head == list->tail) { // just one element in list
		pLink = list->head;

		list->head = NULL;
		list->tail = NULL;
	}
	else { // more than two element in list
		pLink = list->head;

		list->head = pLink->next;
	}

	if (pLink != NULL) list->count--;

	return pLink;
}

#if _BM_ADD_FREEPOOL
#else
/* <<< @: 5010-xxx.HoJune_Byun 20060519  : MPVD Bug : Avoid a certain address */
RET_STATUS BML_InitializeVBVFreePool(BM_FREEPOOL *pool, char *start,
				unsigned int size, unsigned int count, BOOL initExInfo, char *exStart, UINT exSize)
{
	BM_LINK * pTempLink;
	BM_NODE * pTempNode;
	RET_STATUS status;
	int i;
	char *current, *currentEx;

	CHAR *Start_Addr;
	CHAR *End_Addr;
	int	NodeCnt_ommitted = 0;
	BOOL	BML_puLinkFlag = TRUE;

	if ((status = BML_initializeList(&(pool->list))) != BM_NOERROR) return status;

	pool->valid = STATUS_VALID;
	pool->bSize = size;
	pool->bCount = count;
	pool->bTotalCount = count;
	pool->start = (VOID *)start;

	current = start;
	currentEx = exStart;

	for (i=0; i<count; i++)
	{
		if ((pTempLink = BML_getLink(&node_list)) == NULL)
		{
			BM_LOG("BML_getLink fail\n");
			return BM_FREE_NODE_EMPTY;
		}
		pTempNode = pTempLink->node;

		pTempNode->where = pool;
		pTempNode->status = STATUS_FREE;
		pTempNode->refCount = 0;
		pTempNode->bAddress = current;

		Start_Addr	=	current;
		current = (char *) ( current + size );
		End_Addr = current;

		if (initExInfo == TRUE)
		{
			pTempNode->extraInfo = currentEx;
			currentEx = (char *) ( currentEx + exSize );
		}
		else
			pTempNode->extraInfo = NULL;

		/*	This is how MPVD set 'end address' of a node */
		if( (UINT)End_Addr == (((UINT)Start_Addr & 0xFFE00000) | ((UINT)End_Addr & 0x001FFFFF)))
		{
			if ((status = BML_putLink(&(pool->list), pTempLink)) != BM_NOERROR)
				return status;
		}
		else
		{
			pTempNode->where	=	(BM_FREEPOOL *)NULL;
			pTempNode->status	=	STATUS_FREE;
			pTempNode->refCount	=	0;
			pTempNode->bAddress	=	(VOID *)NULL;
			pTempNode->extraInfo	=	(VOID *)NULL;

			NodeCnt_ommitted++;
			BML_puLinkFlag = FALSE;
		}
	}

	if(NodeCnt_ommitted)
	{
		pool->bCount = count - NodeCnt_ommitted;
		pool->bTotalCount = count - NodeCnt_ommitted;
	}

	return BM_NOERROR;
}
/* <<< @: 5010-xxx.HoJune_Byun 20060519  : MPVD Bug Avoid a certain address */
#endif
