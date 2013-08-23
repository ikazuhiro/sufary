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

char nkeys[100][80]; /* nkeys = NOT�����ѥ������(Ex: ^�ؤ�ؤ�) */
char keys[100][80];  /* keys  = �Ԥä��������ѥ������(Ex: �ۤ��ۤ�) */
int num_of_nkeys; int num_of_keys;
/* ������
 *  Ex: ^�ؤ�ؤ�&�ۤ��ۤ�&���Ҥ�
 *  �� nkeys[0] = "�ؤ�ؤ�", nom_of_nkeys = 1
 *  �� keys[0] = "�ۤ��ۤ�", keys[1] = "���Ҥ�", nom_of_nkeys = 2
 *  �����ν����� parse_keys("^�ؤ�ؤ�&�ۤ��ۤ�&���Ҥ�") �ǹԤ��롣
 */

int display_all_flag = 1;
int display_char_num = 316;
/* ������
 *  display_all_flag �� 1 �ʤ顢�������Ƥ�ɽ�����롣
 *  0 �ʤ鵭������Ƭ���� display_char_num ʸ������ɽ�����롣
 */

int main(int argc, char *argv[])
{
    SUFARY *ary;
    DID *did;
    char didfile[1000];

    while (argc > 1){
	if (argv[1][0] == '-')
	    switch (argv[1][1]) {
	    case 's': /* ��å���������Ϥ��ʤ� */
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
	    default : /* ���顼 */
		fprintf(stderr, "%c: ̵���ʥ��ץ����Ǥ���\n", argv[1][1]);
		usage();
	    }
	else
	    break;
	argc--;
	argv++;
    }

    if (argc < 2)
	usage();

    /* �ե�����򳫤� */
    if ((ary = sa_open(argv[2],NULL)) == NULL)
	exit(EXIT_FAILURE);
    
    sprintf(didfile,"%s.did",argv[2]);
    if ((did = sa_open_did(didfile)) == NULL)
	exit(EXIT_FAILURE);

    /* ��̥����å��������� */
    did_check = sa_malloc(did->didsize);
    if (!did_check) {
	fprintf(stderr, "MEMORY ALLOCATE ERROR\n");
	exit(EXIT_FAILURE);
    }
    (void)memset(did_check, 0, did->didsize);

    if (argv[1][0] == '\0') { /* ɸ�����Ϥ��饭����� */
	char cmd[1000];
	while (fgets(cmd, (int)sizeof(cmd), stdin)) {
	    cmd[strlen(cmd) - 1] = '\0'; /* ���Ϥ��줿������ɤβ����٤� */
	    do_kensaku(ary, did, cmd);
	    (void)memset(did_check, 0, did->didsize);
	}
    } else
	do_kensaku(ary, did, argv[1]); /* ������(argv[1])��������� */
    
    sa_close(ary);
    sa_close_did(did);
    sa_free(did_check);

    assert(sa_memory_leak_check());
    return 0;
}


/*
 * ��Ϣ�θ�������
 */
