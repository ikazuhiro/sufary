/*
 * $Id: mkdid.c,v 1.12 2001/03/24 08:34:30 tatuo-y Exp $
 *
 *  SUFARY����������DocID�ե���������ץ����
 *  Version 1.0 981022
 *
 *  USAGE:
 *  mkdid START_TAG [END_TAG] FILE_NAME
 *
 *  ��END_TAG����ꤷ�ʤ��ȡ�END_TAG��START_TAG��Ʊ���Ȥߤʤ��ޤ���
 *
 *  ��DocID�ե���������ˤ�array�ե������FILE_NAME.ary�פ�ɬ��
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "sufary.h"
#include "did.h"
#include "util.h"

static void usage(void);

/*
 * print a message (using <stdarg.h>)
 */
static void print_msg(int ok, char *fmt, ...)
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
int main(int argc, char *argv[])
{
    char *tag1 = NULL, *tag2 = NULL, *txt_fname = NULL;
    int modemode = 0;
    char did_fname[1000];

    did_fname[0] = '\0';
    modemode |= SA_VERBOSE;

    while (argc > 1) {
	if (argv[1][0] == '-')
	    switch (argv[1][1]) {
	    case 'o': /* ���ϥե�����̾�λ��� */
		if (argc == 2)  /* �������ʤ��ȼ����դ��ʤ� */
		    usage();
		strcpy(did_fname, argv[2]);
		argc--;
		argv++;
		break;
	    case 'q': /* ��å���������Ϥ��ʤ� */
		modemode &= ~SA_VERBOSE;
		break;
	    case '-': /* ����ʹߤΰ����ϥ��ץ����ǤϤʤ� */
		argc--;
		argv++;
		goto OUT;
	    default : /* ���顼 */
		fprintf(stderr, "%c: ̵���ʥ��ץ����Ǥ���\n", argv[1][1]);
		usage();
	    }
	else
	    break;
	argc--;
	argv++;
    }
OUT:

    if (argc == 4) {
	tag1 = argv[1];
	tag2 = argv[2];
	txt_fname = argv[3];
    } else if(argc == 3) {
	tag1 = argv[1];
	txt_fname = argv[2];
    } else
	usage();

    sa_set_make_index_mode(modemode);

    sa_make_did(txt_fname, NULL, did_fname, tag1, tag2);

    print_msg(modemode & SA_VERBOSE, "done.\n");

    assert(sa_memory_leak_check());
    return 0;
}


/*
 * usage --- �Ȥ���
 */
static void usage(void){
    fprintf(stderr, "\n"
	    "Version 1.0.1 991014\n\n"
	    "USAGE\n"
	    "  mkdid [ -q ] [ -o FILE_NAME ] START_TAG [ END_TAG ] FILE_NAME\n"
	    "\n"
	    "OPTION\n"
	    "  -o FILE_NAME  : ���ϥե���������\n"
	    "  -q            : ��å������ʤ�\n"
	    "\n"
	);
    exit(EXIT_FAILURE);
}
