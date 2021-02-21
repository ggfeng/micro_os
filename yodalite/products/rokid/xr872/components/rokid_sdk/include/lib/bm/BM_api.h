#ifndef _BM_LOCAL_H_
#define _BM_LOCAL_H_

#define	BM_MAX_LINK		300
#define	BM_MAX_NODE		250

extern BM_LIST		link_list;
extern BM_LIST		node_list;
extern BM_LINK		bm_link[BM_MAX_LINK];
extern BM_NODE		bm_node[BM_MAX_NODE];

extern RET_STATUS BML_initialize(VOID);
#if _BM_ADD_FREEPOOL
RET_STATUS BML_initializeFreePool 	(BM_FREEPOOL *pool, char *start, unsigned int size, unsigned int count, BOOL initExInfo, 
									char *exStart, UINT exSize, BOOL skip_2m_boundary);
RET_STATUS BML_addFreePool 	(BM_FREEPOOL *pool, char *start, unsigned int size, unsigned int count, BOOL initExInfo, 
									char *exStart, UINT exSize, BOOL skip_2m_boundary);
#else
extern RET_STATUS BML_initializeFreePool 	(BM_FREEPOOL *pool, char *start, unsigned int size, unsigned int count, BOOL initExInfo, char *exStart, UINT exSize);
#endif
extern RET_STATUS BML_deinitializeFreePool(BM_FREEPOOL *pool);
extern RET_STATUS BML_initializeList(BM_LIST *list);
extern RET_STATUS BML_putLink(BM_LIST *list, BM_LINK *link);
extern BM_LINK * BML_getLink(BM_LIST *list);

//int BML_initializeLinkMemory(VOID);


/* <<< @: 5010-xxx.HoJune_Byun 20060519  : MPVD Bug Avoid a certain address */
extern RET_STATUS BML_InitializeVBVFreePool(BM_FREEPOOL *pool, char *start, 
				unsigned int size, unsigned int count, BOOL initExInfo, char *exStart, UINT exSize);
/* <<< @: 5010-xxx.HoJune_Byun 20060519  : MPVD Bug Avoid a certain address */
#endif
