/*
 * $Id: test.c,v 1.27 2000/11/28 07:56:42 tatuo-y Exp $
 *
 * test
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sufary.h"
#include "did.h"

#define JOUGEN(x,m) {(x) = ((off_t)(x) > (off_t)(m)) ? (m) : (x);}


SUFARY *ary;
SUF_RESULT sr;
DID *did;
DID_RESULT dr;

char fname[] = "../ChangeLog";
char aryfn[] = "../ChangeLog.ary";
char didfn[] = "../ChangeLog.did";


static SA_INDEX
get_next_ip_test(SA_STRING sstr, SA_INDEX ip)
{
    assert(-1 <= ip && ip < (SA_INDEX)sstr.len);
    ip++;

    while (ip < (SA_INDEX)sstr.len - 15) {
	if (strncmp(sstr.ptr + ip, "* configure.in:", 15) == 0)
	    break;
        ip++;
    }

    if (ip < (SA_INDEX)sstr.len - 15)
        return ip;
    else
        return sstr.len;
}


static void
search_both_ends()
{
    SA_INDEX right_most;
    SA_INDEX len;
    SA_STRING sstr;
    SA_STRING tag1, tag2;
    
    str2sastr("* ", tag1);
    str2sastr("\t*", tag2);

    assert((ary = sa_open(fname, NULL)) != NULL);

    len = sa_get_text_size(ary) - sa_aryidx2txtidx(ary, 0);
    JOUGEN(len, 20);
    sr = sa_find(ary, 0, sa_get_array_size(ary) - 1, 
		 sa_aryidx2txtptr(ary, 0), len, 0);
    assert(sr.stat == SUCCESS);
    printf("Left most ... Ok!\n");	
    sstr = sa_seek_context_lines(ary, sa_aryidx2txtptr(ary, sr.left), 0, 0);
    if (sstr.ptr[sstr.len - 1] == '\n')
	sstr.len--;
    printf(" LINE   >>>>>> %.*s <<<<<<\n", (int)sstr.len, sstr.ptr);
    sstr = sa_seek_context_region(ary, sa_aryidx2txtptr(ary, sr.left),
				  tag1, tag2);
    printf(" REGION >>>>>>\n %.*s\n <<<<<<\n", (int)sstr.len, sstr.ptr);


    assert((right_most = sa_get_array_size(ary)) > 0);

    len = sa_get_text_size(ary) - sa_aryidx2txtidx(ary, right_most - 1);
    JOUGEN(len, 20);
    sr = sa_find(ary, 0, sa_get_array_size(ary) - 1, 
		 sa_aryidx2txtptr(ary, right_most - 1), len, 0);
    assert(sr.stat == SUCCESS);
    printf("Right most ... Ok!\n");
    sstr = sa_seek_context_lines(ary, sa_aryidx2txtptr(ary, sr.left), 0, 0);
    if (sstr.ptr[sstr.len - 1] == '\n')
	sstr.len--;
    printf(" LINE   >>>>>> %.*s <<<<<<\n", (int)sstr.len, sstr.ptr);
    sstr = sa_seek_context_region(ary, sa_aryidx2txtptr(ary, sr.left),
				  tag1, tag2);
    printf(" REGION >>>>>>\n %.*s\n <<<<<<\n", (int)sstr.len, sstr.ptr);

    sa_close(ary);
}


static void
make_did()
{
    assert(sa_make_did(fname, NULL, didfn, "* ", NULL) == SUCCESS);
    assert((did = sa_open_did(didfn)) != NULL);
    assert(sa_get_did_size(did) > 0);
    printf("DID_SIZE: %ld\n", sa_get_did_size(did));
    sa_close_did(did);
}


static void
make_index(SA_INDEX (*get_next_ip)(SA_STRING, SA_INDEX))
{
    assert(get_next_ip != NULL);
    printf(" - Write index\n");
    assert(sa_write_index(fname, NULL, get_next_ip) != FAIL);
    printf(" - Sort index\n");
    assert(sa_sort_index(fname, NULL) != FAIL);
    assert((ary = sa_open(fname, aryfn)) != NULL);
    assert((ary = sa_open(fname, NULL)) != NULL);
    printf(" - Check index\n");
    assert(sa_is_sorted(ary) == 1);
    printf(" - array size = %ld\n", (long) sa_get_array_size(ary));
    sa_close(ary);
}


static void
search_random_key()
{
    char *pos;
    SA_INDEX len;

    pos = sa_aryidx2txtptr(ary, sa_get_array_size(ary) / 3);
    len = sa_get_text_ptr(ary) + sa_get_text_size(ary) - pos;
    JOUGEN(len, 4);

    sr = sa_find(ary, 0, sa_get_array_size(ary) - 1, pos, len, 0);
    assert(sr.stat == SUCCESS);

    printf("KEYWORD: \"%.*s\"\n", (int)len, pos);
    printf("RESULT: Left %ld, Right %ld\n", (long)sr.left, (long)sr.right);
}


static void
get_did()
{
    SA_INDEX i;
    int cnt = 0;

    assert((did = sa_open_did(didfn)) != NULL);
    for (i = sr.left; i <= sr.right; i++) {
	if (cnt++ >= 3)
 	  break;
	dr = sa_didsearch(did, sa_aryidx2txtidx(ary, i));
	assert(dr.stat == SUCCESS);
	printf("REGION %d: no. %ld, start %ld, size %ld\n",
	       cnt, dr.no, dr.start, dr.size);
	printf(" >>>>>\n %.*s\n <<<<<<\n", (int)dr.size,
	       sa_txtidx2txtptr(ary, dr.start));
    }
    sa_close_did(did);
}    


static void
regex_seach(char *pattern)
{
    SA_RESULT_LIST *ll, *tmp;
    SA_STRING sstr;
    int cnt = 0;

    printf("PATTERN: \"%s\"\n", pattern);
    ll = sa_regex(ary, 0, sa_get_array_size(ary) - 1,
		  pattern, strlen(pattern));
    assert(ll != NULL);
    tmp = ll;

    for (; ll != NULL; ll = ll->next) {
	if (cnt++ >= 16)
 	  break;

	sstr = sa_seek_context_lines(ary, sa_txtidx2txtptr(ary, ll->value),
				     0, 0);
	if (sstr.ptr[sstr.len - 1] == '\n')
	  sstr.len--;
	printf(" %d >>>>>> %.*s <<<<<<\n", cnt, (int)sstr.len, sstr.ptr);

    }
    sa_free_result_list(tmp);
}


static void
ignore_case_seach(char *pattern)
{
    SA_RESULT_LIST *ll, *tmp;
    SA_STRING sstr;
    int cnt = 0;

    printf("PATTERN: \"%s\"\n", pattern);
    ll = sa_ignore_case(ary, 0, sa_get_array_size(ary) - 1,
		  pattern, strlen(pattern));
    assert(ll != NULL);
    tmp = ll;

    for (; ll != NULL; ll = ll->next) {
	if (cnt++ >= 16)
 	  break;

	sstr = sa_seek_context_lines(ary, sa_txtidx2txtptr(ary, ll->value),
				     0, 0);
	if (sstr.ptr[sstr.len - 1] == '\n')
	  sstr.len--;
	printf(" %d >>>>>> %.*s <<<<<<\n", cnt, (int)sstr.len, sstr.ptr);

    }
    sa_free_result_list(tmp);
}


int main(int argc, char *argv[])
{
    printf("\n===\n=== make index (character-based)\n===\n");
    /* sa_set_make_index_mode(SA_EUC_2); */
    make_index(sa_get_next_ip_char);

    printf("\n===\n=== search both ends \n===\n");    
    search_both_ends();

    printf("\n===\n=== make index (line-based)\n===\n");
    make_index(sa_get_next_ip_line);

    printf("\n===\n=== search both ends \n===\n");    
    search_both_ends();

    printf("\n===\n=== make index (word-based)\n===\n");
    make_index(sa_get_next_ip_word);

    printf("\n===\n=== search both ends \n===\n");    
    search_both_ends();

    printf("\n===\n=== make did \n===\n");    
    make_did();

    assert((ary = sa_open(fname, NULL)) != NULL);

    printf("\n===\n=== get did \n===\n");    
    search_random_key();
    get_did();

    printf("\n\n===\n=== search regex \n===\n");
    regex_seach("sa?[_a-z]s[^aei]");
    regex_seach("(suf|mk)ary.[ch]");

    printf("\n\n===\n=== ignore case \n===\n");
    ignore_case_seach("speed up");

    sa_close(ary);

    printf("\n\n===\n=== make index by using hook \n===\n");
    make_index(get_next_ip_test);
    printf(" - Dump all suffixes\n");
    assert((ary = sa_open(fname, NULL)) != NULL);
    sa_dump_all_suffixes(ary);
    sa_close(ary);

    printf("\n\n===\n=== done \n===\n");
    printf("No problem! (maybe)\n\n");

    return EXIT_SUCCESS;
}
