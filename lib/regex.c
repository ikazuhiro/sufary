/*
 * $Id: regex.c,v 1.24 2000/10/17 04:12:18 tatuo-y Exp $
 *
 * regular expression seach
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "sufary.h"
#include "util.h"

#define ESCAPE_CHAR ('\\')
#define NEGATED_CHAR_CLASS_CHAR ('^')

/* for bit-vector */
enum {
    NUM_8_BIT_CHAR = 256,
    NUM_7_BIT_CHAR = 128,
    BITS_IN_CHAR = 8
};

/*
  FUNCTION TREE:
  sa_regex
    + sa_regex_sub
        + sa_regex_store_result
	+ sa_regex_wild_card
	+ sa_regex_character_class
            + sa_regex_negate_character_class
        + sa_regex_match_alternation
            + sa_find
            + sa_regex_sub (recursive)
	+ sa_regex_match_one_char
            + sa_regex_sub (recursive)
            + sa_find
        + sa_regex_sub (recursive)
        + sa_regex_wchar_search
            + sa_regex_sub (recursive)
            + sa_find
	+ sa_regex_free_all_2nd_char_bv
 */


typedef struct BIT_VECTORS BIT_VECTORS;
struct BIT_VECTORS {

    unsigned char bv[NUM_8_BIT_CHAR / BITS_IN_CHAR]; /* b.v. for 1st char */
    unsigned char has_bv_2nd[NUM_7_BIT_CHAR / BITS_IN_CHAR];
    unsigned char *bvs_2nd[NUM_7_BIT_CHAR];   /* bit-vectors for 2nd char */

};


/* gloval variable */
SA_RESULT_LIST *m_last_item_ptr;

static SA_RESULT_LIST *sa_regex_sub(SUF_RESULT sr, const char *, int, int,
				       SA_RESULT_LIST *);


/* * some functions and macros for bit-vector handling
 */
#define bit_set(BITVECTOR,BIT_IDX) { \
    (BITVECTOR)[((unsigned char)(BIT_IDX)) >> 3] |= \
    (1 << ( ((unsigned char)(BIT_IDX)) & 0x7)); \
}

#define bit_unset(BITVECTOR,BIT_IDX) { \
    (BITVECTOR)[((unsigned char)(BIT_IDX)) >> 3] &= \
    ~(1 << ( ((unsigned char)(BIT_IDX)) & 0x7)); \
}

#define bit_found(BITVECTOR,BIT_IDX)  \
((BITVECTOR)[((unsigned char)(BIT_IDX)) >> 3]  & \
(1 << (((unsigned char)(BIT_IDX)) & 0x7)))

static void
bit_multi_set(unsigned char *bitvector, int start, int stop)
{
    unsigned int start_char = ((unsigned int) start) >> 3;
    unsigned int stop_char = ((unsigned int) stop) >> 3;
    /*
       printf("%x %x\n",(unsigned char)(0xFF << (((unsigned int)start) & 0x7)),
       (unsigned char)(0xFF >> (((unsigned int)stop) & 0x7)) );
     */
    if (start_char == stop_char) {
	bitvector[start_char] |=
	    ((0xFF << (((unsigned int) start) & 0x7)) &
	     (0xFF >> (7 - (((unsigned int) stop) & 0x7))));
    } else {
	unsigned int i;
	for (i = start_char + 1; i < stop_char; i++)
 	    bitvector[i] = ~0;
/* 	    bitvector[i] = 0xFF; */
	bitvector[start_char] |= (0xFF << (((unsigned int) start) & 0x7));
	bitvector[stop_char] |=
	    (0xFF >> (7 - (((unsigned int) stop) & 0x7)));
    }
}


/* static void */
/* bit_print(char *bitvector, int SIZE) */
/* { */
/*     unsigned int i; */
/*     for (i = 0; i < (SIZE); i++) { */
/* 	if ((i % 32) == 0 && i != 0) */
/* 	    printf("\n"); */
/* 	if ((i % 8) == 0) */
/* 	    printf("[%3x]", i); */
/* 	if (bit_found(bitvector, i)) */
/* 	    printf("1"); */
/* 	else */
/* 	    printf("0"); */
/*     } */
/*     printf("\n"); */
/* } */


/*
 * free all 2nd char bit-vectors
 */
