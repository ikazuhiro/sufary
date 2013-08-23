/*
 * $Id: sang.c,v 1.8 2000/07/16 09:24:58 tatuo-y Exp $
 *
 * sang  ---  Suffix Array ���Ѥ��� N-gram ���פ�Ȥ�ץ����
 *
 * USAGE   sang -n NUM -t NUM FILENAME
 * OPTION
 * -n NUM : NUM �� n-gram �� n ����ꤹ�롣
 * -t NUM : threshold: NUM�ʲ������٤Τ�Τ�ɽ�����ʤ�
 *
 * n-gram �ˤϲ��Ԥϴޤޤ�ʤ���
 *
 * [�¹���]
 * > cat test
 * ABCBACABBAACABCABCACABACABBACBACACAAABACCAB
 * > makeary -q test                 �� array�ե�����κ���
 * > sang -n 6 -t 1 test             �� 6-gram �����٤� 1 ����礭����Τ�ɽ��
 * 2 ACABBA
 * 2 BACABB
 * > sang -n 3 -t 4 test             �� trigram �����٤� 4 ����礭����Τ�ɽ��
 * 6 ACA
 * 5 BAC
 * 6 CAB
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sufary.h"

static void usage(void);
static void do_sang(const SUFARY *ary, int ng, int threshold);
static void hyouji(char*, int, int);
static int mojibakebousi(char*, int);

static void do_sang_2(const SUFARY *ary, int ng, int threshold);

char nstr[1000]; /* n-gram �����Ե��������� */
int lr_mode = 0;


int
main(int argc, char *argv[])
{
    SUFARY *ary;
    int ng = 0; /* n-gram �� n */
    int threshold = 0; /* thresold */
  
    /* �������� */
    if(argc <= 1){
	usage();
	exit(EXIT_FAILURE);
    }
    while (argc > 1){
	/* ���ץ������� */
	if (argv[1][0] == '-')
	    switch (argv[1][1]){
		/* n-gram��n����� */
	    case 'n':
		ng = atoi(argv[2]);
		argc--; argv++;
		break;
		/* threshold: ����ʲ������٤Τ�Τ�ɽ�����ʤ� */
	    case 't': 
		threshold = atoi(argv[2]);
		argc--; argv++;
		break;
	    case 'l': 
		lr_mode = 1;
		break;
	    default : 
		usage();
		exit(EXIT_FAILURE);
	    }

	argc--; argv++;
    }

    if ((ary = sa_open(argv[0],NULL)) == NULL){
	printf("argument ignored.\n");
	usage();
	exit(EXIT_FAILURE);
    }

    if (lr_mode)
	do_sang_2(ary, ng, threshold);
    else
	do_sang(ary, ng, threshold);

    sa_close(ary);
  
    return 0;
}


/*
 * n-gram���פ�Ȥ�
 */
static void
do_sang(const SUFARY *ary, int ng, int threshold)
{
    long tmp;
    int n_ctr = 0;
    char *p;

    nstr[0] = '\0';

/* printf("%ld %ld\n",sa_bottom(ary),sa_top(ary)); */

    /* ary������Ƥ����Ǥ��Ф��ƥ롼�� */
    for (tmp = 0; tmp < ary->arraysize; tmp++){
	/* �ƥ����Ȥ����� */
	p = sa_aryidx2txtptr(ary, tmp);
	/* Ʊ��n-gram�����������䤹 */
	if(strncmp(p,nstr,ng) == 0){
	    n_ctr++;
	}else{
	    if(*nstr != '\0' && !strstr(nstr,"\n"))
		hyouji(nstr,n_ctr,threshold);/* ���Ͱʾ�Τ�Τ�ʸ�������ɻߤ���ɽ�� */
	    /* �Ĥ��Υ���ȥ�� */
	    strncpy(nstr,p,ng);
	    n_ctr = 0;
	}
    }

    hyouji(nstr,n_ctr,threshold);/* ���Ͱʾ�Τ�Τ�ʸ�������ɻߤ���ɽ�� */

    /* ��λ */
    return;
}


