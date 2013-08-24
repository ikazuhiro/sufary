/*
 * $Id: af.c,v 1.16 2000/11/17 02:51:50 tatuo-y Exp $
 *
 * af --- SUFARY Article Finder
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sufary.h"
#include "did.h"
#include "util.h"

static int do_kensaku(SUFARY *ary, DID *did, char *key);
static SA_INDEX check_keyword(SUFARY *ary, DID *d, SUF_RESULT sr, unsigned char var);
static SA_INDEX check_last_keyword(SUFARY *ary, DID *d, SUF_RESULT sr, 
				   DID_RESULT **tas, unsigned char);
static void print_result(SUFARY *ary, DID_RESULT **tas, SA_INDEX how_many);

static void parse_keys_sub(char*);
static int parse_keys(char*);
static void usage(void);

unsigned char *did_check;

char nkeys[100][80]; /* nkeys = NOT検索用キーワード(Ex: ^へろへろ) */
char keys[100][80];  /* keys  = ぴったし検索用キーワード(Ex: ほげほげ) */
int num_of_nkeys; int num_of_keys;
/* 説明！
 *  Ex: ^へろへろ&ほげほげ&うひょ
 *  → nkeys[0] = "へろへろ", nom_of_nkeys = 1
 *  　 keys[0] = "ほげほげ", keys[1] = "うひょ", nom_of_nkeys = 2
 *  これらの処理は parse_keys("^へろへろ&ほげほげ&うひょ") で行われる。
 */

int display_all_flag = 1;
int display_char_num = 316;
/* 説明！
 *  display_all_flag が 1 なら、記事全てを表示する。
 *  0 なら記事の先頭から display_char_num 文字だけ表示する。
 */

int main(int argc, char *argv[])
{
    SUFARY *ary;
    DID *did;
    char didfile[1000];

    while (argc > 1){
	if (argv[1][0] == '-')
	    switch (argv[1][1]) {
	    case 's': /* メッセージを出力しない */
		display_all_flag = 0;
		if (argc == 2)
		    usage();
		display_char_num = atoi(argv[2]);
		argc--;
		argv++;
		break;
	    case 'd':             /* debug mode */
		sa_set_debug_mode(1);
		break;
	    default : /* エラー */
		fprintf(stderr, "%c: 無効なオプションです。\n", argv[1][1]);
		usage();
	    }
	else
	    break;
	argc--;
	argv++;
    }

    if (argc < 2)
	usage();

    /* ファイルを開く */
    if ((ary = sa_open(argv[2],NULL)) == NULL)
	exit(EXIT_FAILURE);
    
    sprintf(didfile,"%s.did",argv[2]);
    if ((did = sa_open_did(didfile)) == NULL)
	exit(EXIT_FAILURE);

    /* 結果チェック配列を準備 */
    did_check = sa_malloc(did->didsize);
    if (!did_check) {
	fprintf(stderr, "MEMORY ALLOCATE ERROR\n");
	exit(EXIT_FAILURE);
    }
    (void)memset(did_check, 0, did->didsize);

    if (argv[1][0] == '\0') { /* 標準入力からキーワード */
	char cmd[1000];
	while (fgets(cmd, (int)sizeof(cmd), stdin)) {
	    cmd[strlen(cmd) - 1] = '\0'; /* 入力されたキーワードの改行潰し */
	    do_kensaku(ary, did, cmd);
	    (void)memset(did_check, 0, did->didsize);
	}
    } else
	do_kensaku(ary, did, argv[1]); /* 第一引数(argv[1])がキーワード */
    
    sa_close(ary);
    sa_close_did(did);
    sa_free(did_check);

    assert(sa_memory_leak_check());
    return 0;
}


/*
 * 一連の検索処理
 */
static int do_kensaku(SUFARY *ary, DID *did, char *key){
    /* [説明] 記事チェック bit でその記事が検索条件を満たすか否かを判断する。
     *    チェック bit は全部で八つ（各記事ごとに）。最下位 bit が 1 なら 
     *    NOTキーワードが含まれており、検索結果として不適当である。他の bit 
     *    は普通のキーワードが含まれているか否かを表す。
     */

    int i;
    unsigned char checker = 0x01;
    SUF_RESULT sr;

    num_of_nkeys = 0;
    num_of_keys = 0;

    if (parse_keys(key) == 0) {
	printf("KEYWORD FORMAT ERROR\n");
	return 0;
    }

    /* [説明] まずNOTキーワードが含まれる記事の NG bit を立てる。*/
    for(i = 0; i < num_of_nkeys; i++) {
	sr = sa_find(ary, 0, ary->arraysize - 1, nkeys[i], strlen(nkeys[i]), 0);
	if (sr.stat == SUCCESS)
	    check_keyword(ary, did, sr, 0x01);
    }

    /* [説明] それから普通キーワードが含まれる記事の OK bit 立てる。*/
    for(i = 0; i < num_of_keys - 1; i++){
	checker *= 2;
	sr = sa_find(ary, 0, ary->arraysize - 1, keys[i], strlen(keys[i]), 0);
	if (sr.stat == SUCCESS) {
	    SA_INDEX ntas;
	    if ((ntas = check_keyword(ary, did, sr, checker)) == 0)
		return 0;
	    printf("[%s] in %d text areas.\n", keys[i], ntas);
	} else {
	    printf("NOT FOUND [%s]\n", keys[i]);
	    return 0;
	}
    }

    /* [説明] 最後の普通キーワードでの検索結果記事の全 bit をチェックすることに
       より、全ての条件を満たす記事検索結果が分かる */

    sr = sa_find(ary, 0, ary->arraysize - 1,
		 keys[num_of_keys - 1], strlen(keys[num_of_keys - 1]), 0);
    if (sr.stat == SUCCESS) {
	DID_RESULT *tas;
	SA_INDEX how_many;

	how_many = check_last_keyword(ary, did, sr, &tas, checker * 2 - 2);
	printf("FOUND %d\n",how_many);
	if (display_char_num > 0) /* <= 0 : 検索結果記事数だけ表示 */
	    print_result(ary, &tas, how_many);
	sa_free(tas);
	return 1;
    } else {
	printf("NOT FOUND [%s]\n", keys[num_of_keys - 1]);
	return 0;
    }
}