static void
sa_regex_free_all_2nd_char_bv(BIT_VECTORS *bvs)
{
    int i;
    int bit_idx;

    for (i = 0; i < NUM_7_BIT_CHAR / BITS_IN_CHAR; i++) {

	if (bvs->has_bv_2nd[i] == 0)
	    continue;

	for (bit_idx = 0; bit_idx < BITS_IN_CHAR; bit_idx++) {
	    unsigned char t = i * BITS_IN_CHAR + bit_idx;
	    if (bit_found(bvs->has_bv_2nd, t))
		free(bvs->bvs_2nd[t]);
	}
    }
}


/*
 * find regex pattern in suffix array (using pseudo TRIE structure).
 */
SA_RESULT_LIST *
sa_regex(const SUFARY *ary, SA_INDEX left, SA_INDEX right,
	 const char *key, int keylen)
{
    SUF_RESULT sr;

    sr.suf = (SUFARY *)ary;
    sr.left = left;
    sr.right = right;

    m_last_item_ptr = NULL;

    return sa_regex_sub(sr, key, keylen, 0, NULL);
}


/*
 * matches 2 bytes char based on bit-vector
 */
static SA_RESULT_LIST *
sa_regex_wchar_search(
    unsigned char first_char,
    SUF_RESULT sr,
    int txt_skip,
    const char *key,
    int keylen,
    int skip,
    BIT_VECTORS *bvs,
    SA_RESULT_LIST *result_list
    )
{
    /*printf("key=%.*s\n",keylen, key); fflush(stdout); */

    if (bit_found(bvs->has_bv_2nd, first_char & 0x7f)) {
	int i;
	unsigned char *p = bvs->bvs_2nd[first_char & 0x7F];
	assert(p != NULL);

	for (i = 0; i < NUM_7_BIT_CHAR / BITS_IN_CHAR; i++) {

	    int bit_idx = 0;

	    if (p[i] == 0)
		continue;

	    for (bit_idx = 0; bit_idx < BITS_IN_CHAR; bit_idx++) {
		unsigned char t = 128 + i * BITS_IN_CHAR + bit_idx;
		SUF_RESULT ssr;
		if ((p[i] & (1 << bit_idx)) == 0)
		    continue;

		ssr = sa_find(sr.suf, sr.left, sr.right, (char*)&t,
			      1, txt_skip + 1);
		if (ssr.stat != SUCCESS)
		    continue;
		result_list = sa_regex_sub(ssr, key + skip, keylen - skip,
					   txt_skip + 2, result_list);
	    }
	}

    } else {
	unsigned char c12;
	for (c12 = 0xA1; c12 < 0xFF; c12++) {
	    SUF_RESULT ssr;
	    ssr = sa_find(sr.suf, sr.left, sr.right, (char*)&c12,
			  1, txt_skip + 1);
	    if (ssr.stat != SUCCESS)
		continue;
	    result_list = sa_regex_sub(ssr, key + skip, keylen - skip,
				       txt_skip + 2, result_list);
	}
    }

    return result_list;
}


/* 
 * store one result
 */
static SA_RESULT_LIST *
sa_regex_store_one_result(SA_INDEX value, int len, SA_RESULT_LIST *next)
{
    SA_RESULT_LIST* newp; 
    newp = malloc(sizeof(SA_RESULT_LIST));
    newp->value = value;
    newp->len = len;
    newp->next = next;
    return newp;
}


/*
 * store results of regex search.
 */
static SA_RESULT_LIST *
sa_regex_store_result(SUF_RESULT sr, int found_str_len,
		      SA_RESULT_LIST *result_list)
{
    SA_INDEX result_idx;
    for (result_idx = sr.left; result_idx <= sr.right; result_idx++) {
	SA_RESULT_LIST *new =
	    sa_regex_store_one_result(sa_aryidx2txtidx(sr.suf, result_idx),
				      found_str_len, NULL);
	if (result_list == NULL)
	    result_list = new;
	else
	    /* use module variable to add a item to the end of result-list */
	    m_last_item_ptr->next = new;
	m_last_item_ptr = new;
	/*
	   printf("* FOUND '%.*s' from %ld\n",found_str_len,
	   sa_aryidx2txtptr(ary, left),sa_aryidx2txtidx(ary,result_idx));
	 */
    }
    return result_list;
}


/*
 * SA_RESULT_LIST for storing results of regex search
 */
void
sa_free_result_list(SA_RESULT_LIST *listp)
{
    SA_RESULT_LIST *next;
    for (; listp != NULL; listp = next) {
	next = listp->next;
	free(listp);
    }
}


/*
 * handle the bit-vector for 2nd byte of 2 bytes character.  set 1s to
 * bits in "2nd char bit-vector".
 */
