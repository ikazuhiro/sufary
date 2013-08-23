/*
 * $Id: did.c,v 1.4 2000/11/17 02:51:50 tatuo-y Exp $
 *
 * extract region with Docment Id
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sufary.h"
#include "util.h"
#include "config.h"
#include "did.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

/*
 *   領域検索
 *
 * description
 *   2分探索により、目的インデックス未満の最大数を探す。
 *
 * (例) S はドキュメントの開始、Eは終了位置を表す。
 *      target=24 ならば 20〜56 が目的ドキュメント
 *        → start = 20, size=36  
 *      target=67 はドキュメント外。
 *        → start = -1, size=-1  
 *
 * DocID |  0 | 12 | 20 | 56 | 89 | 97 |
 *          S    E    S    E    S    E 
 *
 */
SA_INDEX
sa_dididx2txtidx(const DID *did, SA_INDEX idx)
{
    assert(did != NULL);
    assert(idx >= 0);
    {
#ifdef WORDS_BIGENDIAN
	return sa_get_did_ptr(did)[idx];
#else
	SA_INDEX rv = sa_get_did_ptr(did)[idx];
	sa_reverse_byte_order(&rv, sizeof(SA_INDEX));
	return rv;
#endif
    }
}

DID_RESULT
sa_didsearch(const DID *did, SA_INDEX target)
{
    SA_INDEX size;
    SA_INDEX cur;
    SA_INDEX li = 0;
    SA_INDEX ri;
    SA_INDEX region_from = 0;
    SA_INDEX region_to = 0;
    assert(did != NULL);
    assert(0 <= target);

    size = ri = sa_get_did_size(did);

    sa_dprintf("did-search: target = %ld\n", target);
    while (li < ri) {
	cur = li / 2 + ri / 2 + (1 & li & ri);

	sa_dprintf("did-search: l=[%ld] c=[%ld] r=[%ld]\n", li, cur, ri);
	region_from = sa_dididx2txtidx(did, cur * 2);
	region_to   = sa_dididx2txtidx(did, cur * 2 + 1);
	if (region_to <= target) {
	    li = cur + 1;
	} else if (target < region_from) {
	    ri = cur;
	} else
	    break;		/* the target is in this region */
    }
    
    {
	DID_RESULT dr;

	if (region_from <= target && target < region_to) {
	    dr.stat = SUCCESS;
	    dr.no = cur;
	    dr.start = region_from;
	    dr.size = region_to - region_from;
	} else {
	    dr.stat = FAIL;
	    dr.no = cur + 1;
	    if (target < region_from)
		dr.no = 0;
	    dr.start = 0;
	    dr.size = 0;
	}

	sa_dprintf("did-search: *** %s [no=%ld start=%ld size=%ld]\n",
		   dr.stat == FAIL ? "NOT-FOUND" : "FOUND",
		   dr.no, dr.start, dr.size);

	return dr;
    }
}

/*
 * access functions for DID
 */
SA_INDEX
sa_get_start_position(const DID *did, SA_INDEX id)
{
    assert(did != NULL);
    return sa_dididx2txtidx(did, id * 2);
}

SA_INDEX
sa_get_end_position(const DID *did, SA_INDEX id)
{
    assert(did != NULL);
    return sa_dididx2txtidx(did, id * 2 + 1);
}


SA_INDEX *
sa_get_did_ptr(const DID *did)
{
    assert(did != NULL);
    return did->docid;
}

SA_INDEX
sa_get_did_size(const DID *did)
{
    assert(did != NULL);
    /* number of documents, not number of indexes */
    return did->didsize / 2;
}


/*
 * open DocID file
 */
DID *
sa_open_did(const char *filename)
{
    DID *newdid;
    SA_MMAP *mms;
    assert(filename != NULL);

    newdid = (DID *)sa_calloc(sizeof(DID), 1);
    if (newdid == NULL){
	sa_error_no = MEMORY_ALLOCATE_ERROR;
	return NULL;
    }

    mms = sa_open_mmap(filename, SA_MMAP_RO);
    if (mms == NULL) {
	sa_error_no = MMAP_ERROR;
	return NULL;
    }

    newdid->mmdid = mms;

    newdid->docid = (SA_INDEX*)sa_get_mmap_ptr(mms);
    newdid->didsize = sa_get_mmap_size(mms) / sizeof(SA_INDEX);
    return newdid;
}


/*
 * close DocID file
 */
void
sa_close_did(DID *did)
{
    assert(did != NULL);
    sa_close_mmap(did->mmdid);
    sa_free(did);
}
