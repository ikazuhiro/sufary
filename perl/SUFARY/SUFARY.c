/*
 * This file was generated automatically by xsubpp version 1.9507 from the 
 * contents of SUFARY.xs. Do not edit this file, edit SUFARY.xs instead.
 *
 *	ANY CHANGES MADE HERE WILL BE LOST! 
 *
 */

#line 1 "SUFARY.xs"
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

#line 23 "SUFARY.c"
XS(XS_SUFARY_suf_openfile)
{
    dXSARGS;
    if (items < 1 || items > 2)
	croak("Usage: SUFARY::suf_openfile(s,t=NULL)");
    {
	char *	s = (char *)SvPV(ST(0),PL_na);
	char *	t;
	SUFARY *	RETVAL;

	if (items < 2)
	    t = NULL;
	else {
	    t = (char *)SvPV(ST(1),PL_na);
	}
#line 21 "SUFARY.xs"
	RETVAL = sa_open(s,t);
#line 41 "SUFARY.c"
	ST(0) = sv_newmortal();
	sv_setref_pv(ST(0), "SUFARYPtr", (void*)RETVAL);
    }
    XSRETURN(1);
}

XS(XS_SUFARY_suf_closefile)
{
    dXSARGS;
    if (items != 1)
	croak("Usage: SUFARY::suf_closefile(ary)");
    {
	SUFARY *	ary;

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");
#line 30 "SUFARY.xs"
	sa_close(ary);
#line 64 "SUFARY.c"
    }
    XSRETURN_EMPTY;
}

XS(XS_SUFARY_suf_find)
{
    dXSARGS;
    if (items < 2 || items > 5)
	croak("Usage: SUFARY::suf_find(ary,pat,left=0,right=sa_get_array_size(ary)-1,skip=0)");
    SP -= items;
    {
	SUFARY *	ary;
	SA_STRING	pat;
	off_t	left;
	off_t	right;
	int	skip;
#line 41 "SUFARY.xs"
	SUF_RESULT sr;
	off_t i;
#line 84 "SUFARY.c"

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");

	{
	  int len;
	  pat.ptr = (char *)SvPV(ST(1),len);
	  pat.len = len;
	};

	if (items < 3)
	    left = 0;
	else {
	    left = (off_t)SvIV(ST(2));
	}

	if (items < 4)
	    right = sa_get_array_size(ary)-1;
	else {
	    right = (off_t)SvIV(ST(3));
	}

	if (items < 5)
	    skip = 0;
	else {
	    skip = (int)SvIV(ST(4));
	}
#line 44 "SUFARY.xs"
	sr = sa_find(ary, left, right, pat.ptr, pat.len, skip);

	if (sr.stat == SUCCESS) {
		EXTEND(SP, sr.right - sr.left + 1);
		for (i = sr.left; i <= sr.right; i++) {
			PUSHs(sv_2mortal(newSVnv(
			    sa_aryidx2txtidx(ary, i))));
		}
	}
#line 126 "SUFARY.c"
	PUTBACK;
	return;
    }
}

XS(XS_SUFARY_suf_range_search)
{
    dXSARGS;
    if (items < 2 || items > 5)
	croak("Usage: SUFARY::suf_range_search(ary,pat,left=0,right=sa_get_array_size(ary)-1,skip=0)");
    SP -= items;
    {
	SUFARY *	ary;
	SA_STRING	pat;
	off_t	left;
	off_t	right;
	int	skip;
#line 63 "SUFARY.xs"
	SUF_RESULT sr;
#line 146 "SUFARY.c"

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");

	{
	  int len;
	  pat.ptr = (char *)SvPV(ST(1),len);
	  pat.len = len;
	};

	if (items < 3)
	    left = 0;
	else {
	    left = (off_t)SvIV(ST(2));
	}

	if (items < 4)
	    right = sa_get_array_size(ary)-1;
	else {
	    right = (off_t)SvIV(ST(3));
	}

	if (items < 5)
	    skip = 0;
	else {
	    skip = (int)SvIV(ST(4));
	}
#line 65 "SUFARY.xs"
	sr = sa_find(ary, left, right, pat.ptr, pat.len, skip);

	if (sr.stat == SUCCESS) {
		EXTEND(SP, 2);
		PUSHs(sv_2mortal(newSVnv(sr.left)));
		PUSHs(sv_2mortal(newSVnv(sr.right)));
	}
#line 186 "SUFARY.c"
	PUTBACK;
	return;
    }
}

