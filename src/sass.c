/*
 * $Id: sass.c,v 1.45 2000/11/17 02:51:50 tatuo-y Exp $
 *
 * sass --- Suffix Array Simple Search
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "sufary.h"
#include "util.h"
#include "getopt.h"

#ifdef WIN32					/* FIXME */
#define HAVE_CONFIG_H
#endif

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#define TAG_LEN_MAX 1000
#define INPUT_STR_LEN_MAX 10000
#define FILE_NUM_MAX 256

int regex_mode = 0;
int stdin_mode = 0;
int ignore_case_mode = 0;
int before_context = 0;
int after_context = 0;
int keyword_emphasis_mode = 0;
int tag_mode = 0;
int array_file_mode = 0;
int uniq_mode = 0;
int sort_mode = 0;		/* sort by the position in the file */
int count_only_mode = 0;
char tags[2][TAG_LEN_MAX];	/* for tags option */
char array_file_name[FILE_NUM_MAX]; /* for array-file option */
SA_INDEX *rgnary;		/* for sort_mode */
SA_INDEX rgnary_idx = 0;
SA_INDEX size_of_rgnary;

static void search_all_files(char *fnames[], int num_of_files,
			     const char *key);
static void search_for_the_given_pattern(const SUFARY *ary, const char *key,
					 const char *fname);
static void show_usage(void);
static void show_mini_usage(void);
static int parse_options(int, char**);
static int ck_atoi(char const *, int *);

/*
 * Command line options.
 */
static const char *short_options = "0123456789hirsA:B:C::kvdt:a:Suc";
static struct option long_options[] = {
    {"help",             no_argument,       NULL, 'h'},
    {"ignore-case",      no_argument,       NULL, 'i'},
    {"regex",            no_argument,       NULL, 'r'},
    {"stdin",            no_argument,       NULL, 's'},
    {"keyword-emphasis", no_argument,       NULL, 'k'},
    {"after-context",    required_argument, NULL, 'A'},
    {"before-context",   required_argument, NULL, 'B'},
    {"context",          optional_argument, NULL, 'C'},
    {"tags",             required_argument, NULL, 't'},
    {"array-file",       required_argument, NULL, 'a'},
    {"debug",            no_argument,       NULL, 'd'},
    {"version",          no_argument,       NULL, 'v'},
    {"sort",             no_argument,       NULL, 'S'},
    {"uniq",             no_argument,       NULL, 'u'},
    {"count",            no_argument,       NULL, 'c'},
    {"0",                no_argument,       NULL, '0'},
    {"1",                no_argument,       NULL, '1'},
    {"2",                no_argument,       NULL, '2'},
    {"3",                no_argument,       NULL, '3'},
    {"4",                no_argument,       NULL, '4'},
    {"5",                no_argument,       NULL, '5'},
    {"6",                no_argument,       NULL, '6'},
    {"7",                no_argument,       NULL, '7'},
    {"8",                no_argument,       NULL, '8'},
    {"9",                no_argument,       NULL, '9'},
    {NULL, 0, NULL, 0}
};


int
main(int argc, char *argv[])
{
    int argc_offset;
    int num_of_files;
    char* key = NULL;
    argc_offset = parse_options(argc, argv);

    if (argc_offset == argc) {
	show_mini_usage();
	exit(EXIT_FAILURE);
    }

    if (!stdin_mode) {
	key = argv[argc_offset];
	argc_offset++;
    }

    num_of_files = argc - argc_offset;
    if (num_of_files < 1) {
	show_mini_usage();
	exit(EXIT_FAILURE);
    }

    if (stdin_mode) {
	char cmd[INPUT_STR_LEN_MAX];
	while (fgets(cmd, (int)sizeof(cmd), stdin)) {
	    cmd[strlen(cmd) - 1] = '\0'; /* delete '\n' */
	    search_all_files(argv + argc_offset, num_of_files, cmd);
	}
    } else {
	search_all_files(argv + argc_offset, num_of_files, key);
    }

    assert(sa_memory_leak_check());
    return 0;
}


