/*
 * $Id: did.h,v 1.2 2002/01/22 23:29:15 tatuo-y Exp $
 *
 *  did.h --- region index controller
 *
 */

#ifndef _DID_H_
#define _DID_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
#define SA_FILE_NAME_MAX 1000

typedef long SA_INDEX;

typedef enum sa_stat_ {
    SUCCESS,
    FAIL
} SA_STAT;
*/

typedef struct {
    void *mmdid;		/* SA_MMAP *mmdid; */
    SA_INDEX *docid;		/* mmapされた配列 */
    SA_INDEX didsize;		/* DID 配列の大きさ */
} DID;

typedef struct {
    SA_INDEX start;		/* ドキュメント開始位置 */
    SA_INDEX size;		/* ドキュメントサイズ */
    SA_INDEX no;		/* ドキュメント番号 */
    SA_STAT stat;
} DID_RESULT;


/* in did.c */
extern DID_RESULT sa_didsearch(const DID *did, SA_INDEX target);
extern SA_INDEX sa_get_start_position(const DID *did, SA_INDEX id);
extern SA_INDEX sa_get_end_position(const DID *did, SA_INDEX id);
extern SA_INDEX sa_dididx2txtidx(const DID *did, SA_INDEX idx);

extern DID *sa_open_did(const char *fn);
extern void sa_close_did(DID *did);

extern SA_INDEX *sa_get_did_ptr(const DID *did);
extern SA_INDEX sa_get_did_size(const DID *did);


/* in make-did.c */
extern SA_STAT sa_make_did(const char *txt_fname, char *ary_fname,
			   char *did_fname,
			   const char *tag1, const char *tag2);

#ifdef __cplusplus
}
#endif

#endif				/* _DID_H_ */