XS(XS_SUFARY_suf_get_position)
{
    dXSARGS;
    if (items != 2)
	croak("Usage: SUFARY::suf_get_position(ary,i)");
    {
	SUFARY *	ary;
	off_t	i = (off_t)SvIV(ST(1));
	off_t	RETVAL;

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");
#line 79 "SUFARY.xs"
	RETVAL = sa_aryidx2txtidx(ary, i);
#line 210 "SUFARY.c"
	ST(0) = sv_newmortal();
	sv_setiv(ST(0), (IV)RETVAL);
    }
    XSRETURN(1);
}

XS(XS_SUFARY_suf_getline)
{
    dXSARGS;
    if (items < 2 || items > 4)
	croak("Usage: SUFARY::suf_getline(ary,pos,bkwrd=0,frwrd=0)");
    SP -= items;
    {
	SUFARY*	ary;
	off_t	pos = (off_t)SvIV(ST(1));
	off_t	bkwrd;
	off_t	frwrd;
#line 91 "SUFARY.xs"
	SA_STRING sstr;
#line 230 "SUFARY.c"

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");

	if (items < 3)
	    bkwrd = 0;
	else {
	    bkwrd = (off_t)SvIV(ST(2));
	}

	if (items < 4)
	    frwrd = 0;
	else {
	    frwrd = (off_t)SvIV(ST(3));
	}
#line 93 "SUFARY.xs"
	sstr = sa_seek_context_lines(ary,
	    sa_txtidx2txtptr(ary, pos), bkwrd, frwrd);
	XPUSHs(sv_2mortal(newSVpvn(sstr.ptr, sstr.len)));
#line 254 "SUFARY.c"
	PUTBACK;
	return;
    }
}

XS(XS_SUFARY_suf_get_line_info)
{
    dXSARGS;
    if (items < 2 || items > 4)
	croak("Usage: SUFARY::suf_get_line_info(ary,pos,bkwrd=0,frwrd=0)");
    SP -= items;
    {
	SUFARY*	ary;
	off_t	pos = (off_t)SvIV(ST(1));
	off_t	bkwrd;
	off_t	frwrd;
#line 105 "SUFARY.xs"
	SA_STRING sstr;
#line 273 "SUFARY.c"

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");

	if (items < 3)
	    bkwrd = 0;
	else {
	    bkwrd = (off_t)SvIV(ST(2));
	}

	if (items < 4)
	    frwrd = 0;
	else {
	    frwrd = (off_t)SvIV(ST(3));
	}
#line 107 "SUFARY.xs"
	sstr = sa_seek_context_lines(ary,
	    sa_txtidx2txtptr(ary, pos), bkwrd, frwrd);
	XPUSHs(sv_2mortal(newSVnv(sstr.ptr - sa_get_text_ptr(ary))));	
	XPUSHs(sv_2mortal(newSVnv(sstr.len)));
#line 298 "SUFARY.c"
	PUTBACK;
	return;
    }
}

XS(XS_SUFARY_suf_getstr)
{
    dXSARGS;
    if (items != 3)
	croak("Usage: SUFARY::suf_getstr(ary,from,size)");
    SP -= items;
    {
	SUFARY*	ary;
	off_t	from = (off_t)SvIV(ST(1));
	off_t	size = (off_t)SvIV(ST(2));

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");
#line 119 "SUFARY.xs"
	XPUSHs(sv_2mortal(newSVpvn(sa_txtidx2txtptr(ary, from), size)));
#line 323 "SUFARY.c"
	PUTBACK;
	return;
    }
}

XS(XS_SUFARY_suf_block)
{
    dXSARGS;
    if (items != 4)
	croak("Usage: SUFARY::suf_block(ary,pos,btag,etag)");
    SP -= items;
    {
	SUFARY*	ary;
	off_t	pos = (off_t)SvIV(ST(1));
	SA_STRING	btag;
	SA_STRING	etag;
#line 129 "SUFARY.xs"
	char *key;
	SA_STRING sstr;
#line 343 "SUFARY.c"

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");

	{
	  int len;
	  btag.ptr = (char *)SvPV(ST(2),len);
	  btag.len = len;
	};

	{
	  int len;
	  etag.ptr = (char *)SvPV(ST(3),len);
	  etag.len = len;
	};
#line 132 "SUFARY.xs"
	key = sa_txtidx2txtptr(ary,pos);
	sstr = sa_seek_context_region(ary,key,btag,etag);
	XPUSHs(sv_2mortal(newSVpvn(sstr.ptr, sstr.len)));
#line 367 "SUFARY.c"
	PUTBACK;
	return;
    }
}