void
sa_regex_set_bits(
    unsigned char uc,	/* bit bector number */
    unsigned char from,	/* start index in bit vector */
    unsigned char to,	/* start index in bit vector */
    BIT_VECTORS *bvs
    )
{
    unsigned char *ptr = NULL;
    unsigned char uc7 = uc & 0x7F;

    if (bit_found(bvs->has_bv_2nd, uc7)) {

	ptr = bvs->bvs_2nd[uc7];
	assert(ptr != NULL);

    } else if (!bit_found(bvs->bv, uc)) {
	/* No "2nd char bit-vector" exists, make it! */

	ptr = malloc(NUM_7_BIT_CHAR / BITS_IN_CHAR);
	memset(ptr, 0x00, NUM_7_BIT_CHAR / BITS_IN_CHAR);
	bvs->bvs_2nd[uc7] = ptr;

	bit_set(bvs->bv, uc);
	bit_set(bvs->has_bv_2nd, uc7);

    } else
	return;

    assert(ptr != NULL);

    bit_multi_set(ptr, from & 0x7F, to & 0x7F);
}


/*
 * handles "nageted character class" like '[^a-zA-Z]'.
 * negates bit-vectors for characters.
 */
static void
sa_regex_negate_character_class(BIT_VECTORS *bvs)
{
    int c_idx;

    /* negate ascci char */

    for (c_idx = 0; c_idx < NUM_8_BIT_CHAR / BITS_IN_CHAR / 2; c_idx++)
	bvs->bv[c_idx] = ~bvs->bv[c_idx];
    bit_unset(bvs->bv, 0x00);	/* never match '\0' */
    bit_unset(bvs->bv, 0x0a);	/* never match '\n' */

    /* negate 2 bytes char */

    /* If a 1st byte of 2 bytes char has "2nd byte bit-vector",
       this process nagates the "2nd byte bit-vector" and doesn't
       negate the bit related to the "1st byte" in "1st byte
       bit-vector(char_bv)". */

    for (; c_idx < NUM_8_BIT_CHAR / BITS_IN_CHAR; c_idx++)
	if (bvs->bv[c_idx]) {
	    unsigned char bit_idx;
	    for (bit_idx = 0; bit_idx < BITS_IN_CHAR; bit_idx++)
		if (bvs->has_bv_2nd[c_idx - NUM_8_BIT_CHAR / BITS_IN_CHAR / 2] &
		    (1 << bit_idx)) {

		    unsigned char this_char = c_idx * BITS_IN_CHAR + bit_idx;
		    unsigned char ci;
		    unsigned char *p = bvs->bvs_2nd[this_char & 0x7F];
		    assert(p != NULL);

		    for (ci = 0; ci < NUM_7_BIT_CHAR / BITS_IN_CHAR; ci++)
			p[ci] = ~p[ci];

		} else {
		    bvs->bv[c_idx] ^= (1 << bit_idx);
		}
	} else {
	    bvs->bv[c_idx] = 0xFF;
	}
}


/*
 * wild card handling
 */
static int
sa_regex_wild_card(BIT_VECTORS *bvs)
{
    memset(bvs->bv, 0xFF, NUM_8_BIT_CHAR / BITS_IN_CHAR);
    bit_unset(bvs->bv, 0x00);	/* never match '\0' */
    bit_unset(bvs->bv, 0x0A);	/* never match '\n' */

    return 1;
}


/*
 * character class handling
 */
