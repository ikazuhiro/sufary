/*
  $Id: sufary.c,v 1.5 2000/11/17 02:51:50 tatuo-y Exp $
 */

#include "sufary.h"
#include "did.h"
#include "ruby.h"

#define GET_SUFARY(obj, dat) Data_Get_Struct(obj, SUFARY, dat)
#define GET_OFF_T(obj) (off_t)NUM2LONG(obj)
#define GET_SUFARYRDID(obj, dat) Data_Get_Struct(obj, R_DID, dat)

typedef struct {
    SUFARY *ary;
    DID *did;
} R_DID;

static VALUE Sufary;
static VALUE SufaryDid;
static VALUE SufaryError;

/* For Sufary class. */
static VALUE sufary_s_new(int argc, VALUE* argv, VALUE klass);
static void sufary_free(SUFARY *ary);
static VALUE sufary_close(VALUE klass);
static VALUE sufary_array_size(VALUE klass);
static VALUE sufary_text_size(VALUE klass);
static VALUE sufary_search(int argc, VALUE *argv, VALUE klass);
static VALUE sufary_get_context_lines(int argc, VALUE *argv, VALUE klass);
static VALUE sufary_get_context_region(VALUE klass, VALUE stridx,
				       VALUE start_tag, VALUE end_tag);
static VALUE sufary_get_string(VALUE klass, VALUE txtidx, VALUE length);
static VALUE split_region(SA_STRING region, char *key, VALUE keylen);

/* For SufaryDid class. */
static VALUE sufarydid_s_new(VALUE klass, VALUE path, VALUE array);
static void sufarydid_free(R_DID *rdid);
static VALUE sufarydid_get_doc_number(VALUE klass, VALUE stridx);
static VALUE sufarydid_get_doc_region(VALUE klass, VALUE stridx);

/*
 * Initilize Sufary class instance.
 */
static VALUE
sufary_s_new(int argc, VALUE* argv, VALUE klass)
{
    SUFARY *ary;
    VALUE text_path, array_path;
    char *textp, *arryp;
  
    rb_scan_args(argc, argv, "11", &text_path, &array_path);  

    Check_SafeStr(text_path);
    textp = STR2CSTR(text_path);
    if (array_path != Qnil) {
	Check_SafeStr(array_path);
	arryp = STR2CSTR(array_path);
    } else {
	arryp = NULL;
    }
  
    ary = sa_open(textp, arryp);
    if (ary == NULL)
	rb_raise(SufaryError, sa_get_error_str);

    return Data_Wrap_Struct(klass, 0, sufary_free, ary);
}

/*
 * Destroy instance.
 */
static void
sufary_free(SUFARY *ary)
{
    sa_close(ary);
}

/*
 * Instance methods.
 */
static VALUE
sufary_array_size(VALUE klass)
{
    SUFARY *ary;

    GET_SUFARY(klass, ary);

    return INT2NUM(ary->arraysize);
}

static VALUE
sufary_text_size(VALUE klass)
{
    SUFARY *ary;

    GET_SUFARY(klass, ary);

    return INT2NUM(ary->textsize);
}

static VALUE
sufary_search(int argc, VALUE *argv, VALUE klass)
{
    SUFARY *ary;
    SUF_RESULT result;
    VALUE keyword, left, right, base_text_skip, indexes;
    char *keywd;
    off_t l, r, i, j;
    int bts, len;

    GET_SUFARY(klass, ary);

    rb_scan_args(argc, argv, "13", &keyword, &left, &right,
		 &base_text_skip);
    l = (left == Qnil) ? 0 : NUM2LONG(left);
    r = (right == Qnil) ? ary->arraysize - 1 : NUM2LONG(right);
    bts = (base_text_skip == Qnil) ? 0 : NUM2INT(base_text_skip);

    Check_SafeStr(keyword);
    keywd = str2cstr(keyword, &len);

    result = sa_find(ary, l, r, keywd, len, bts);
    if (result.stat == FAIL)
	return rb_ary_new2(0);
    else {
	indexes = rb_ary_new2(result.right - result.left + 1);
	for (i = result.left, j = 0; i <= result.right; i++, j++) {
	    off_t txtidx = sa_aryidx2txtidx(ary, i);
	    rb_ary_store(indexes, j,
			 rb_ary_new3(2, INT2NUM(txtidx), INT2NUM(len)));
	}
	return indexes;
    }
}

static VALUE
sufary_get_context_lines(int argc, VALUE *argv, VALUE klass)
{
    SUFARY *ary;
    SA_STRING region;
    VALUE stridx, bkwrd, frwrd;
    int bk, fr;
    char *key;

    rb_scan_args(argc, argv, "12", &stridx, &bkwrd, &frwrd);
    bk = (bkwrd == Qnil) ? 0 : NUM2INT(bkwrd);
    fr = (frwrd == Qnil) ? 0 : NUM2INT(frwrd);

    GET_SUFARY(klass, ary);
    key = sa_txtidx2txtptr(ary,
			   GET_OFF_T(rb_ary_entry(stridx, 0)));
    region = sa_seek_context_lines(ary, key, bk, fr);

    return split_region(region, key, rb_ary_entry(stridx, 1));
}