XS(XS_SUFARY_suf_get_region_info)
{
    dXSARGS;
    if (items != 4)
	croak("Usage: SUFARY::suf_get_region_info(ary,pos,btag,etag)");
    SP -= items;
    {
	SUFARY*	ary;
	off_t	pos = (off_t)SvIV(ST(1));
	SA_STRING	btag;
	SA_STRING	etag;
#line 144 "SUFARY.xs"
	char *key;
	SA_STRING sstr;
#line 387 "SUFARY.c"

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");

	{
	  int len;
	  btag.ptr = (char *)SvPV(ST(2),len);
	  btag.len = len;
	};

	{
	  int len;
	  etag.ptr = (char *)SvPV(ST(3),len);
	  etag.len = len;
	};
#line 147 "SUFARY.xs"
	key = sa_txtidx2txtptr(ary,pos);
	sstr = sa_seek_context_region(ary,key,btag,etag);
	XPUSHs(sv_2mortal(newSVnv(sstr.ptr - sa_get_text_ptr(ary))));	
	XPUSHs(sv_2mortal(newSVnv(sstr.len)));
#line 412 "SUFARY.c"
	PUTBACK;
	return;
    }
}

XS(XS_SUFARY_suf_arraysize)
{
    dXSARGS;
    if (items != 1)
	croak("Usage: SUFARY::suf_arraysize(ary)");
    {
	SUFARY*	ary;
	off_t	RETVAL;

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");
#line 157 "SUFARY.xs"
	RETVAL = sa_get_array_size(ary);
#line 435 "SUFARY.c"
	ST(0) = sv_newmortal();
	sv_setiv(ST(0), (IV)RETVAL);
    }
    XSRETURN(1);
}

XS(XS_SUFARY_suf_textsize)
{
    dXSARGS;
    if (items != 1)
	croak("Usage: SUFARY::suf_textsize(ary)");
    {
	SUFARY*	ary;
	off_t	RETVAL;

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");
#line 165 "SUFARY.xs"
	RETVAL = sa_get_text_size(ary);
#line 459 "SUFARY.c"
	ST(0) = sv_newmortal();
	sv_setiv(ST(0), (IV)RETVAL);
    }
    XSRETURN(1);
}

XS(XS_SUFARY_suf_regex_search)
{
    dXSARGS;
    if (items < 2 || items > 4)
	croak("Usage: SUFARY::suf_regex_search(ary,pat,left=0,right=sa_get_array_size(ary)-1)");
    SP -= items;
    {
	SUFARY*	ary;
	SA_STRING	pat;
	off_t	left;
	off_t	right;
#line 177 "SUFARY.xs"
	SA_RESULT_LIST *ll, *tmp;
#line 479 "SUFARY.c"

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");

	{
	  int len;
	  pat.ptr = (char *)SvPV(ST(1),len);
	  pat.len = len;
	};

	if (items < 3)
	    left = 0;
	else {
	    left = (off_t)SvIV(ST(2));
	}

	if (items < 4)
	    right = sa_get_array_size(ary)-1;
	else {
	    right = (off_t)SvIV(ST(3));
	}
#line 179 "SUFARY.xs"
	ll = sa_regex(ary, left, right, pat.ptr, pat.len);

	for (tmp = ll; tmp != NULL; tmp = tmp->next) {
		XPUSHs(sv_2mortal(newSVnv(tmp->value)));
	}
	sa_free_result_list(ll);
#line 512 "SUFARY.c"
	PUTBACK;
	return;
    }
}

XS(XS_SUFARY_suf_case_insensitive_search)
{
    dXSARGS;
    if (items < 2 || items > 4)
	croak("Usage: SUFARY::suf_case_insensitive_search(ary,pat,left=0,right=sa_get_array_size(ary)-1)");
    SP -= items;
    {
	SUFARY*	ary;
	SA_STRING	pat;
	off_t	left;
	off_t	right;
#line 194 "SUFARY.xs"
	SA_RESULT_LIST *ll, *tmp;
#line 531 "SUFARY.c"

	if (sv_derived_from(ST(0), "SUFARYPtr")) {
	    IV tmp = SvIV((SV*)SvRV(ST(0)));
	    ary = (SUFARY *) tmp;
	}
	else
	    croak("ary is not of type SUFARYPtr");

	{
	  int len;
	  pat.ptr = (char *)SvPV(ST(1),len);
	  pat.len = len;
	};

	if (items < 3)
	    left = 0;
	else {
	    left = (off_t)SvIV(ST(2));
	}

	if (items < 4)
	    right = sa_get_array_size(ary)-1;
	else {
	    right = (off_t)SvIV(ST(3));
	}
#line 196 "SUFARY.xs"
	ll = sa_ignore_case(ary, left, right, pat.ptr, pat.len);

	for (tmp = ll; tmp != NULL; tmp = tmp->next) {
		XPUSHs(sv_2mortal(newSVnv(tmp->value)));
	}
	sa_free_result_list(ll);
#line 564 "SUFARY.c"
	PUTBACK;
	return;
    }
}