static void
search_all_files(char *fnames[], int num_of_files, const char *key)
{
    SUFARY *ary[FILE_NUM_MAX];
    int i;

    if (array_file_name) {
	if ((ary[0] = sa_open(fnames[0], array_file_name)) == NULL) {
	    fprintf(stderr, "%s or %s: File open error\n",
		    fnames[0], array_file_name);
	    exit(EXIT_FAILURE);
	}
	search_for_the_given_pattern(ary[0], key, NULL);
	sa_close(ary[0]);
	return;
    }

    for (i = 0; i < num_of_files; i++) {
	if ((ary[i] = sa_open(fnames[i], NULL)) == NULL) {
	    fprintf(stderr, "%s: File open error\n", fnames[i]);
	    exit(EXIT_FAILURE);
	}
	if (num_of_files == 1)
	    search_for_the_given_pattern(ary[i], key, NULL);
	else
	    search_for_the_given_pattern(ary[i], key, fnames[i]);
	sa_close(ary[i]);
    }
}


/*
 * Get region information and store tmporary array (for sorting).
 */
static SA_STRING
get_one_region(const SUFARY *ary, SA_INDEX pos)
{
    SA_STRING sstr;
    char *cur;
    cur = sa_txtidx2txtptr(ary, pos);
    if (tag_mode) {
	SA_STRING t1, t2;
	str2sastr(tags[0], t1);
	str2sastr(tags[1], t2);
	sstr = sa_seek_context_region(ary, cur, t1, t2);
    } else {
	sstr = sa_seek_context_lines(ary, cur, before_context, after_context);
	if (sstr.ptr[sstr.len - 1] == '\n')
	    sstr.len--;
    }

    return sstr;
}


static void 
print_one_region(const SUFARY *ary, SA_STRING sstr, SA_INDEX pos, 
		 const char *fname, int key_len)
{
    char* s;
    SA_INDEX rgn_len;
    s = sstr.ptr;
    rgn_len = sstr.len;

    if (fname != NULL)
	printf("%s:", fname);

    if (keyword_emphasis_mode) {
	int pos_in_rgn;
	pos_in_rgn = sa_position_in_string(ary, sstr, pos);
	printf("%.*s", pos_in_rgn, s);
	printf("\x1b[7m%.*s\x1b[0m", key_len, s + pos_in_rgn);
	printf("%.*s\n", (int)(rgn_len - (pos_in_rgn + key_len)),
	       s + pos_in_rgn + key_len);
    } else {
	printf("%.*s\n", (int)rgn_len, s);
    }

    if (after_context >= 1 || before_context >= 1 || tag_mode) {
	printf("--\n");
    }
}


static int
index_cmp(const void *data1, const void *data2)
{
    SA_INDEX *a = (SA_INDEX *)data1;
    SA_INDEX *b = (SA_INDEX *)data2;
    return *a - *b;
}


static void
print_sorted_result(const SUFARY *ary, const char *fname)
{
    int i;
    char *pre_region_tail;
    pre_region_tail = sa_get_text_ptr(ary);
    qsort(rgnary, size_of_rgnary, sizeof(SA_INDEX), index_cmp); 
    for (i = 0; i < size_of_rgnary; i++) {
	if (sa_txtidx2txtptr(ary, rgnary[i]) >= pre_region_tail
	    || uniq_mode == 0) {
	    SA_STRING sstr;
	    sstr = get_one_region(ary, rgnary[i]);
	    print_one_region(ary, sstr, rgnary[i], fname, 2);
	    pre_region_tail = sstr.ptr + sstr.len;
	}
    }
}


/*
 * Find a keyword and print regions.
 */
