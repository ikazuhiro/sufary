/*
 * $Id: file.c,v 1.21 2000/11/17 02:51:50 tatuo-y Exp $
 *
 * open/close files
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>

#include "sufary.h"
#include "util.h"
#include "config.h"


static SA_STAT sa_opentextfile(SUFARY *ary, const char *s);
static SA_STAT sa_openarrayfile(SUFARY *ary, const char *s);
static void sa_closetextfile(SUFARY *ary);
static void sa_closearrayfile(SUFARY *ary);


/*
 * access functions
 */
char *
sa_get_text_ptr(const SUFARY *ary)
{
    assert(ary != NULL);
    return ary->text;
}

SA_INDEX
sa_get_text_size(const SUFARY *ary)
{
    assert(ary != NULL);
    return ary->textsize;
}

SA_INDEX *
sa_get_array_ptr(const SUFARY *ary)
{
    assert(ary != NULL);
    return ary->array;
}

SA_INDEX
sa_get_array_size(const SUFARY *ary)
{
    assert(ary != NULL);
    return ary->arraysize;
}


/*
 *   指定されたテキストファイルとarrayファイルを開く。
 *   arrayファイル名をNULLに指定すれば、テキストファイル名に
 *   '.ary' を付加したものがarrayファイル名になる。
 */
SUFARY *
sa_open(const char *txt_fname, char *ary_fname)
{
    SUFARY *newary;
    char fname_tmp[SA_FILE_NAME_MAX];

    if (ary_fname == NULL || ary_fname[0] == '\0')
	ary_fname = sa_add_suffix_to_file_name(fname_tmp, txt_fname, "ary");

    newary = (SUFARY *)sa_calloc(sizeof(SUFARY), 1);
    if (newary == NULL) {
	sa_error_no = MEMORY_ALLOCATE_ERROR;
	return NULL;
    }

    if (sa_opentextfile(newary, txt_fname) == FAIL ||
	sa_openarrayfile(newary, ary_fname) == FAIL) {
	sa_close(newary);
	sa_free(newary);
	sa_error_no = FILE_OPEN_ERROR;
	return NULL;
    }

    return newary;
}


/*
 * open only a text file
 */
static SA_STAT
sa_opentextfile(SUFARY *ary, const char *filename)
{
    SA_MMAP *mms;
    assert(ary != NULL);
    assert(filename != NULL);

    mms = sa_open_mmap(filename, SA_MMAP_RO);
    if (mms == NULL) {
	sa_error_no = MMAP_ERROR;
	return FAIL;
    }

    ary->mmtxt = mms;

    ary->textsize = sa_get_mmap_size(mms);
    ary->text = sa_get_mmap_ptr(mms);

    return SUCCESS;
}


/*
 * open only an array file
 */
static SA_STAT
sa_openarrayfile(SUFARY *ary, const char *filename)
{
    SA_MMAP *mms;
    assert(ary != NULL);
    assert(filename != NULL);

    mms = sa_open_mmap(filename, SA_MMAP_RO);
    if (mms == NULL) {
	sa_error_no = MMAP_ERROR;
	return FAIL;
    }

    ary->mmary = mms;

    ary->arraysize = sa_get_mmap_size(mms) / sizeof(SA_INDEX);
    ary->array = (SA_INDEX*)sa_get_mmap_ptr(mms);

    return SUCCESS;
}


/*
 * close a text file and an array file
 */
void
sa_close(SUFARY *ary)
{
    assert(ary != NULL);
    sa_closetextfile(ary);
    sa_closearrayfile(ary);
    sa_free(ary);
    /* 無意味: ary = NULL; */
}


/*
 * close only a text file
 */
void
sa_closetextfile(SUFARY *ary)
{
    assert(ary != NULL);
    sa_close_mmap(ary->mmtxt);
}


/*
 * close only an array file
 */
void
sa_closearrayfile(SUFARY *ary)
{
    assert(ary != NULL);
    sa_close_mmap(ary->mmary);
}


/*
 * write a big endian ordering long int
 */
size_t
sa_fwrite(SA_INDEX idx, FILE *fd)
{
#ifndef WORDS_BIGENDIAN
    sa_reverse_byte_order(&idx, sizeof(SA_INDEX));
#endif
    return fwrite(&idx, 1, sizeof(SA_INDEX), fd);
}

