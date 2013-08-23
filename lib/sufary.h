/*
 * $Id: sufary.h,v 1.52 2002/01/22 23:29:15 tatuo-y Exp $
 *
 *  sufary.h --- SUFARY Library Header File
 *
 */

#ifndef _SUFARY_H_
#define _SUFARY_H_

#include <sys/types.h>
#include <stdarg.h>

#ifndef dp
#include <stdio.h>
#define dp(x) printf(#x " = %Lg\n", (long double)(x))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KEYWORD_MAX_LENGTH
#define KEYWORD_MAX_LENGTH 5000
#endif

#define SA_FILE_NAME_MAX 1000

typedef long SA_INDEX;


typedef enum sa_error_ {
    OTHER_ERROR = 0,
    MEMORY_ALLOCATE_ERROR = 1,
    FILE_OPEN_ERROR = 2,
    MMAP_ERROR = 3,
    KEYWORD_PARSING_ERROR = 4
} SA_ERROR;

extern int sa_error_no;
extern char *sa_error_str[];

#define sa_get_error_str ((char*)sa_error_str[sa_error_no])


typedef enum sa_stat_ {
    SUCCESS,
    FAIL
} SA_STAT;


typedef struct {
    void *mmtxt;		/* SA_MMAP *mmtxt; */
    void *mmary;		/* SA_MMAP *mmary; */
    char *text;			/* テキストファイルのマップアドレス */
    SA_INDEX *array;
    SA_INDEX textsize;		/* テキストファイルのサイズ */
    SA_INDEX arraysize;		/* Array の大きさ */
} SUFARY;

typedef struct {
    SUFARY *suf;
    SA_INDEX left;		/* 検索範囲の左端(範囲の内側を指す) */
    SA_INDEX right;		/* 検索範囲の右端(範囲の内側を指す) */
    SA_STAT stat;
} SUF_RESULT;


typedef enum sa_mki_mode_ {
    SA_MISIYOU1 = 1,
    SA_MISIYOU2 = 2,
    SA_IP_00_7F_AND_EUC = 4,
    SA_CUT_TOP = 8,
    SA_IP_00_7F = 16,
    SA_IP_80_FF = 32,
    SA_VERBOSE = 64
} SA_MAKE_INDEX_MODE;


/* 検索結果リスト(for regex) */
typedef struct SA_RESULT_LIST SA_RESULT_LIST;
struct SA_RESULT_LIST {
    SA_INDEX value;
    int len;
    SA_RESULT_LIST *next;
};


typedef struct {
    char *ptr;
    size_t len;
} SA_STRING;

#define str2sastr(c, s) {s.ptr = (char *)c; s.len = strlen(c);}


typedef struct {
    SA_INDEX cur;
    SA_INDEX skip;
} SA_INDEX_POINT;		/* 未使用... */



/* in search.c */
extern SUF_RESULT sa_find(const SUFARY *ary, SA_INDEX left, SA_INDEX right,
			  const char *keyword, int keyword_length,
			  int base_txt_skip);
extern char *sa_aryidx2txtptr(const SUFARY *ary, SA_INDEX idx);
extern SA_INDEX sa_aryidx2txtidx(const SUFARY *ary, SA_INDEX idx);
extern char *sa_txtidx2txtptr(const SUFARY *ary, SA_INDEX idx);
extern void sa_set_debug_mode(int i);


/* in region.c */
extern size_t sa_copy_region(char *dst, const SA_STRING src);
extern char *sa_dup_region(const SA_STRING src);
extern SA_STRING sa_seek_context_lines(const SUFARY *ary, char *pos,
				       int bkwrd, int frwrd);
extern char *sa_seek_bol(char *pos, char *bof);
extern char *sa_seek_eol(char *pos, char *eof);
extern SA_STRING sa_seek_context_region(const SUFARY *ary, char *pos,
					const SA_STRING begin_tag,
					const SA_STRING end_tag);
extern char *sa_seek_pattern_backward(char *pos, char *bof,
				      const SA_STRING pattern);
extern char *sa_seek_pattern_forward(char *pos, char *eof,
				     const SA_STRING pattern);
extern SA_INDEX sa_position_in_string(const SUFARY *ary, SA_STRING sstr,
				      SA_INDEX pos);


/* in regex.c */
extern SA_RESULT_LIST *sa_regex(const SUFARY *ary,
				SA_INDEX left, SA_INDEX right,
				const char *keyword, int keyword_length);
extern SA_RESULT_LIST *sa_ignore_case(const SUFARY *ary,
				      SA_INDEX left, SA_INDEX right,
				      const char *keyword, int keyword_length);
extern void sa_free_result_list(SA_RESULT_LIST *listp);


/* in file.c */
extern SUFARY *sa_open(const char *text_file_name, char *array_file_name);
extern void sa_close(SUFARY *ary);

extern char *sa_get_text_ptr(const SUFARY *ary);
extern SA_INDEX sa_get_text_size(const SUFARY *ary);
extern SA_INDEX* sa_get_array_ptr(const SUFARY *ary);
extern SA_INDEX sa_get_array_size(const SUFARY *ary);

extern size_t sa_fwrite(SA_INDEX idx, FILE *fd);


/* in make-index.c */
extern void sa_set_make_index_mode(int mode);
extern void sa_set_make_index_memory_size(int mega_byte);

extern SA_STAT sa_sort_index(const char *txt_fname, char *ary_fname);

extern SA_STAT sa_write_index(const char *in_fname, char *ary_fname,
			      SA_INDEX (*get_next_ip_func)(SA_STRING, SA_INDEX)
);

extern int sa_is_sorted(const SUFARY *ary);
extern void sa_dump_all_suffixes(const SUFARY *ary);

extern SA_INDEX sa_get_next_ip_char(SA_STRING sstr, SA_INDEX ip);
extern SA_INDEX sa_get_next_ip_line(SA_STRING sstr, SA_INDEX ip);
extern SA_INDEX sa_get_next_ip_word(SA_STRING sstr, SA_INDEX ip);
extern SA_INDEX sa_get_next_ip_byte(SA_STRING sstr, SA_INDEX ip);

extern void sa_set_ignored_chars(char *p);
extern char *sa_get_ignored_chars(void);
extern void sa_set_my_chars(char *p);
extern char *sa_get_my_chars(void);

extern SA_INDEX sa_get_next_ip_after_delimiter(SA_STRING sstr, SA_INDEX ip);
extern void sa_set_my_delimiters(char *p);
extern char *sa_get_my_delimiters(void);

//extern SA_INDEX (*sa_get_next_ip_hook)(SA_STRING, SA_INDEX);


#ifdef __cplusplus
}
#endif

#endif				/* _SUFARY_H_ */
