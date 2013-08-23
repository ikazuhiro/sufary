/*
 * $Id: search.c,v 1.50 2000/11/27 09:11:15 tatuo-y Exp $
 *
 * string search
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sufary.h"
#include "util.h"
#include "config.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))


int sa_error_no;
char *sa_error_str[] = {
/* 00 */ "THE ERROR",
/* 01 */ "MEMORY ALLOCATE ERROR",
/* 02 */ "FILE OPEN ERROR",
/* 03 */ "MMAP ERROR",
/* 04 */ "KEYWORD PARSING ERROR"
};


/*
 * struct for binary search
 */
typedef struct sa_bsearch_result_ {
    SA_INDEX center;
    SA_INDEX left;
    SA_INDEX right;
    int left_prefix_len;
    int right_prefix_len;
    int base_txt_skip;
    SA_STAT state;
} SA_BSEARCH_RESULT;


/*
 * set debug mode
 */
void
sa_set_debug_mode(int i)
{
    set_debug_mode(i);
}


/*
 * sistring(半無限文字列) の比較
 *
 * return value
 *   0 : MATCH 一致
 *   - : LESS  sistring(pos) < str  ('abc...' < 'ccc')
 *   + : ABOVE sistring(pos) > str  ('abc...' > 'aaa')
 */
static int
cmp_sistr(
    const unsigned char *txt,	/* 比較される文字列(検索対象テキスト) */
    const int *str,		/* 比較する文字列(検索キーワード) */
    int *diffpos,		/* 始めて異なった位置(文字目) */
    int len			/* 比較する文字数 */
    )
{
    int i;

    assert(txt != NULL && str != NULL);
    assert(0 <= len);

    for (i = 0; i < len; i++) {
	int t = (int)(*txt);
	if (t != *str) {
	    *diffpos = i;
/*  	    sa_dprintf("intcmp: (%d) - (%d) = %d\n", t, *str, (t - *str)); */
	    return (t - *str);
	}
	txt++;
	str++;
    }
    *diffpos = len;
    return 0;
}


/*
 * binary search called from sa_find
 */
static SA_BSEARCH_RESULT
sa_bsearch(
    const SUFARY *ary,
    SA_BSEARCH_RESULT sbr,
    const int *key,
    int keylen
    )
{
    assert(0 <= sbr.left && sbr.right <= sa_get_array_size(ary));
    assert(sbr.left < sbr.right);

    sbr.state = FAIL;

    sa_dprintf("bsearch: start! [ %ld, %ld )\n", sbr.left, sbr.right);

    while (sbr.left < sbr.right) {  /* avoid negative number (usually "<=")*/
	int txt_skip = MIN(sbr.left_prefix_len, sbr.right_prefix_len);
	int key_skip = txt_skip - sbr.base_txt_skip;
	int diffpos;		/* 文字列比較で異なりのあった位置 */
 	SA_INDEX cur =  /* avoid overflow */
	    sbr.left / 2 + sbr.right / 2 + (1 & sbr.left & sbr.right);
	int result_of_cmp = cmp_sistr(
	    (unsigned char*)sa_aryidx2txtptr(ary, cur) + txt_skip,
	    key + key_skip,
	    &diffpos, keylen - key_skip);

	sa_dprintf("bsearch: txt_skip = %d, key_skip = %d, base_txt_skip = %d\n",
		(int)txt_skip, (int)key_skip, (int)sbr.base_txt_skip);
	sa_dprintf("bsearch: %ld = ( %ld + %ld ) / 2  cmp = %d\n",
		cur, sbr.left, sbr.right, result_of_cmp);

	if (result_of_cmp < 0) { /* text < key */
	    sbr.left = cur + 1;
	    sbr.left_prefix_len = txt_skip + diffpos;
	} else if (result_of_cmp > 0) {	/* text > key */
	    sbr.right = cur;
	    sbr.right_prefix_len = txt_skip + diffpos;
	} else {		/* text == key */
	    sbr.state = SUCCESS;
	    sbr.center = cur;
	    break;
	}
    }

    if (sbr.state == FAIL)
	sbr.center = sbr.left / 2 + sbr.right / 2 + (1 & sbr.left & sbr.right);

    sa_dprintf("bsearch: * %ld  [ %ld, %ld )\n",
	    sbr.center, sbr.left, sbr.right);
    
    return sbr;
}


/*
 * suffix array search
 *
 * example: searching for "fb" (keyword)
 * results: from l to r (represented by '#')
 *
 *                 abxxiabcdefgxyzpoop
 *   left          hipdabbbbbbbbbbxaba      right
 *    |            bbbcfffffffffffhllm        |
 *    +-----------------##########------------+
 *                      |        |
 *                      l        r
 */