XS(XS_SUFARY_suf_error_msg)
{
    dXSARGS;
    if (items != 0)
	croak("Usage: SUFARY::suf_error_msg()");
    {
	char *	RETVAL;
#line 207 "SUFARY.xs"
	RETVAL = sa_error_str[sa_error_no];
#line 579 "SUFARY.c"
	ST(0) = sv_newmortal();
	sv_setpv((SV*)ST(0), RETVAL);
    }
    XSRETURN(1);
}

XS(XS_SUFARY_suf_error_no)
{
    dXSARGS;
    if (items != 0)
	croak("Usage: SUFARY::suf_error_no()");
    {
	int	RETVAL;
#line 215 "SUFARY.xs"
	RETVAL = sa_error_no;
#line 595 "SUFARY.c"
	ST(0) = sv_newmortal();
	sv_setiv(ST(0), (IV)RETVAL);
    }
    XSRETURN(1);
}

XS(XS_SUFARY_suf_set_debug_mode)
{
    dXSARGS;
    if (items != 0)
	croak("Usage: SUFARY::suf_set_debug_mode()");
    {
#line 223 "SUFARY.xs"
	sa_set_debug_mode(1);
#line 610 "SUFARY.c"
    }
    XSRETURN_EMPTY;
}

XS(XS_SUFARY_suf_memory_leak_check)
{
    dXSARGS;
    if (items != 0)
	croak("Usage: SUFARY::suf_memory_leak_check()");
    {
	int	RETVAL;
#line 229 "SUFARY.xs"
	RETVAL = sa_memory_leak_check();
#line 624 "SUFARY.c"
	ST(0) = sv_newmortal();
	sv_setiv(ST(0), (IV)RETVAL);
    }
    XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_SUFARY)
{
    dXSARGS;
    char* file = __FILE__;

    XS_VERSION_BOOTCHECK ;

        newXSproto("SUFARY::suf_openfile", XS_SUFARY_suf_openfile, file, "$;$");
        newXSproto("SUFARY::suf_closefile", XS_SUFARY_suf_closefile, file, "$");
        newXSproto("SUFARY::suf_find", XS_SUFARY_suf_find, file, "$$;$$$");
        newXSproto("SUFARY::suf_range_search", XS_SUFARY_suf_range_search, file, "$$;$$$");
        newXSproto("SUFARY::suf_get_position", XS_SUFARY_suf_get_position, file, "$$");
        newXSproto("SUFARY::suf_getline", XS_SUFARY_suf_getline, file, "$$;$$");
        newXSproto("SUFARY::suf_get_line_info", XS_SUFARY_suf_get_line_info, file, "$$;$$");
        newXSproto("SUFARY::suf_getstr", XS_SUFARY_suf_getstr, file, "$$$");
        newXSproto("SUFARY::suf_block", XS_SUFARY_suf_block, file, "$$$$");
        newXSproto("SUFARY::suf_get_region_info", XS_SUFARY_suf_get_region_info, file, "$$$$");
        newXSproto("SUFARY::suf_arraysize", XS_SUFARY_suf_arraysize, file, "$");
        newXSproto("SUFARY::suf_textsize", XS_SUFARY_suf_textsize, file, "$");
        newXSproto("SUFARY::suf_regex_search", XS_SUFARY_suf_regex_search, file, "$$;$$");
        newXSproto("SUFARY::suf_case_insensitive_search", XS_SUFARY_suf_case_insensitive_search, file, "$$;$$");
        newXSproto("SUFARY::suf_error_msg", XS_SUFARY_suf_error_msg, file, "");
        newXSproto("SUFARY::suf_error_no", XS_SUFARY_suf_error_no, file, "");
        newXSproto("SUFARY::suf_set_debug_mode", XS_SUFARY_suf_set_debug_mode, file, "");
        newXSproto("SUFARY::suf_memory_leak_check", XS_SUFARY_suf_memory_leak_check, file, "");
    XSRETURN_YES;
}