/*
 * 文字列を複数のキーに分割
 * うまくいったら 1、失敗したら 0 を返す。
 */
static int parse_keys(char *kstr){
    char *start = kstr;
    int keylen = strlen(kstr);
    int i;

    for (i = 0; i < keylen - 1; i++)
	if (strncmp(kstr + i, "&&", 2) == 0)
	    return 0;

    if (kstr[keylen - 1] == '&')
	return 0;

    for (i = 0; i < keylen; i++)
	if (kstr[i] == '&') {
	    kstr[i] = '\0';
	    parse_keys_sub(start);
	    start = kstr + i + 1;
	}

    if (*start != '\0')
	parse_keys_sub(start);

    if (num_of_keys == 0)
	return 0;
    else
	return 1;
} 

static void parse_keys_sub(char *start){
    if (*start == '^') {
	strcpy(nkeys[num_of_nkeys], start + 1);
	num_of_nkeys++;
    } else {
	strcpy(keys[num_of_keys], start);
	num_of_keys++;
    }
}


/*
 * 各キーワードの検索結果を全体の検索結果に格納
 * キーワードを含む Text Area 数を返す
 */
static SA_INDEX check_keyword(SUFARY *ary, DID *d, SUF_RESULT sr,
			      unsigned char var)
{
    SA_INDEX ai;
    SA_INDEX num_of_tas = 0;    

    for (ai = sr.left; ai <= sr.right; ai++) {
	DID_RESULT dr;
	dr = sa_didsearch(d, sa_aryidx2txtidx(ary, ai));
	if (dr.stat == SUCCESS) {
	    if ((did_check[dr.no] & var) == 0)
		num_of_tas++;    
	    did_check[dr.no] |= var;
	}
    }
    return num_of_tas;
}


/*
 * 最後のキーワードでの検索。
 * 総合結果は tas に格納。返し値は Text Area 数。
 */
static SA_INDEX check_last_keyword(SUFARY *ary, DID *d, SUF_RESULT sr,
				   DID_RESULT **tas, unsigned char sikii)
{
    SA_INDEX ai;
    SA_INDEX how_many = 0;
    SA_INDEX sar = sr.right;
    SA_INDEX sal = sr.left;

    /* FIXME: 多めに malloc */
    *tas = (DID_RESULT*)sa_malloc(sizeof(DID_RESULT) * (sar - sal + 1));

    if (*tas == NULL) {
	fprintf(stderr, "MEMORY ALLOCATE ERROR\n");
	exit(EXIT_FAILURE);
    }

    for (ai = sal; ai <= sar; ai++) {
	DID_RESULT dr;

	dr = sa_didsearch(d, sa_aryidx2txtidx(ary, ai));
	if (dr.stat != SUCCESS)
	    continue;
	if (did_check[dr.no] == sikii ) {
	    did_check[dr.no] = 0x01; /* 出力済み */
	    (*tas)[how_many] = dr;
/*       printf("%d %ld %ld\n", how_many, */
/* 	     (*tas)[how_many].start, (*tas)[how_many].size); */
	    how_many++;
	}
    }

    return how_many;
}


/*
 * 検索結果 Text Area を出力
 */
static void print_result(SUFARY *ary, DID_RESULT **tas, SA_INDEX how_many)
{
    int i;

    for (i = 0; i < how_many; i++) {
	SA_STRING art;
	if (display_all_flag == 1) {
	    art.ptr = sa_txtidx2txtptr(ary, (*tas)[i].start);
	    art.len = (*tas)[i].size;
	    printf("%.*s\n", (int)art.len, art.ptr);
	} else {
	    int j;
	    int ctr8 = 0;
	    art.ptr = sa_txtidx2txtptr(ary, (*tas)[i].start);
	    art.len = display_char_num;
	    for (j = display_char_num - 1; j >= 0; j--)
		if ((unsigned char)art.ptr[j] & 0x80)
		    ctr8++;
		else
		    break;
	    if (ctr8 % 2 == 1)
		art.len--; 
	    printf("---%.*s\n", (int)art.len, art.ptr);
	}
    }
}


/*
 * usage
 */
static void usage(void)
{
    fprintf(stderr, "\n"
	    "af --- Article Finder : 記事検索プログラム\n\n"
	    "Version 0.0.4 991014\n\n"
	    "USAGE\n"
	    "  af [ -s NUM ] KEYWORD FILE_NAME\n"
	    "\n"
	    "OPTION\n"
	    "  -s NUM  : 検索結果記事の先頭から NUM 文字だけ表示する\n"
	    "            NUM = 0 のときは検索結果記事数のみを表示する\n"
	    "\n"
	    "TOPIC\n"
	    " * KEYWORD に '' を指定すると標準入力からキーワード入力\n"
	    " * キーワードの書き方\n"
	    "    AND検索: キーワードを & でつなぐ\n"
	    "    NOT検索: キーワードの前に ^ をつける\n"
	    "    (例) ^歴史&言語&システム\n"
	    "\n"
	);
    exit(EXIT_FAILURE);
}