SUF_RESULT
sa_find(
    const SUFARY *ary,
    SA_INDEX left,		/* result-range: left-inside */
    SA_INDEX right,		/* result-range: right-inside */
    const char *key,		/* 検索キー */
    int keylen,			/* 検索キーの文字列長 */
    int base_txt_skip		/* 検索対象文字列のスキップ数 */
    )
{
    static int keyary[KEYWORD_MAX_LENGTH];
    SA_BSEARCH_RESULT sbr, sbr_r, sbr_l;
    SUF_RESULT sr;
    int i;

    assert(0 <= left && right < ary->arraysize);
    assert(base_txt_skip >= 0);

    sa_dprintf("find: key = %.*s\n", keylen, key);
    for (i = 0; i < keylen; i++) {
	keyary[i] = (unsigned char)key[i];
	sa_dprintf("find: key[%d] = '%c', keyary[%d] = %d\n",
		i, key[i], i, keyary[i]);
    }

    sbr.left = left;
    sbr.right = right + 1;
    sbr.left_prefix_len = base_txt_skip;
    sbr.right_prefix_len = base_txt_skip;
    sbr.base_txt_skip = base_txt_skip;

    /* step 1: find a position mathces the key "fb" (represented by '*').
     *
     *                 abxxiabcdefgxyzpoop
     *   left          hipdabbbbbbbbbbxaba      right
     *    |            bbbcfffffffffffhllm        |
     *    +---------------------*-----------------++
     *    |                     |                  |
     *   lb                   center               rb
     */
    sbr = sa_bsearch(ary, sbr, keyary, keylen);
    if (sbr.state == FAIL) {
	sr.stat = FAIL;
	return sr;
    }

    /* step 2: find a right boundary (represented by '*').
     *         '%' means a seach area.
     *
     *                 abxxiabcdefgxyzpoop
     *   left          hipdabbbbbbbbbbxaba      right
     *    |            bbbcfffffffffffhllm        |
     *    +-------------+-------#-----*-+---------+
     *                  |       %%%%%%%%|
     *                  |       |       |
     *                 lb      lor      rb
     */
    keyary[keylen] = 256;
    sbr_r = sbr;
    sbr_r.left = sbr.center;
    sbr_r.left_prefix_len = keylen + base_txt_skip;
    sbr_r = sa_bsearch(ary, sbr_r, keyary, keylen + 1);

    /* step 3: find a left boundary (represented by '*').
     *
     *                 abxxiabcdefgxyzpoop
     *   left          hipdabbbbbbbbbbxaba      right
     *    |            bbbcfffffffffffhllm        |
     *    +-------------+---*---######+-----------+
     *                  %%%%%%%%%     |
     *                  |       |     |
     *                 lb      rol    rb
     */
    keyary[keylen] = -1;
    sbr_l = sbr;
    sbr_l.right = sbr.center + 1;
    sbr_l.right_prefix_len = keylen + base_txt_skip;
    sbr_l = sa_bsearch(ary, sbr_l, keyary, keylen + 1);

    sr.suf = (SUFARY *)ary;
    sr.left = sbr_l.center;
    sr.right = sbr_r.center - 1;
    sr.stat = SUCCESS;
    sa_dprintf("find: found [ %ld, %ld ]\n", sr.left, sr.right);
    return sr;
}

/*
 *   テキストファイルを覗くための関数群
 *   - arrayファイルのidx番目の要素を返す
 *   - textファイルのidx番目をさす文字列ポインタを返す
 *   - arrayファイルのidx番目の要素がさす文字列ポインタを返す
 */
SA_INDEX
sa_aryidx2txtidx(const SUFARY *ary, SA_INDEX idx)
{
#ifdef WORDS_BIGENDIAN
    return sa_get_array_ptr(ary)[idx];
#else
    SA_INDEX rv = sa_get_array_ptr(ary)[idx];
    sa_reverse_byte_order(&rv, sizeof(SA_INDEX));
    return rv;
#endif
}

char *
sa_txtidx2txtptr(const SUFARY *ary, SA_INDEX idx)
{
    return sa_get_text_ptr(ary) + idx;
}

char *
sa_aryidx2txtptr(const SUFARY *ary, SA_INDEX idx)
{
#ifdef WORDS_BIGENDIAN
    return sa_get_text_ptr(ary) + sa_get_array_ptr(ary)[idx];
#else
    SA_INDEX rv = sa_get_array_ptr(ary)[idx];
    sa_reverse_byte_order(&rv, sizeof(SA_INDEX));
    return sa_get_text_ptr(ary) + rv;
#endif
}

