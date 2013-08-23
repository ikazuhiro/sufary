/*
 * $Id: util.h,v 1.13 2002/01/22 23:29:15 tatuo-y Exp $
 */

#ifndef _SA_UTIL_H_
#define _SA_UTIL_H_

#include <stdarg.h>
#include "mmp.h"
#include "my-malloc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MSB_ON(CHAR) (0x80 & (unsigned char)(CHAR))


extern void sa_reverse_byte_order(void *p, int size);
extern void sa_reverse_byte_order_all(void* p, int size, size_t num);

extern char *sa_add_suffix_to_file_name(char *new_fname, const char *fname,
					const char *suffix);

extern int is_debug_mode(void);
extern void set_debug_mode(int);
extern void sa_dprintf(char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif				/* _SA_UTIL_H_ */

