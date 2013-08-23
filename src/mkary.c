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
static int no_sort_mode = MODE_OFF; /* �����Ȥ��ʤ��⡼�� */
static int sort_only_mode = MODE_OFF; /* �����Ȥ������ʤ��⡼�� */
static int sort_check_mode = MODE_OFF; /* �����Ȥ���Ƥ뤫�Ǹ�˥����å� */
static int dump_mode = MODE_OFF;
static int bunkatu_sort_mode = MODE_OFF; /* 990219 */
static int block_size_mega; /* 990219 ʬ��֥�å��� */


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
    char txt_fname[FILE_NAME_LEN];	/* ���ϥե�����̾ */
    char ary_fname[FILE_NAME_LEN];	/* ������ե�����̾ */

    int modemode = 0;

    SA_INDEX (*get_next_ip)(SA_STRING, SA_INDEX) = sa_get_next_ip_char;

    txt_fname[0] = '\0';
    ary_fname[0] = '\0';

    progname = argv[0];		/* �ץ����̾ */

    modemode |= SA_VERBOSE;


    if (argc <= 1)
	usage();

    while (argc > 1) {
	if (argv[1][0] == '-') {
	    char op = argv[1][1];
	    if (op == '-')	/* long option name: --char */
		op = argv[1][2];
	    switch (op) {
	    case 'o':		/* ���ϥե�����̾�λ��� */
		if (argc == 2){	/* �������ʤ��ȼ����դ��ʤ� */
		    fprintf(stderr,"-o <filename> --- ���ϥե�����̾�����\n");
		    exit(EXIT_FAILURE);
		}
		strcpy(ary_fname,argv[2]);
		argc--;
		argv++;
		break;
	    case 'l':		/* �����˥���ǥå������� */
		get_next_ip = sa_get_next_ip_line;
		break;
	    case 'w':		/* �����˥���ǥå������� */
		get_next_ip = sa_get_next_ip_word;
		break;
	    case 'c':		/* ��ʸ����˥���ǥå�������ʥǥե���ȡ�*/
		sa_set_my_chars(NULL);
		get_next_ip = sa_get_next_ip_char;
		break;
	    case 'b':		/* 2�Х��Ȱ�ʸ��������Ԥʤ�ʤ� */
		get_next_ip = sa_get_next_ip_byte;
		break;
	    case 'J':		/* ���ܸ�⡼�� */
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
	    case 'n':		/* -ns �����Ȥ��ʤ��⡼�� */
		if (argv[1][2] == 's')
		    no_sort_mode = MODE_ON;
		break;
	    case 's':		/* -so �����Ȥ������ʤ��⡼�� */
		if (argv[1][2] == 'o')
		    sort_only_mode = MODE_ON;
		break;
	    case '#':		/* #�ǻϤޤ�Ԥϥ����ȥ����� */
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
	    case 'M':		/* ʬ��&�ޡ��� 990219 */
		if (argc == 2)    /* �������ʤ��ȼ����դ��ʤ� */
		    usage();
		sscanf(argv[2], "%d", &block_size_mega);
		bunkatu_sort_mode = MODE_ON;
		argc--;
		argv++;
		break;
	    default :		/* ���顼 */
		fprintf(stderr, "%c: ̵���ʥ��ץ����Ǥ���\n", argv[1][1]);
		usage();
	    }
	} else {
	    strcpy(txt_fname, argv[1]); /* �ƥ����ȥե�����̾ */
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
  -o=file      ���ϥե���������( ɸ��� text_file_name.ary )\n\
  -l           ��ñ�̤Ǻ��� ( \\n�Ƕ��ڤ� )\n\
  -w           ñ��ñ�̤Ǻ��� ( ' ',\\t,\\n �ʤɤǶ��ڤ� )\n\
  -c           ʸ��ñ�̤Ǻ��� ( ɸ�� )\n\
  -b           �Х���ñ�̤Ǻ���\n\
  -q           ��å������ʤ�\n\
  -ns          �����Ȥ��ʤ�(No Sort)\n\
  -so          �����Ȥ�����(Sort Only)\n\
  -J           ���ܸ�ʸ���� '<' �ʳ���̵�뤹��(ʸ��ñ�̤ΤȤ�)\n\
  -#           #�ǻϤޤ�Ԥϥ����ȥ�����(��ñ�̤ΤȤ�)\n\
  -M=MEM       ʬ�䤷�ƥ����Ȥ�Ԥ����Ǹ�˥ޡ�������\n\
               MEM ��ʬ��֥�å��Υ�������ᥬ�Х���ñ�̤ǻ��ꡣ\n\
               ������­�ΤȤ��ˤ�ɬ��\n\
  -C           �����ȥ����Ȥ���Ƥ뤫�Υ����å�\n\
  -D           dump all suffixes.\n\
  -d           show the debug information.\n",
	    VERSION);
    exit(EXIT_FAILURE);
}