static int
sa_regex_character_class(const char *key, int keylen, BIT_VECTORS *bvs)
{
    int i;
    int not_op = 0;

    memset(bvs->bv, 0x00, NUM_8_BIT_CHAR / BITS_IN_CHAR);

    for (i = 1; i < keylen; i++) {

	if (i == 1 && key[i] == NEGATED_CHAR_CLASS_CHAR)
	    not_op = 1;

	else if (key[i] == ']' && i >= 2 && key[i - 1] != ESCAPE_CHAR)
	    break;

	else if (key[i] == '-' && i + 1 < keylen) {
	    unsigned char ri;
	    for (ri = key[i - 1]; ri <= key[i + 1]; ri++)
		bit_set(bvs->bv, ri);

	    i++;

	} else if (!MSB_ON(key[i])) {
	    /* ascci */
	    bit_set(bvs->bv, key[i]);
	} else {
	    /* 2 bytes character */

	    unsigned char c11 = key[i];	/* 1st and 2nd byte */
	    unsigned char c12 = key[i + 1];	/* of 2 bytes char  */

	    if (i + 4 < keylen && key[i + 2] == '-') {
		/* range indication */

		/* 2nd 2 bytes char of range indication */
		unsigned char c21 = key[i + 3];
		unsigned char c22 = key[i + 4];

		if (c11 == c21) {
		    /* same vector  samp:[ぁ-ん] */
		    sa_regex_set_bits(c11, c12, c22, bvs);

		} else {
		    unsigned char c_idx;

		    sa_regex_set_bits(c11, c12, 0xFF, bvs);
		    sa_regex_set_bits(c21, 0x00, c22, bvs);

		    /* ALL BIT ON */
		    for (c_idx = c11 + 1; c_idx < c21; c_idx++) {
			if (bit_found(bvs->has_bv_2nd, c_idx & 0x7F)) {
			    free(bvs->bvs_2nd[c_idx & 0x7F]);
			    bit_unset(bvs->has_bv_2nd, c_idx & 0x7F);
			} else
			    bit_set(bvs->bv, c_idx);
		    }
		}

		i += 4;

	    } else {
		sa_regex_set_bits(c11, c12, c12, bvs);

		i++;
	    }
	}
    }

    /* ERROR */
    if (key[i] != ']') {
	printf("PARSE ERROR\n");
	exit(EXIT_FAILURE);
    }

    if (not_op)
	sa_regex_negate_character_class(bvs);

    return i + 1;
}


/*
 * matches one character. handles escape character.
 */
static SA_RESULT_LIST *
sa_regex_match_one_char(
    SUF_RESULT srslt,
    const char *key,
    int keylen,
    int txt_skip,
    SA_RESULT_LIST *result_list
    )
{
    SUF_RESULT sr;
    int key_skip;
    int len;

    if (MSB_ON(key[0]) && keylen > 1 && MSB_ON(key[1])) {
	len = key_skip = 2;
    } else {
	len = key_skip = 1;
    }

    if (key[0] == ESCAPE_CHAR) {
	/* handle '\n' '\t' '\0xXX' */

	unsigned char dcc;	/* decodec char */

	if (keylen > 1) {
	    if (key[1] == 'n')
		dcc = '\n';
	    else if (key[1] == 't')
		dcc = '\t';
	    else if (key[1] == 'x') {
		int hex2int;
		int sscanf_ok = sscanf(key + 2, "%2x", &hex2int);
		assert(sscanf_ok);
		dcc = hex2int;
		key_skip += 2;
	    } else
		dcc = key[1];
	}

	key_skip++;

	sr = sa_find(srslt.suf, srslt.left, srslt.right, (char*)&dcc,
		     1, txt_skip);

    } else
	sr = sa_find(srslt.suf, srslt.left, srslt.right, key, len, txt_skip);

    if (sr.stat == SUCCESS)
	result_list = sa_regex_sub(sr, key + key_skip, keylen - key_skip,
				   txt_skip + len, result_list);

    if (key[key_skip] == '?') {
	key_skip++;
	result_list = sa_regex_sub(srslt, key + key_skip, keylen - key_skip,
				   txt_skip, result_list);
    }

    return result_list;
}


/*
 * handle "alternation" like '(hoge|uhi|ahaha)'.
 */
static SA_RESULT_LIST *
sa_regex_match_alternation(
    SUF_RESULT srslt,
    const char *key,
    int keylen,
    int txt_skip,
    SA_RESULT_LIST *result_list
    )
{
    SUF_RESULT sr;
    int key_skip;
    int i;
    int alternation_str_start = 1;

    /* scan the key for the end of alternation in advance. */
    for (i = 1; i < keylen; i++)
	if ((key[i] == ')') && i > 1 && key[i - 1] != ESCAPE_CHAR)
	    break;
    if (key[i] != ')') {
	printf("PARSE ERROR\n");
	exit(EXIT_FAILURE);
    }
    key_skip = i + 1;

    for (i = 1; i < key_skip; i++) {
	if ((key[i] == ')' || key[i] == '|') && i > 1 &&
	    key[i - 1] != ESCAPE_CHAR) {
	    int len = i - alternation_str_start;
	    /* printf("[%.*s]\n", len, key+alternation_str_start); */
	    sr = sa_find(srslt.suf, srslt.left, srslt.right,
			 key + alternation_str_start, len, txt_skip);
	    if (sr.stat == SUCCESS) {
		result_list =
		    sa_regex_sub(sr, key + key_skip, keylen - key_skip,
			      txt_skip + len, result_list);
	    }

	    alternation_str_start = i + 1;
	}
    }

    if (key[key_skip] == '?') {
	key_skip++;
	result_list = sa_regex_sub(srslt, key + key_skip, keylen - key_skip,
				txt_skip, result_list);
    }

    return result_list;
}


