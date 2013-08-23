/*
 * $Id: util.c,v 1.13 2000/06/20 04:24:00 kazuma-t Exp $
 */

#include <stdio.h>

#include "config.h"
#include "util.h"

static int m_debug_mode = 0;


/*
 * reverse byte ordering: one
 */
void sa_reverse_byte_order(void *p, int size)
{
    int j;
    char *c = (char*)p;

    for (j = 0; j < (size / 2); j++) {
	char tmp = *(c + j);
	*(c + j)= *(c + size - 1 - j);
	*(c + size - 1 - j) = tmp;
    }
}


/*
 * reverse byte ordering: all
 */
void
sa_reverse_byte_order_all(void *p, int size, size_t num)
{
    /* You can make this faster.  Try it. */
    size_t i;
    for (i = 0; i < num; i++)
	sa_reverse_byte_order((char *)p + (i * size), size);
}	


/* 
 * make new file name: file name + suffix
 */
char* sa_add_suffix_to_file_name(char *new_fname, const char *fname,
				 const char *suffix)
{
    sprintf(new_fname, "%s.%s", fname, suffix);
    return new_fname;
}

/* 
 * debug mode
 */
int is_debug_mode(void)
{
    return m_debug_mode;
}

void set_debug_mode(int i)
{
    m_debug_mode = i;
}

/*
 * print debug message
 */
void sa_dprintf(char *fmt, ...)
{
    va_list args;

    if (m_debug_mode == 0)
        return;

    fflush(stdout);
    fprintf(stderr, "DEBUG %s ", PACKAGE);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fflush(stderr);
}
