/*
 * $Id: my-malloc.c,v 1.6 2000/11/17 02:51:50 tatuo-y Exp $
 *
 * memory leak check
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "my-malloc.h"

#define MALLOC_LIST_FILE_NAME_MAX 20

/*
void * (*sa_malloc_func_ptr)() = malloc;
void * (*sa_calloc_func_ptr)() = calloc;
void (*sa_free_func_ptr)() = free;
*/


static void *sa_malloc_aux(size_t size, char *file, int line);
static void *sa_calloc_aux(size_t nmemb, size_t size, char *file, int line);
static void sa_free_aux(void *ptr, char *file, int line);

void * (*sa_malloc_func_ptr)() = sa_malloc_aux;
void * (*sa_calloc_func_ptr)() = sa_calloc_aux;
void (*sa_free_func_ptr)() = sa_free_aux;


typedef struct SA_MALLOC_LIST SA_MALLOC_LIST;
struct SA_MALLOC_LIST {
    void *ptr;
    SA_MALLOC_LIST *next;
    size_t size;
    char file[MALLOC_LIST_FILE_NAME_MAX];
    int line;
};


void *sa_malloc_list_top = NULL;


static void *
sa_malloc_aux(size_t size, char *file, int line)
{
    SA_MALLOC_LIST *tmp;
    void *ptr;

    ptr = malloc(size);
    if (ptr == NULL)
	return NULL;

    memset(ptr, NEW_GARBAGE, size);

    tmp = malloc(sizeof(SA_MALLOC_LIST));
    if (tmp == NULL) {
	free(ptr);
	return NULL;
    }

    tmp->ptr = ptr;
    tmp->size = size;
    tmp->next = sa_malloc_list_top;
    tmp->line = line;
    strcpy(tmp->file, file); // FIXME
    sa_malloc_list_top = tmp;

    return ptr;
}


static void *
sa_calloc_aux(size_t nmemb, size_t size, char *file, int line)
{
    SA_MALLOC_LIST *tmp;
    void *ptr;

    ptr = calloc(nmemb, size);
    if (ptr == NULL)
	return NULL;

    tmp = malloc(sizeof(SA_MALLOC_LIST));
    if (tmp == NULL) {
	free(ptr);
	return NULL;
    }

    tmp->ptr = ptr;
    tmp->size = size;
    tmp->next = sa_malloc_list_top;
    tmp->line = line;
    strcpy(tmp->file, file); // FIXME
    sa_malloc_list_top = tmp;

    return ptr;
}


static void
sa_free_aux(void *ptr, char *file, int line)
{
    SA_MALLOC_LIST *prev, *cur;

    if (ptr == NULL)
	fprintf(stderr, "WARNING: FREE NULL POINTER, \"%s\", %d\n",
		file, line);

    prev = NULL;
    for (cur = sa_malloc_list_top; cur != NULL; cur = cur->next) {
	if (cur->ptr == ptr) {
	    if (prev == NULL)
		sa_malloc_list_top = cur->next;
	    else
		prev->next = cur->next;

	    memset(ptr, OLD_GARBAGE, cur->size);
	    free(cur);
	    free(ptr);
	    return;
	}
	prev = cur;
    }
    if (ptr == NULL) {
	fprintf(stderr, "FAIL TO FREE, \"%s\", %d\n", file, line);
	assert("FAIL TO FREE" && 0);
	exit(1);
    }
}


int
sa_memory_leak_check(void)
{
    if (sa_malloc_list_top == NULL) {
	return 1;
    } else {
	SA_MALLOC_LIST *next;
	for ( ; sa_malloc_list_top != NULL; sa_malloc_list_top = next) {
	    next = ((SA_MALLOC_LIST *)sa_malloc_list_top)->next;
	    fprintf(stderr, "MEMORY LEAK, \"%s\", %d\n", 
		    ((SA_MALLOC_LIST *)sa_malloc_list_top)->file,
		    ((SA_MALLOC_LIST *)sa_malloc_list_top)->line);
	    free((SA_MALLOC_LIST *)sa_malloc_list_top);
	}
	return 0;
    }
}


int
sa_set_memory_debug_mode(int mode) 
{
    if (mode == 1) {
	sa_malloc_func_ptr = sa_malloc_aux;
	sa_calloc_func_ptr = sa_calloc_aux;
	sa_free_func_ptr = sa_free_aux;
    } else if (mode == 0) {
	sa_malloc_func_ptr = malloc;
	sa_calloc_func_ptr = calloc;
	sa_free_func_ptr = free;
    } else {
	return 0;		/* illegal mode setting */
    }

    return 1;
}



/*
int main()
{
char *p, *q, *r;

p = sa_malloc(72);
q = sa_malloc(72);
r = sa_malloc(72);

sa_free(r);
sa_free(p);


assert(sa_memory_leak_check());

return 0;
}
*/
