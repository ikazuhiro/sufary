/*
 * $Id: mmp.h,v 1.3 2002/01/22 23:29:15 tatuo-y Exp $
 *
 *  mmp.h --- memory mapped file wrapper
 *
 */

#ifndef _MMP_H_
#define _MMP_H_


#include <sys/types.h>

#ifndef dp
#include <stdio.h>
#define dp(x) printf(#x " = %Lg\n", (long double)(x))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mmap_open_mode_ {
    SA_MMAP_RO,
    SA_MMAP_RW
} SA_MMAP_MODE;


typedef struct {
    off_t size;
    void *map;
    void *other;
} SA_MMAP;


extern SA_MMAP * sa_open_mmap(const char *fname, const SA_MMAP_MODE mode);
extern void sa_close_mmap(SA_MMAP *mmr);

extern off_t sa_get_mmap_size(const SA_MMAP *mmr);

extern void * sa_get_mmap_ptr(const SA_MMAP *mmr);


#ifdef __cplusplus
}
#endif

#endif				/* _MMP_H_ */