/*
 *   n-gram��ɽ��
 *
 * parameters
 *   nstr : n-gramʸ����ؤΥݥ��󥿡�
 *   n_ctr : ����n-gramʸ����νи��Ŀ�
 *   threshold : ɽ��������(����ʲ������٤Τ�Τ�ɽ������)
 */
static void
hyouji(char *nstr, int n_ctr, int threshold)
{
    /* ���Ͱʾ�Τ�Τ�ʸ�������ɻߤ���ɽ�� */
    if(n_ctr >= threshold){
	(void)mojibakebousi(nstr, 0);
	printf("%d %s\n", n_ctr + 1, nstr);
    }
}


/*
 * �Ȥ�����ɽ��
 */
static void
usage(){
    fprintf(stderr, "Version 0.2  970327  YAMASITA Tatuo (tatuo-y@cl.aist-nara.ac.jp)\nUSAGE   sang -n NUM -t NUM FILENAME\nOPTION\n  -n NUM : N for N-gram\n  -t NUM : threshold\n");
}


/* from show.c(for 'array')
 *    ����ʸ���λĳ���Ĥ֤����ü�ʸ����Ĥ֤���
 */
static int
mojibakebousi(char *buf, int haba){
    int i, pre_hankaku, post_hankaku;

    pre_hankaku = 0; /* ������ɤ������Ⱦ��ʸ���ο� */
    post_hankaku = 0; /* ������ɤ�����Ⱦ��ʸ���ο� */
    for (i = 0; i < strlen(buf); i++) {
	if ((unsigned char)buf[i] < 0x80) { /* Ⱦ��ʸ��������� */
	    if (i < haba)
		pre_hankaku++;
	    else
		post_hankaku++;
	    if ((unsigned char)buf[i] < 0x20)
		buf[i] = '!'; /* �ü�ʸ���� !  */
	}
    }
    /* ʸ�������ɻ�: ��Ƭ�˴����θ����ʬ���褿�Ȥ�������� % �ˤ��롥 */
    if (pre_hankaku % 2 == 1)
	buf[0] = '%';
    /* ������: �Ǹ�˴���������ʬ���褿�Ȥ�������� % �ˤ��롥 */
    if (post_hankaku % 2 == 1 && strlen(buf) % 2 == 0)
	buf[i-1] = '%';
    if (post_hankaku % 2 == 0 && strlen(buf) % 2 == 1)
	buf[i-1] = '%';

    return pre_hankaku;
}


static int
get_common_prefix_length(const char *a, const char *b)
{
    int i = 0;
    if (a == NULL || b == NULL)
	return 0;
    /* double space, CL */
    while(a[i] == b[i] && !(a[i] == '\n' || (a[i] == ' ' && a[i + 1] == ' ')))
	i++;
    return i;
}


#define COMMON_PREFIX_MAX 50
/*
 * longest repetition
 */
static void
do_sang_2(const SUFARY *ary, int ng, int threshold)
{
    SA_INDEX tmp;
    char *pre_p = NULL;
    int pre_l = 0;


    static SA_INDEX counters[COMMON_PREFIX_MAX + 1];

    for (tmp = 0; tmp <= ary->arraysize; tmp++) {
	int len = 0;
	char *p;
	if (tmp < ary->arraysize) {
	    p = sa_aryidx2txtptr(ary, tmp);
	    len = get_common_prefix_length(pre_p, p);	
	    if (len > COMMON_PREFIX_MAX)
		len = COMMON_PREFIX_MAX;
	}
	if (pre_l > len) {
	    int i;
	    int pre_c = 0;
	    for (i = pre_l; i > len && i >= ng; i--) {
		if (counters[i] + 1 > threshold && pre_c != counters[i]) {
		    printf("%ld:", (long)counters[i] + 1);
		    printf("%.*s\n", i, pre_p);
		}
		counters[i - 1] += counters[i];
		pre_c = counters[i];
		counters[i] = 0;
	    }
	}
	counters[len]++;
	pre_p = p;
	pre_l = len;
    }

    return;
}
