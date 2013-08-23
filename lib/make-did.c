/*
 * $Id: make-did.c,v 1.3 2000/11/17 02:51:50 tatuo-y Exp $
 *
 * make did
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sufary.h"
#include "util.h"
#include "config.h"
#include "did.h"


/*
 * Compare SA_INDEX-es.
 */
static int
longcomp(const SA_INDEX *i, const SA_INDEX *j)
{
    return *i - *j;
}


/*
 * Make a doc-id file.
 */
SA_STAT sa_make_did(const char *txt_fname, char *ary_fname,
		    char* did_fname,
		    const char* tag1, const char* tag2)
{
    SUFARY* ary;
    SUF_RESULT sr_tag1;
    SA_INDEX i;
    SA_INDEX num;
    SA_INDEX *rslt;
    FILE *of;

    char fname_tmp[SA_FILE_NAME_MAX];
    assert(txt_fname != NULL);
    assert(tag1 != NULL);


    if (did_fname == NULL || did_fname[0] == '\0')
	did_fname = sa_add_suffix_to_file_name(fname_tmp, txt_fname, "did");

    if ((ary = sa_open(txt_fname, ary_fname)) == NULL) {
	sa_error_no = FILE_OPEN_ERROR;
	return FAIL;
    }

    sr_tag1 = sa_find(ary, 0, ary->arraysize - 1, tag1, strlen(tag1), 0);
    if (sr_tag1.stat == FAIL) {
	printf("pattern \"%s\" can not be found.\n", tag1);
	sa_close(ary);
	return FAIL;
    }

    num = sr_tag1.right - sr_tag1.left + 1;

    rslt = sa_malloc(num * 2 * sizeof(SA_INDEX));
	
/*
    if (m_mki_mode & SA_VERBOSE)
	printf("Number of Documents = %ld\n", (long)num);
*/

    for (i = 0; i < num; i++)
	rslt[i] = sa_aryidx2txtidx(ary, sr_tag1.left + i);

/* for (i = 0; i < num; i++) printf("p %ld\n", rslt[i]); */

    if (tag2 != NULL && tag2[0] != '\0') {
	/* ドキュメント終了タグが指定されている場合 */
	int len_tag2 = strlen(tag2);
	SUF_RESULT sr_tag2;

	sr_tag2 = sa_find(ary, 0, ary->arraysize - 1, tag2, strlen(tag2), 0);
	if (sr_tag2.stat == FAIL) { /* ドキュメント開始タグ */
	    printf("pattern \"%s\" can not be found.\n", tag2);
	    sa_free(rslt);
	    sa_close(ary);
	    return FAIL;
	}

	if(num != sr_tag2.right - sr_tag2.left + 1) {
	    printf("number of \"%s\" != number of \"%s\"\n", tag1, tag2);
	    sa_free(rslt);
	    sa_close(ary);
	    return FAIL;
	}

	for (i = 0; i < num; i++)
	    rslt[num + i] = sa_aryidx2txtidx(ary, sr_tag2.left + i) + len_tag2;

	qsort(rslt, (size_t)(num * 2), sizeof(SA_INDEX),
	      (int(*)(const void*, const void*))longcomp);

    } else {
	/* ドキュメント終了タグ ＝ 開始タグ */

	qsort(rslt, (size_t)num, sizeof(SA_INDEX),
	      (int(*)(const void*, const void*))longcomp);

	rslt[num * 2 - 1] = sa_get_text_size(ary);
	for (i = num * 2 - 2; i > 0; i -= 2)
	    rslt[i] = rslt[i - 1] = rslt[i / 2];
    }

    /* for (i = 0; i < num * 2; i++) printf("a %ld\n", rslt[i]); */

    if ((of = fopen(did_fname, "wb")) == NULL) {
	sa_error_no = FILE_OPEN_ERROR;
	sa_free(rslt);
	sa_close(ary);
	return FAIL;
    }

    for (i = 0; i < num * 2; i++)
	sa_fwrite(rslt[i], of);

    sa_free(rslt);
    sa_close(ary);
    fclose(of);

    return SUCCESS;
}