static SA_RESULT_LIST *
sa_regex_sub(
    SUF_RESULT srslt,
    const char *key,
    int keylen,
    int txt_skip,
    SA_RESULT_LIST *result_list
    )
{
    int i;
    int key_skip = 0;
    BIT_VECTORS bitvectors;

    memset(bitvectors.has_bv_2nd, 0x00, NUM_7_BIT_CHAR / BITS_IN_CHAR);

    if (0 == keylen)
	return sa_regex_store_result(srslt, txt_skip, result_list);

    if (key[0] == '.') {
	/* wild card --- use bit vector */
	key_skip = sa_regex_wild_card(&bitvectors);

    } else if (key[0] == '[') {
	/* character class --- use bit vector */
	key_skip = sa_regex_character_class(key, keylen, &bitvectors);

    } else if (key[0] == '?') {
	/* question --- skip! */
	/* 他のルーチンで先読みして処理しているのでここではスキップ */
	return sa_regex_sub(srslt, key + 1, keylen - 1, txt_skip,
			 result_list);

    } else if (key[0] == '(') {
	/* alternation */
	return sa_regex_match_alternation(srslt, key, keylen, txt_skip,
					   result_list);
    } else {
	/* one character */
	return sa_regex_match_one_char(srslt, key, keylen, txt_skip,
					result_list);
    }

    /* use bit vector (wild card and character class) */
    for (i = 0; i < NUM_8_BIT_CHAR / BITS_IN_CHAR; i++) {
	int bit_idx = 0;
	if (!bitvectors.bv[i])
	    continue;
	for (bit_idx = 0; bit_idx < BITS_IN_CHAR; bit_idx++) {
	    unsigned char t = i * BITS_IN_CHAR + bit_idx;
	    SUF_RESULT sr;
	    if (!(bitvectors.bv[i] & (1 << bit_idx)))
		continue;
	    /*printf("  search [%c] %d\n",t,t); */
	    sr = sa_find(srslt.suf, srslt.left, srslt.right, (char*)&t,
			 1, txt_skip);
	    if (sr.stat != SUCCESS)
		continue;
	    /*printf("  FOUND [%c] %x\n",t,t); */

	    if (!MSB_ON(t))	/* ASCII CHAR */
		result_list =
		    sa_regex_sub(sr, key + key_skip, keylen - key_skip,
				 txt_skip + 1, result_list);
	    else
		result_list = sa_regex_wchar_search(t, sr, txt_skip, key,
						     keylen, key_skip,
						     &bitvectors, result_list);
	}
    }

    sa_regex_free_all_2nd_char_bv(&bitvectors);

    if (key[key_skip] == '?') {
	key_skip++;
	result_list = sa_regex_sub(srslt, key + key_skip, keylen - key_skip,
				txt_skip, result_list);
    }

    return result_list;
}


static void
sa_expand_ignore_case(char *new_key, const char *key)
{
    int i, j = 0;
    for (i = 0; i < (int)strlen(key); i++) {
        if (isalpha((unsigned char)key[i])) {
            new_key[j] = '[';
            new_key[j + 1] = tolower(key[i]);
            new_key[j + 2] = toupper(key[i]);
            new_key[j + 3] = ']';
            j += 4;
        } else if (key[i] == '[' || key[i] == ']' || key[i] == '\\' ||
                   key[i] == '(' || key[i] == ')' || key[i] == '.' ||
                   key[i] == '|' || key[i] == '*' || key[i] == '?') {
            new_key[j] = '\\';
            new_key[j + 1] = key[i];
            j += 2;
        } else {
            new_key[j] = key[i];
            j++;
        }
    }
    new_key[j] = '\0';
}


extern SA_RESULT_LIST *sa_ignore_case(const SUFARY *ary,
				      SA_INDEX left, SA_INDEX right,
				      const char *keyword, int keyword_length)
{
    SA_RESULT_LIST *rl;
    char *tmp_key;
    tmp_key = malloc(keyword_length * 4 + 1); /* estimation: len x 4 */
    sa_expand_ignore_case(tmp_key, keyword);
    rl = sa_regex(ary, left, right, tmp_key, strlen(tmp_key));
    free(tmp_key); 
    return rl;
}



