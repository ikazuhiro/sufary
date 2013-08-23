/*
 * $Id: sang.c,v 1.8 2000/07/16 09:24:58 tatuo-y Exp $
 *
 * sang  ---  Suffix Array を用いて N-gram 統計をとるプログラム
 *
 * USAGE   sang -n NUM -t NUM FILENAME
 * OPTION
 * -n NUM : NUM で n-gram の n を指定する。
 * -t NUM : threshold: NUM以下の頻度のものは表示しない
 *
 * n-gram には改行は含まれない。
 *
 * [実行例]
 * > cat test
 * ABCBACABBAACABCABCACABACABBACBACACAAABACCAB
 * > makeary -q test                 ● arrayファイルの作成
 * > sang -n 6 -t 1 test             ● 6-gram で頻度が 1 より大きいものを表示
 * 2 ACABBA
 * 2 BACABB
 * > sang -n 3 -t 4 test             ● trigram で頻度が 4 より大きいものを表示
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

char nstr[1000]; /* n-gram を一時待機させる場所 */
int lr_mode = 0;


int
main(int argc, char *argv[])
{
    SUFARY *ary;
    int ng = 0; /* n-gram の n */
    int threshold = 0; /* thresold */
  
    /* 引数処理 */
    if(argc <= 1){
	usage();
	exit(EXIT_FAILURE);
    }
    while (argc > 1){
	/* オプション指定 */
	if (argv[1][0] == '-')
	    switch (argv[1][1]){
		/* n-gramのnを指定 */
	    case 'n':
		ng = atoi(argv[2]);
		argc--; argv++;
		break;
		/* threshold: これ以下の頻度のものは表示しない */
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
 * n-gram統計をとる
 */
static void
do_sang(const SUFARY *ary, int ng, int threshold)
{
    long tmp;
    int n_ctr = 0;
    char *p;

    nstr[0] = '\0';

/* printf("%ld %ld\n",sa_bottom(ary),sa_top(ary)); */

    /* ary中の全ての要素に対してループ */
    for (tmp = 0; tmp < ary->arraysize; tmp++){
	/* テキストを得る */
	p = sa_aryidx2txtptr(ary, tmp);
	/* 同じn-gram→カウンタ増やす */
	if(strncmp(p,nstr,ng) == 0){
	    n_ctr++;
	}else{
	    if(*nstr != '\0' && !strstr(nstr,"\n"))
		hyouji(nstr,n_ctr,threshold);/* 閾値以上のものを文字化け防止して表示 */
	    /* つぎのエントリへ */
	    strncpy(nstr,p,ng);
	    n_ctr = 0;
	}
    }

    hyouji(nstr,n_ctr,threshold);/* 閾値以上のものを文字化け防止して表示 */

    /* 終了 */
    return;
}


/*
 *   n-gramの表示
 *
 * parameters
 *   nstr : n-gram文字列へのポインター
 *   n_ctr : このn-gram文字列の出現個数
 *   threshold : 表示の閾値(これ以下の頻度のものは表示せず)
 */
static void
hyouji(char *nstr, int n_ctr, int threshold)
{
    /* 閾値以上のものを文字化け防止して表示 */
    if(n_ctr >= threshold){
	(void)mojibakebousi(nstr, 0);
	printf("%d %s\n", n_ctr + 1, nstr);
    }
}


/*
 * 使い方を表示
 */
static void
usage(){
    fprintf(stderr, "Version 0.2  970327  YAMASITA Tatuo (tatuo-y@cl.aist-nara.ac.jp)\nUSAGE   sang -n NUM -t NUM FILENAME\nOPTION\n  -n NUM : N for N-gram\n  -t NUM : threshold\n");
}


/* from show.c(for 'array')
 *    全角文字の残骸をつぶす．特殊文字もつぶす．
 */
static int
mojibakebousi(char *buf, int haba){
    int i, pre_hankaku, post_hankaku;

    pre_hankaku = 0; /* キーワードより前の半角文字の数 */
    post_hankaku = 0; /* キーワードより後ろの半角文字の数 */
    for (i = 0; i < strlen(buf); i++) {
	if ((unsigned char)buf[i] < 0x80) { /* 半角文字を数える */
	    if (i < haba)
		pre_hankaku++;
	    else
		post_hankaku++;
	    if ((unsigned char)buf[i] < 0x20)
		buf[i] = '!'; /* 特殊文字は !  */
	}
    }
    /* 文字化け防止: 先頭に漢字の後ろ部分が来たとき，代わりに % にする． */
    if (pre_hankaku % 2 == 1)
	buf[0] = '%';
    /* 安全策: 最後に漢字の前部分が来たとき，代わりに % にする． */
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