static VALUE
sufary_get_context_region(VALUE klass, VALUE stridx,
			  VALUE start_tag, VALUE end_tag)
{
    SUFARY *ary;
    SA_STRING stag, etag, region;
    char *key;

    GET_SUFARY(klass, ary);
    Check_SafeStr(start_tag);
    Check_SafeStr(end_tag);
    stag.ptr = rb_str2cstr(start_tag, &(stag.len));
    etag.ptr = rb_str2cstr(end_tag, &(etag.len));
    key = sa_txtidx2txtptr(ary,
 			   GET_OFF_T(rb_ary_entry(stridx, 0)));
    region = sa_seek_context_region(ary, key, stag, etag);

    return split_region(region, key, rb_ary_entry(stridx, 1));
}

static VALUE
sufary_get_string(VALUE klass, VALUE txtidx, VALUE length)
{
    SUFARY *ary;
    char *ptr;
    int len;

    GET_SUFARY(klass, ary);
    ptr = sa_txtidx2txtptr(ary, GET_OFF_T(txtidx));
    len = NUM2INT(length);

    return rb_str_new(ptr, len);
}

static VALUE
split_region(SA_STRING region, char *key, VALUE keylen)
{
    VALUE bstr, kstr, astr;
    size_t blen, klen, alen;
    char *bp, *ap;

    bp = region.ptr;
    blen = key - bp;
    klen = NUM2LONG(keylen);
    ap = key + klen;
    alen = region.len - blen - klen;

    bstr = rb_str_new(bp, blen);
    kstr = rb_str_new(key, klen);
    astr = rb_str_new(ap, alen);

    return rb_ary_new3(3, bstr, kstr, astr);
}

/*
 * For SufaryDid class.
 */
static VALUE
sufarydid_s_new(VALUE klass, VALUE path, VALUE array)
{
    R_DID *rdid;
    SUFARY *ary;

    Check_SafeStr(path);
    rdid = ALLOC(R_DID);
    rdid->did = sa_open_did(STR2CSTR(path));
    if (rdid->did == NULL)
	rb_raise(SufaryError, sa_get_error_str);

    GET_SUFARY(array, ary);
    rdid->ary = ary;

    return Data_Wrap_Struct(klass, 0, sufarydid_free, rdid);
}

static void
sufarydid_free(R_DID *rdid)
{
    sa_close_did(rdid->did);
}

static VALUE
sufarydid_get_doc_number(VALUE klass, VALUE stridx)
{
    R_DID *rdid;
    DID_RESULT dr;

    GET_SUFARYRDID(klass, rdid);
    dr = sa_didsearch(rdid->did,
		      GET_OFF_T(rb_ary_entry(stridx, 0)));

    return INT2NUM(dr.no);
}

static VALUE
sufarydid_get_doc_region(VALUE klass, VALUE stridx)
{
    SUFARY *ary;
    R_DID *rdid;
    DID_RESULT dr;
    SA_STRING doc;
    off_t index;

    GET_SUFARYRDID(klass, rdid);
    index = GET_OFF_T(rb_ary_entry(stridx, 0));

    dr = sa_didsearch(rdid->did, index);
    if (dr.no < 0)
	return Qnil;
    doc.ptr = sa_txtidx2txtptr(rdid->ary, dr.start);
    doc.len = dr.size;

    return split_region(doc, sa_txtidx2txtptr(rdid->ary, index),
			rb_ary_entry(stridx, 1));
}

/*
 * Initialize class
 */
void
Init_sufary(void)
{
    Sufary = rb_define_class("Sufary", rb_cObject);
    SufaryDid = rb_define_class("SufaryDid", rb_cObject);
    SufaryError = rb_define_class("SufaryError", rb_eStandardError);

    rb_define_singleton_method(Sufary, "new", sufary_s_new, -1);
    rb_define_method(Sufary, "array_size", sufary_array_size, 0);
    rb_define_method(Sufary, "text_size", sufary_text_size, 0);
    rb_define_method(Sufary, "search", sufary_search, -1);
    rb_define_method(Sufary, "get_context_lines",
		     sufary_get_context_lines, -1);
    rb_define_method(Sufary, "get_context_region",
		     sufary_get_context_region, 3);
    rb_define_method(Sufary, "get_string", sufary_get_string, 2);

    rb_define_singleton_method(SufaryDid, "new", sufarydid_s_new, 2);
    rb_define_method(SufaryDid, "get_doc_number",
		     sufarydid_get_doc_number, 1);
    rb_define_method(SufaryDid, "get_doc_region",
		     sufarydid_get_doc_region, 1);
}