static void
search_for_the_given_pattern(const SUFARY *ary, const char *key,
			     const char *fname)
{
    if (strlen(key) == 0)
	return;

    if (regex_mode || ignore_case_mode) {
	SA_RESULT_LIST *ll, *tmp;

	if (ignore_case_mode)
 	    tmp = sa_ignore_case(ary, 0, ary->arraysize - 1, key, strlen(key));
	else
	    tmp = sa_regex(ary, 0, ary->arraysize - 1, key, strlen(key));

	size_of_rgnary = 0;

	if (count_only_mode) {
	    for (ll = tmp; ll != NULL; ll = ll->next)
		size_of_rgnary++;
	    if (fname != NULL)
		printf("%s:", fname);
	    printf("%ld\n", (long)size_of_rgnary);
	    return;
	}

	if (sort_mode) {
	    int i = 0;
	    for (ll = tmp; ll != NULL; ll = ll->next)
		size_of_rgnary++;
            rgnary = sa_malloc(sizeof(SA_INDEX) * size_of_rgnary);
	    assert(rgnary != NULL && "too many results!");
	    for (ll = tmp; ll != NULL; ll = ll->next)
		rgnary[i++] = ll->value;
	} else {
	    for (ll = tmp; ll != NULL; ll = ll->next) {
		long pos = ll->value;
		SA_STRING sstr;
		sstr = get_one_region(ary, pos);
		print_one_region(ary, sstr, pos, fname, ll->len);
	    }
	}

        sa_free_result_list(tmp);
	
    } else {
	SUF_RESULT sr;
	sr = sa_find(ary, 0, ary->arraysize - 1, (char *)key, strlen(key), 0);

	if (sr.stat == SUCCESS) {
	    SA_INDEX ai;
	    if (count_only_mode) {
		if (fname != NULL)
		    printf("%s:", fname);
		printf("%ld\n", (long)(sr.right - sr.left + 1));
		return;
	    }

	    if (sort_mode) {
		size_of_rgnary = sr.right - sr.left + 1;
		rgnary = sa_malloc(sizeof(SA_INDEX) * size_of_rgnary);
		assert(rgnary != NULL && "too many results!");
		for (ai = sr.left; ai <= sr.right; ai++)
		    rgnary[ai - sr.left] = sa_aryidx2txtidx(ary, ai);
	    } else {
		for (ai = sr.left; ai <= sr.right; ai++) {
		    SA_INDEX pos = sa_aryidx2txtidx(ary, ai);
		    SA_STRING sstr;
		    sstr = get_one_region(ary, pos);
		    print_one_region(ary, sstr, pos, fname, strlen(key));
		}
	    }
	} else {
	    if (count_only_mode) {
		if (fname != NULL)
		    printf("%s:", fname);
		printf("0\n");
		return;
	    }
	}
    }

    if (sort_mode && rgnary) {
	print_sorted_result(ary, fname);
	rgnary_idx = 0;
	sa_free(rgnary);
    }
}


/*
 * Parse tagstring: "<a>,</a>" --> "<a>" and "</a>"
 */
static void
parse_tags(const char *tgstr)
{
    int i, j;
    int tag_id = 0;
    for (i = 0, j = 0; i <= (int)strlen(tgstr); i++, j++) {
	if (tgstr[i] == '\\' && i < (int)strlen(tgstr) && tgstr[i + 1] == 'n') {
	    tags[tag_id][j] = '\n';
	    i++;
	} else if (tgstr[i] == ',') {
	    tags[tag_id][j] = '\0';
	    tag_id = 1;
	    j = -1;
	} else {
	    tags[tag_id][j] = tgstr[i];
	}
    }
/*     printf("[%s] -> [%s] [%s]\n", tgstr, tags[0], tags[1]); */
}


