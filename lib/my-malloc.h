/*
 * $Id: my-malloc.h,v 1.6 2002/01/22 23:29:15 tatuo-y Exp $
 *
 * memory leak check
 *
 */

#ifndef _MY_MALLOC_H_
#define _MY_MALLOC_H_

#ifdef __cplusplus
extern "C" {
#endif

#define NEW_GARBAGE 0xCC
#define OLD_GARBAGE 0xDD

#define sa_malloc(X) sa_malloc_func_ptr(X, __FILE__, __LINE__)
#define sa_calloc(X,Y) sa_calloc_func_ptr(X, Y, __FILE__, __LINE__)
#define sa_free(X) sa_free_func_ptr(X, __FILE__, __LINE__)

extern void * (*sa_malloc_func_ptr)();
extern void * (*sa_calloc_func_ptr)();
extern void (*sa_free_func_ptr)();

extern int sa_set_memory_debug_mode(int mode);
extern int sa_memory_leak_check(void);

#ifdef __cplusplus
}
#endif

#endif /* _MY_MALLOC_H_ */

