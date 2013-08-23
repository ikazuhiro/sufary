/*
 * $Id: SUFARY.xs,v 1.13 2000/11/17 02:51:50 tatuo-y Exp $
 *
 * written by T.Nakayama
 */
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "sufary.h"
#include "util.h"

MODULE = SUFARY PACKAGE = SUFARY
PROTOTYPES: ENABLE

SUFARY *
suf_openfile(s,t=NULL)
	char *s
	char *t
	CODE:
	RETVAL = sa_open(s,t);
	OUTPUT:
	RETVAL


void
suf_closefile(ary)
	SUFARY *ary;
	CODE:
	sa_close(ary);


void
suf_find(ary,pat,left=0,right=sa_get_array_size(ary)-1,skip=0)
	SUFARY *ary
	SA_STRING pat
	off_t left
	off_t right
	int skip
	PREINIT:
	SUF_RESULT sr;
	off_t i;
	PPCODE:
	sr = sa_find(ary, left, right, pat.ptr, pat.len, skip);
    
	if (sr.stat == SUCCESS) {
		EXTEND(SP, sr.right - sr.left + 1);
		for (i = sr.left; i <= sr.right; i++) {
			PUSHs(sv_2mortal(newSVnv(
			    sa_aryidx2txtidx(ary, i))));
		}
	}


void
suf_range_search(ary,pat,left=0,right=sa_get_array_size(ary)-1,skip=0)
	SUFARY *ary
	SA_STRING pat
	off_t left
	off_t right
	int skip
	PREINIT:
	SUF_RESULT sr;
	PPCODE:
	sr = sa_find(ary, left, right, pat.ptr, pat.len, skip);
    
	if (sr.stat == SUCCESS) {
		EXTEND(SP, 2);
		PUSHs(sv_2mortal(newSVnv(sr.left)));
		PUSHs(sv_2mortal(newSVnv(sr.right)));
	}


off_t
suf_get_position(ary,i)
	SUFARY *ary
	off_t i;
	CODE:
	RETVAL = sa_aryidx2txtidx(ary, i);
	OUTPUT:
	RETVAL


void
suf_getline(ary,pos,bkwrd=0,frwrd=0)
	SUFARY* ary
	off_t pos
	off_t bkwrd
	off_t frwrd
	PREINIT:
	SA_STRING sstr;
	PPCODE:
	sstr = sa_seek_context_lines(ary,
	    sa_txtidx2txtptr(ary, pos), bkwrd, frwrd);
	XPUSHs(sv_2mortal(newSVpvn(sstr.ptr, sstr.len)));


void
suf_get_line_info(ary,pos,bkwrd=0,frwrd=0)
	SUFARY* ary
	off_t pos
	off_t bkwrd
	off_t frwrd
	PREINIT:
	SA_STRING sstr;
	PPCODE:
	sstr = sa_seek_context_lines(ary,
	    sa_txtidx2txtptr(ary, pos), bkwrd, frwrd);
	XPUSHs(sv_2mortal(newSVnv(sstr.ptr - sa_get_text_ptr(ary))));	
	XPUSHs(sv_2mortal(newSVnv(sstr.len)));


void
suf_getstr(ary,from,size)
	SUFARY* ary
	off_t from
	off_t size
	PPCODE:
	XPUSHs(sv_2mortal(newSVpvn(sa_txtidx2txtptr(ary, from), size)));


void
suf_block(ary,pos,btag,etag)
	SUFARY* ary
	off_t pos
	SA_STRING btag
	SA_STRING etag
	PREINIT:
	char *key;
	SA_STRING sstr;
	PPCODE:
	key = sa_txtidx2txtptr(ary,pos);
	sstr = sa_seek_context_region(ary,key,btag,etag);
	XPUSHs(sv_2mortal(newSVpvn(sstr.ptr, sstr.len)));


void
suf_get_region_info(ary,pos,btag,etag)
	SUFARY* ary
	off_t pos
	SA_STRING btag
	SA_STRING etag
	PREINIT:
	char *key;
	SA_STRING sstr;
	PPCODE:
	key = sa_txtidx2txtptr(ary,pos);
	sstr = sa_seek_context_region(ary,key,btag,etag);
	XPUSHs(sv_2mortal(newSVnv(sstr.ptr - sa_get_text_ptr(ary))));	
	XPUSHs(sv_2mortal(newSVnv(sstr.len)));


off_t
suf_arraysize(ary)
	SUFARY* ary;
	CODE:
	RETVAL = sa_get_array_size(ary);
	OUTPUT:
	RETVAL

off_t
suf_textsize(ary)
	SUFARY* ary;
	CODE:
	RETVAL = sa_get_text_size(ary);
	OUTPUT:
	RETVAL


void
suf_regex_search(ary,pat,left=0,right=sa_get_array_size(ary)-1)
	SUFARY* ary
	SA_STRING pat
	off_t left
	off_t right
	PREINIT:
	SA_RESULT_LIST *ll, *tmp;
	PPCODE:
	ll = sa_regex(ary, left, right, pat.ptr, pat.len);

	for (tmp = ll; tmp != NULL; tmp = tmp->next) {
		XPUSHs(sv_2mortal(newSVnv(tmp->value)));
	}
	sa_free_result_list(ll);


void
suf_case_insensitive_search(ary,pat,left=0,right=sa_get_array_size(ary)-1)
	SUFARY* ary
	SA_STRING pat
	off_t left
	off_t right
	PREINIT:
	SA_RESULT_LIST *ll, *tmp;
	PPCODE:
	ll = sa_ignore_case(ary, left, right, pat.ptr, pat.len);

	for (tmp = ll; tmp != NULL; tmp = tmp->next) {
		XPUSHs(sv_2mortal(newSVnv(tmp->value)));
	}
	sa_free_result_list(ll);


char *
suf_error_msg()
	CODE:
	RETVAL = sa_error_str[sa_error_no];
	OUTPUT:
	RETVAL


int
suf_error_no()
	CODE:
	RETVAL = sa_error_no;
	OUTPUT:
	RETVAL


void
suf_set_debug_mode()
	CODE:
	sa_set_debug_mode(1);


int
suf_memory_leak_check()
	CODE:
	RETVAL = sa_memory_leak_check();
	OUTPUT:
	RETVAL