static int do_kensaku(SUFARY *ary, DID *did, char *key){
    /* [����] ���������å� bit �Ǥ��ε����������������������ݤ���Ƚ�Ǥ��롣
     *    �����å� bit ��������Ȭ�ġʳƵ������Ȥˡˡ��ǲ��� bit �� 1 �ʤ� 
     *    NOT������ɤ��ޤޤ�Ƥ��ꡢ������̤Ȥ�����Ŭ���Ǥ��롣¾�� bit 
     *    �����̤Υ�����ɤ��ޤޤ�Ƥ��뤫�ݤ���ɽ����
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

    /* [����] �ޤ�NOT������ɤ��ޤޤ�뵭���� NG bit ��Ω�Ƥ롣*/
    for(i = 0; i < num_of_nkeys; i++) {
	sr = sa_find(ary, 0, ary->arraysize - 1, nkeys[i], strlen(nkeys[i]), 0);
	if (sr.stat == SUCCESS)
	    check_keyword(ary, did, sr, 0x01);
    }

    /* [����] ���줫�����̥�����ɤ��ޤޤ�뵭���� OK bit Ω�Ƥ롣*/
    for(i = 0; i < num_of_keys - 1; i++){
	checker *= 2;
	sr = sa_find(ary, 0, ary->arraysize - 1, keys[i], strlen(keys[i]), 0);
	if (sr.stat == SUCCESS) {
	    SA_INDEX ntas;
	    if ((ntas = check_keyword(ary, did, sr, checker)) == 0)
		return 0;
	    printf("[%s] in %ld text areas.\n", keys[i], ntas);
	} else {
	    printf("NOT FOUND [%s]\n", keys[i]);
	    return 0;
	}
    }

    /* [����] �Ǹ�����̥�����ɤǤθ�����̵������� bit ������å����뤳�Ȥ�
       ��ꡢ���Ƥξ�������������������̤�ʬ���� */

    sr = sa_find(ary, 0, ary->arraysize - 1,
		 keys[num_of_keys - 1], strlen(keys[num_of_keys - 1]), 0);
    if (sr.stat == SUCCESS) {
	DID_RESULT *tas;
	SA_INDEX how_many;

	how_many = check_last_keyword(ary, did, sr, &tas, checker * 2 - 2);
	printf("FOUND %ld\n",how_many);
	if (display_char_num > 0) /* <= 0 : ������̵���������ɽ�� */
	    print_result(ary, &tas, how_many);
	sa_free(tas);
	return 1;
    } else {
	printf("NOT FOUND [%s]\n", keys[num_of_keys - 1]);
	return 0;
    }
}


/*
 * ʸ�����ʣ���Υ�����ʬ��
 * ���ޤ����ä��� 1�����Ԥ����� 0 ���֤���
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
 * �ƥ�����ɤθ�����̤����Τθ�����̤˳�Ǽ
 * ������ɤ�ޤ� Text Area �����֤�
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
 * �Ǹ�Υ�����ɤǤθ�����
 * ����̤� tas �˳�Ǽ���֤��ͤ� Text Area ����
 */
static SA_INDEX check_last_keyword(SUFARY *ary, DID *d, SUF_RESULT sr,
				   DID_RESULT **tas, unsigned char sikii)
{
    SA_INDEX ai;
    SA_INDEX how_many = 0;
    SA_INDEX sar = sr.right;
    SA_INDEX sal = sr.left;

    /* FIXME: ¿��� malloc */
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
	    did_check[dr.no] = 0x01; /* ���ϺѤ� */
	    (*tas)[how_many] = dr;
/*       printf("%d %ld %ld\n", how_many, */
/* 	     (*tas)[how_many].start, (*tas)[how_many].size); */
	    how_many++;
	}
    }

    return how_many;
}


/*
 * ������� Text Area �����
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
	    "af --- Article Finder : ���������ץ����\n\n"
	    "Version 0.0.4 991014\n\n"
	    "USAGE\n"
	    "  af [ -s NUM ] KEYWORD FILE_NAME\n"
	    "\n"
	    "OPTION\n"
	    "  -s NUM  : ������̵�������Ƭ���� NUM ʸ������ɽ������\n"
	    "            NUM = 0 �ΤȤ��ϸ�����̵������Τߤ�ɽ������\n"
	    "\n"
	    "TOPIC\n"
	    " * KEYWORD �� '' ����ꤹ���ɸ�����Ϥ��饭���������\n"
	    " * ������ɤν���\n"
	    "    AND����: ������ɤ� & �ǤĤʤ�\n"
	    "    NOT����: ������ɤ����� ^ ��Ĥ���\n"
	    "    (��) ^���&����&�����ƥ�\n"
	    "\n"
	);
    exit(EXIT_FAILURE);
}