static void
show_usage(void)
{
    printf("\
sass %s, simple search program of SUFARY.\n\n\
Usage: sass [options] <pattern> <file>...\n\
  -r, --regex               use regex for the pattern.\n\
  -i, --ignore-case         ignore case.\n\
  -s, --stdin               read the pattern from stdin.\n\
  -k, --keyword-emphasis    emphasize keyword in results\n\
  -A, --after-context=NUM   print NUM lines of trailing context\n\
  -B, --before-context=NUM  print NUM lines of leading context\n\
  -C, --context[=NUM]       print NUM (default 2) lines of output context\n\
                            unless overriden by -A or -B\n\
  -NUM                      same as --context=NUM\n\
  -t, --tags=TAG1,TAG2      print text regions (from TAG1 to TAG2).\n\
                            example: -t '<doc>,</doc>', -t '\\n\\n,\\n\\n'\n\
  -S, --sort                sort results by the position in the file.\n\
  -u, --uniq                similar to --sort, but\n\
                            only output the first of an equal sequence.\n\
  -c, --count               only print a count of matches.\n\
  -a, --array-file=FILE     specify an array file.\n\
  -v, --version             show the version of SUFARY and exit.\n\
  -d, --debug               show the debug information.\n\n\
Notes:\n\
  You should prepare an array file for each target file.\n\
  An array file name must be its original file name followed by \".ary\"\n\
  or must be specified by '-a' option.\n",
	   VERSION);
}


static void
show_mini_usage(void)
{
    printf("\
Usage: sass [options] <pattern> <file ...>\n\
Try `sass --help' for more information.\n");
}


static int
parse_options(int argc, char **argv)
{
    int digit_args_val = 0;

    while (1) {
        int ch = getopt_long(argc, argv, short_options, long_options, NULL);
        if (ch == EOF) {
            break;
	}
	switch (ch) {
	case 'h':
	    show_usage();
	    exit(EXIT_FAILURE);
	    break;
	case 'i':
	    ignore_case_mode = 1;
	    break;
	case 'r':
	    regex_mode = 1;
	    break;
	case 's':
	    stdin_mode = 1;
	    break;
	case 'A':
	    if (ck_atoi(optarg, &after_context)) {
		fprintf(stderr, "invalid context length argument\n");
		exit(EXIT_FAILURE);
	    }
	    break;
	case 'B':
	    if (ck_atoi(optarg, &before_context)) {
		fprintf(stderr, "invalid context length argument\n");
		exit(EXIT_FAILURE);
	    }
	    break;
	case 'C':
	    if (optarg) {
		if (ck_atoi(optarg, &before_context)) {
		    fprintf(stderr, "invalid context length argument\n");
		    exit(EXIT_FAILURE);
		}
		after_context  = before_context;
	    } else {
		before_context = 2;
		after_context  = 2;
	    }

	    break;
	case 'd':
	    sa_set_debug_mode(1);
	    break;
	case 't':
	    tag_mode = 1;
	    parse_tags(optarg);
	    break;
	case 'a':
	    array_file_mode = 1;
	    strcpy(array_file_name, optarg);
	    break;
	case 'k':
	    keyword_emphasis_mode = 1;
	    break;
	case 'u':
	    uniq_mode = sort_mode = 1;
	    break;
	case 'S':
	    sort_mode = 1;
	    break;
	case 'c':
	    count_only_mode = 1;
	    break;
	case 'v':
            printf("sass of SUFARY %s\n", VERSION);
	    printf("Copyright (C) 1997-1999 YAMASHITA Tatuo All rights reserved.\n");
            exit(EXIT_FAILURE);
	    break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    digit_args_val = 10 * digit_args_val + ch - '0';
	    before_context = digit_args_val;
	    after_context  = digit_args_val;
	    break;
	default:
	    show_mini_usage();
	    exit(EXIT_FAILURE);
	}
    }
    return optind;
}

/* Imported from GNU grep-2.3 [1999-11-08] by satoru-t */

/* Convert STR to a positive integer, storing the result in *OUT.
   If STR is not a valid integer, return -1 (otherwise 0). */
static int
ck_atoi (str, out)
     char const *str;
     int *out;
{
  char const *p;
  for (p = str; *p; p++)
    if (*p < '0' || *p > '9')
      return -1;

  *out = atoi (optarg);
  return 0;
}
