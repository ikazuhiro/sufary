/*
 * $Id: mkary.c,v 1.23 2000/11/28 07:56:43 tatuo-y Exp $
 *
 * mkary --- making suffix array index
 *
 * Reference: [1] Kenneth W. Church, NLPRS '95  Invited Lecture
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "sufary.h"
#include "config.h"

#define FILE_NAME_LEN 1000
#define MODE_ON 1
#define MODE_OFF 0

static void usage(void);


/*
 * global variables  
 */
static char *progname; /* program name */
static int no_sort_mode = MODE_OFF; /* ソートしないモード */
static int sort_only_mode = MODE_OFF; /* ソートしかしないモード */
static int sort_check_mode = MODE_OFF; /* ソートされてるか最後にチェック */
static int dump_mode = MODE_OFF;
static int bunkatu_sort_mode = MODE_OFF; /* 990219 */
static int block_size_mega; /* 990219 分割ブロック数 */


/*
 * print a message (using <stdarg.h>)
 */
void print_msg(int ok, char *fmt, ...)
{
    va_list args;

    if (!ok)
        return;
    fflush(stdout);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}


/*
 * main
 */
int main(int argc, char **argv)
{
    char txt_fname[FILE_NAME_LEN];	/* 入力ファイル名 */
    char ary_fname[FILE_NAME_LEN];	/* 出力先ファイル名 */

    int modemode = 0;

    SA_INDEX (*get_next_ip)(SA_STRING, SA_INDEX) = sa_get_next_ip_char;

    txt_fname[0] = '\0';
    ary_fname[0] = '\0';

    progname = argv[0];		/* プログラム名 */

    modemode |= SA_VERBOSE;


    if (argc <= 1)
	usage();

    while (argc > 1) {
	if (argv[1][0] == '-') {
	    char op = argv[1][1];
	    if (op == '-')	/* long option name: --char */
		op = argv[1][2];
	    switch (op) {
	    case 'o':		/* 出力ファイル名の指定 */
		if (argc == 2){	/* 引数がないと受け付けない */
		    fprintf(stderr,"-o <filename> --- 出力ファイル名を指定\n");
		    exit(EXIT_FAILURE);
		}
		strcpy(ary_fname,argv[2]);
		argc--;
		argv++;
		break;
	    case 'l':		/* 一行毎にインデックスを作る */
		get_next_ip = sa_get_next_ip_line;
		break;
	    case 'w':		/* 一語毎にインデックスを作る */
		get_next_ip = sa_get_next_ip_word;
		break;
	    case 'c':		/* 一文字毎にインデックスを作る（デフォルト）*/
		sa_set_my_chars(NULL);
		get_next_ip = sa_get_next_ip_char;
		break;
	    case 'b':		/* 2バイト一文字処理を行なわない */
		get_next_ip = sa_get_next_ip_byte;
		break;
	    case 'J':		/* 日本語モード */
		modemode |= SA_IP_80_FF;
		sa_set_my_chars("<");
		break;
	    case 'q':		/* quiet mode */
		modemode &= ~SA_VERBOSE;
		break;
	    case 'v':		/* verbose mode */
		modemode |= SA_VERBOSE;
		break;
	    case 't':
		modemode |= SA_CUT_TOP;
		break;
	    case 'n':		/* -ns ソートしないモード */
		if (argv[1][2] == 's')
		    no_sort_mode = MODE_ON;
		break;
	    case 's':		/* -so ソートしかしないモード */
		if (argv[1][2] == 'o')
		    sort_only_mode = MODE_ON;
		break;
	    case '#':		/* #で始まる行はコメントアウト */
		sa_set_ignored_chars("#");
		break;
	    case 'd':		/* debug mode */
		sa_set_debug_mode(1);
		break;
	    case 'C':
		sort_check_mode = MODE_ON;
		break;
	    case 'D':
		dump_mode = MODE_ON;
		break;
	    case 'M':		/* 分割&マージ 990219 */
		if (argc == 2)    /* 引数がないと受け付けない */
		    usage();
		sscanf(argv[2], "%d", &block_size_mega);
		bunkatu_sort_mode = MODE_ON;
		argc--;
		argv++;
		break;
	    default :		/* エラー */
		fprintf(stderr, "%c: 無効なオプションです。\n", argv[1][1]);
		usage();
	    }
	} else {
	    strcpy(txt_fname, argv[1]); /* テキストファイル名 */
	}
	argc--;
	argv++;
    }


    sa_set_make_index_mode(modemode);

    if (sort_check_mode) {
	SUFARY *ary = sa_open(txt_fname, ary_fname);
	int is_sorted;
	printf("Are indexes sorted?  Checking\n");
	is_sorted = sa_is_sorted(ary);
	printf("%s\n", is_sorted ? "Ok." : "No good!");
	sa_close(ary);
	return !is_sorted;
    }

    if (dump_mode) {
	SUFARY *ary = sa_open(txt_fname, ary_fname);
	sa_dump_all_suffixes(ary);
	sa_close(ary);
	return 0;
    }

    /* write index */
    if (sort_only_mode != MODE_ON) {
	if (sa_write_index(txt_fname, ary_fname, get_next_ip) == FAIL) {
	    fprintf(stderr,"file open error: \"%s\" or \"%s\"\n",
		    txt_fname, ary_fname);
	    return 1;
	}
    }

    /* sort index */
    if (no_sort_mode == MODE_ON) {
	print_msg(modemode & SA_VERBOSE, "No sort.\n");
    } else {
	if (bunkatu_sort_mode == MODE_ON)
	    sa_set_make_index_memory_size(block_size_mega);
	else 
	    sa_set_make_index_memory_size(0);

	if (sa_sort_index(txt_fname, ary_fname) == FAIL) {
	    fprintf(stderr,"file open error: \"%s\"\n", ary_fname);
	    return 1;
	}
    }

    print_msg(modemode & SA_VERBOSE, "Done.\n");
    
    return 0;
}


/*
 * print "usage"
 */
void usage(void){
    fprintf(stderr, "\
mkary %s, an indexer of SUFARY.\n\n\
Usage: mkary [options] file\n\
  -o=file      出力ファイルを指定( 標準は text_file_name.ary )\n\
  -l           行単位で作成 ( \\nで区切る )\n\
  -w           単語単位で作成 ( ' ',\\t,\\n などで区切る )\n\
  -c           文字単位で作成 ( 標準 )\n\
  -b           バイト単位で作成\n\
  -q           メッセージなし\n\
  -ns          ソートしない(No Sort)\n\
  -so          ソートだけす(Sort Only)\n\
  -J           日本語文字と '<' 以外は無視する(文字単位のとき)\n\
  -#           #で始まる行はコメントアウト(行単位のとき)\n\
  -M=MEM       分割してソートを行い、最後にマージする\n\
               MEM で分割ブロックのサイズをメガバイト単位で指定。\n\
               メモリ不足のときには必須\n\
  -C           ちゃんとソートされてるかのチェック\n\
  -D           dump all suffixes.\n\
  -d           show the debug information.\n",
	    VERSION);
    exit(EXIT_FAILURE);
}

