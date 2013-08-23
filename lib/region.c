/*
 * $Id: region.c,v 1.3 2000/07/05 04:21:50 tatuo-y Exp $
 *
 * Extract region
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sufary.h"
#include "util.h"
#include "config.h"

/*
 * Copy from string src to string dst,
 * and add a null character to the tail of dst.
 */
size_t
sa_copy_region(char *dst, const SA_STRING src)
{
    memcpy(dst, src.ptr, src.len);
    dst[src.len] = '\0';
    return src.len;
}

/*
 * Allocate memory, copy from string src
 * and add a null character to the tail.
 */
char *
sa_dup_region(const SA_STRING src)
{
    char *dst;

    dst = malloc(src.len + 1);
    if (dst == NULL) {
	sa_error_no = MEMORY_ALLOCATE_ERROR;
	return NULL;
    }
    sa_copy_region(dst, src);
    return dst;
}

/*
 * Extract a line containing the position referenced by pos
 * and the backward bkwrd lines and the forward frwrd lines.
 */
SA_STRING
sa_seek_context_lines(const SUFARY *ary, char *pos,
		      int bkwrd, int frwrd)
{
    char *head, *tail;
    char *bof, *eof;
    SA_STRING str;

    assert(bkwrd >= 0 && frwrd >=0);
    assert(pos >= sa_get_text_ptr(ary));

    bof = sa_get_text_ptr(ary);
    eof = bof + sa_get_text_size(ary);

    head = sa_seek_bol(pos, bof);
    tail = sa_seek_eol(head, eof);

    while (bof < head && bkwrd > 0) {
	head = sa_seek_bol(head - 1, bof);
	bkwrd--;
    }
    while (eof > tail && frwrd > 0) {
	tail = sa_seek_eol(tail + 1, eof);
	frwrd--;
    }

    str.ptr = head;
    str.len = tail - head;

    return str;
}

/*
 * Seek the begining of the line
 * containing the position referenced by pos.
 * bof is the begining of file or a limit of seeking.
 */
char *
sa_seek_bol(char *pos, char *bof)
{
    assert(pos >= bof);

    while (bof < pos) {
	pos--;
	if (*pos == '\n')
	    return pos + 1;
    }
    return bof;
}

/*
 * Seek the end of the line(the next character of '\n')
 * containing the position referenced by pos.
 * eof is the end of file or a limit of seeking.
 */
char *
sa_seek_eol(char *pos, char *eof)
{
    assert(pos <= eof);

    while (pos < eof) {
	if (*pos == '\n')
	    return pos + 1;
	pos++;
    }
    return eof;
}

/*
 * Extract the region which contains the position referenced by pos,
 * and begins with begin_tag and ends with end_tag.
 */
SA_STRING
sa_seek_context_region(const SUFARY *ary, char *pos,
		       const SA_STRING begin_tag,
		       const SA_STRING end_tag)
{
    char *head, *tail;
    char *bof, *eof;
    SA_STRING str;

    assert(pos >= sa_get_text_ptr(ary));

    bof = sa_get_text_ptr(ary);
    eof = bof + sa_get_text_size(ary);

    head = sa_seek_pattern_backward(pos, bof, begin_tag);
    tail = sa_seek_pattern_forward(pos, eof, end_tag); 

    str.ptr = head;
    str.len = tail - head;

    return str;
}

/*
 * Seek the pattern with scanning backword from pos.
 * bof is the beginning of file or a limit of seek.
 */
char *
sa_seek_pattern_backward(char *pos, char *bof, const SA_STRING pattern)
{
    assert(pos >= bof);

    while (bof < pos) {
	if (memcmp(pos, pattern.ptr, pattern.len) == 0)
	    return pos;
	pos--;
    }
    return bof;
}

/*
 * Seek the pattern with scanning forword from pos
 * (the next character of pattern).
 * eof is the end of file or a limit of seek.
 */
char *
sa_seek_pattern_forward(char *pos, char *eof, const SA_STRING pattern)
{
    assert(pos < eof);

    while (pos <= eof - pattern.len) {
	if (memcmp(pos, pattern.ptr, pattern.len) == 0)
	    return pos + pattern.len;
	pos++;
    }
    return eof;
}

/*
 * Get a relative position in SA_STRING
 */
SA_INDEX
sa_position_in_string(const SUFARY *ary, SA_STRING sstr, SA_INDEX pos)
{
    SA_INDEX relative_pos;
    char *cur;
    cur = sa_txtidx2txtptr(ary, pos);
    relative_pos =  cur - sstr.ptr;
    return relative_pos;
}
