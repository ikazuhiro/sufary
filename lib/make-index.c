/*
 * $Id: make-index.c,v 1.33 2000/11/28 07:56:42 tatuo-y Exp $
 *
 * make indexes
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>

#include <time.h>

#include "sufary.h"
#include "util.h"
#include "config.h"
#include "make-index.h"

#define STR_CACHE_SIZE 20
/* for heap */
typedef struct {
    SA_INDEX aryidx;
    SA_INDEX max;
    char strbuf[STR_CACHE_SIZE + 1];
} BLOCK;


static void insertheap(BLOCK** a, BLOCK* v);
static void upheap(BLOCK** a, int k);
static void downheap(BLOCK** a, int k);
static BLOCK* hremove(BLOCK** a);

static int m_size_of_heap = 0;

static SA_INDEX *m_array_ptr;
static SA_INDEX m_size_of_array_file;
static char *m_text_ptr;
static char *m_text_last_ptr;

static int m_memory_size = 0;


static void multikey_qsort(SA_INDEX *x, char *txt, SA_INDEX n, SA_INDEX depth);
static SA_STAT sa_sort_each_block(SA_INDEX, SA_INDEX, BLOCK**);
static SA_STAT sa_merge_blocks(char*, SA_INDEX, BLOCK**);
static int sa_strcmp(const char *p1, const char *p2);


SA_INDEX m_progress_counter;
int sa_mki_mode;


void
sa_set_make_index_memory_size(int mega_byte)
{
    m_memory_size = mega_byte;
}


void
sa_set_make_index_mode(int mode)
{
    sa_mki_mode = mode;
}


static void 
print_progress_meter(SA_INDEX current, SA_INDEX total)
{
    static char bar[]
        = "oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo";
    static int scale = sizeof(bar) - 1;
    static int prev_percent;
    int progress, percent;
    assert(0 < total && current <= total);

    if (current <= 0 || current == total || prev_percent == 100)
	prev_percent = -1;

    progress = (int)((double)current / total * scale);
    percent  = (int)((double)current / total * 100);

    if (percent > prev_percent) {
        printf("\r");
        printf("%3d%% |%.*s%*s| %ld", percent, 
               progress, bar, scale - progress, "", (long)current);
        prev_percent = percent;
	fflush(stdout);
    }
}




/*
 * Compare suffixes for qsort.
 */
static int 
suffix_compare(const SA_INDEX *a, const SA_INDEX *b)
{
    int j =  sa_strcmp(m_text_ptr + *a, m_text_ptr + *b);
    return j;
}


SA_STAT
sa_sort_index_system_qsort()
{
#ifndef WORDS_BIGENDIAN
    sa_reverse_byte_order_all(m_array_ptr, sizeof(SA_INDEX),
			      (size_t)m_size_of_array_file);
#endif
    qsort(m_array_ptr, (size_t)m_size_of_array_file, sizeof(SA_INDEX),
	  (int (*)(const void *, const void *))suffix_compare);
#ifndef WORDS_BIGENDIAN
    sa_reverse_byte_order_all(m_array_ptr, sizeof(SA_INDEX),
			      (size_t)m_size_of_array_file);
#endif
    return SUCCESS;
}


SA_STAT
sa_sort_index_multikey_qsort()
{
#ifndef WORDS_BIGENDIAN
    sa_reverse_byte_order_all(m_array_ptr, sizeof(SA_INDEX),
			      (size_t)m_size_of_array_file);
#endif
    m_progress_counter = 0;
    multikey_qsort(m_array_ptr, m_text_ptr, m_size_of_array_file, 0);
#ifndef WORDS_BIGENDIAN
    sa_reverse_byte_order_all(m_array_ptr, sizeof(SA_INDEX),
			      (size_t)m_size_of_array_file);
#endif
    if (sa_mki_mode & SA_VERBOSE) {
	print_progress_meter(m_size_of_array_file, m_size_of_array_file);
	printf("\n");
    }
    return SUCCESS;
}


SA_STAT
sa_sort_index_divide_and_merge(char *ary_fname)
{
    SA_INDEX bl_size = 1000000 / sizeof(SA_INDEX) * m_memory_size;
    BLOCK **bl;
    char merged_ary_fname[SA_FILE_NAME_MAX];
    int num_of_blocks;

#ifndef WORDS_BIGENDIAN
    sa_reverse_byte_order_all(m_array_ptr, sizeof(SA_INDEX),
			      (size_t)m_size_of_array_file);
#endif

    num_of_blocks = m_size_of_array_file / bl_size;
    if (m_size_of_array_file % bl_size)
	num_of_blocks++;

    bl = sa_malloc((num_of_blocks + 1) * sizeof(BLOCK*)); 

    if (sa_mki_mode & SA_VERBOSE)
	printf(" Number of blocks = %d\n", num_of_blocks);

    if (sa_sort_each_block(bl_size, m_size_of_array_file, bl) == FAIL)
	return FAIL;
    sprintf(merged_ary_fname, "%s-", ary_fname);
    if (sa_merge_blocks(merged_ary_fname, m_size_of_array_file, bl) == FAIL)
	return FAIL;

    sa_dprintf("sort blocks: rename \"%s\" with \"%s\"\n",
	       merged_ary_fname, ary_fname);
    if (rename(merged_ary_fname, ary_fname) != 0) {
	sa_error_no = FILE_OPEN_ERROR;
	return FAIL;
    }

    /* I have to free all *bl */
    sa_free(bl);

    return SUCCESS;
}


/*
 * Sort index-pointers.
 */
SA_STAT
sa_sort_index(const char *txt_fname, char *ary_fname)
{
    SA_MMAP *mmtxt, *mmary;
    char fname_tmp[SA_FILE_NAME_MAX];

    if (ary_fname == NULL || ary_fname[0] == '\0')
	ary_fname = sa_add_suffix_to_file_name(fname_tmp, txt_fname, "ary");

    mmtxt = sa_open_mmap(txt_fname, SA_MMAP_RO);
    mmary = sa_open_mmap(ary_fname, SA_MMAP_RW);

    if (mmtxt == NULL || mmary == NULL) {
	sa_error_no = MMAP_ERROR;
	return FAIL;
    }

    m_text_ptr = sa_get_mmap_ptr(mmtxt);
    m_text_last_ptr = m_text_ptr + sa_get_mmap_size(mmtxt);
    m_array_ptr = (SA_INDEX*)sa_get_mmap_ptr(mmary);
    m_size_of_array_file = sa_get_mmap_size(mmary) / sizeof(SA_INDEX);

    if (sa_mki_mode & SA_VERBOSE)
	printf("Sorting index file \"%s\"\n", ary_fname);

    {
	clock_t before;
        double elapsed;

        before = clock();
	if (m_memory_size == 0) {
	    sa_dprintf("multikey qsort\n");
	    sa_sort_index_multikey_qsort();
	} else if (m_memory_size == -1) {
	    sa_dprintf("system qsort\n");
	    sa_sort_index_system_qsort();
	} else {
	    sa_dprintf("divede and merge\n");
	    sa_sort_index_divide_and_merge(ary_fname);
	}

        elapsed = clock() - before;
	sa_dprintf("%.3f seconds\n", elapsed / CLOCKS_PER_SEC);
    }

    sa_close_mmap(mmary);
    sa_close_mmap(mmtxt);

    return SUCCESS;
}


/*
 * Sort each block
 */
static SA_STAT
sa_sort_each_block(SA_INDEX bl_size, SA_INDEX num_of_indexes, BLOCK **bl)
{
    SA_INDEX j;

    sa_dprintf("sort each block: block size = %ld\n", bl_size);

    m_progress_counter = 0;
    for (j = 0; j < num_of_indexes; j += bl_size) {
	BLOCK* tmp = sa_malloc(sizeof(BLOCK));
	SA_INDEX size = bl_size;

	if (j + bl_size > num_of_indexes)
	    size = num_of_indexes - j;

	tmp->aryidx = j;
	tmp->max = tmp->aryidx + size;

	if (sa_mki_mode & SA_VERBOSE)
	    printf(" Block %d (%ld -> %ld)\n",
		   (int)(j / bl_size) + 1, tmp->aryidx, tmp->max);

	multikey_qsort(m_array_ptr + tmp->aryidx, m_text_ptr, size, 0);
	if (sa_mki_mode & SA_VERBOSE)
	    printf("\n");

	{
	    SA_INDEX aa = m_array_ptr[tmp->aryidx];
	    strncpy(tmp->strbuf, m_text_ptr + aa, STR_CACHE_SIZE);
	}

	tmp->strbuf[STR_CACHE_SIZE] = '\0';
	insertheap(bl, tmp);

    }

    return SUCCESS;
}    


/*
 * Merge all blocks into an index file.
 */
static SA_STAT
sa_merge_blocks(char *merged_ary_fname, SA_INDEX num_of_indexes, BLOCK **bl)
{
    int i;
    FILE* mfd;

    if (sa_mki_mode & SA_VERBOSE) 
	printf("Merging indexes\n");

    if ((mfd = fopen(merged_ary_fname, "wb")) == NULL) {
	sa_error_no = FILE_OPEN_ERROR;
	return FAIL;
    }

    for (i = 0; i < num_of_indexes; i++) {
	BLOCK* tmp = bl[1];

	sa_fwrite(m_array_ptr[tmp->aryidx], mfd);

	if (sa_mki_mode & SA_VERBOSE)
	    print_progress_meter(i, m_size_of_array_file);

	(tmp->aryidx)++;
	if (tmp->aryidx >= tmp->max) {
	    hremove(bl);
	    continue;
	}

	strncpy(tmp->strbuf, m_text_ptr + m_array_ptr[tmp->aryidx],
		STR_CACHE_SIZE);

	tmp->strbuf[STR_CACHE_SIZE] = '\0';
	downheap(bl, 1);
    }

    if (sa_mki_mode & SA_VERBOSE) {
	print_progress_meter(m_size_of_array_file, m_size_of_array_file);
	printf("\n");
    }
    
    fclose(mfd);

    return SUCCESS;
}


/*
 * Heap
 *   Reference: R. Sedgewick 著 野下浩平、星守、佐藤創、田口東 共訳, 
 *     "アルゴリズム (Algorithms) 原書第2版 第1巻 基礎・整列", 近代科学社
 */
static int
two_level_compare(const BLOCK *a, const BLOCK *b)
{
    int r = strcmp(a->strbuf, b->strbuf);
    if (r == 0) {
	SA_INDEX aa = m_array_ptr[a->aryidx];
	SA_INDEX bb = m_array_ptr[b->aryidx];
	return strcmp(m_text_ptr + aa + STR_CACHE_SIZE,
		      m_text_ptr + bb + STR_CACHE_SIZE);
    }
    return r;
}

static void
upheap(BLOCK **a, int k)
{
    BLOCK *v;
    v = a[k];
    while (k > 1 &&
	   suffix_compare(m_array_ptr + a[k / 2]->aryidx,
			  m_array_ptr + v->aryidx) > 0) {
	a[k] = a[k / 2];
	k = k / 2;
    }
    a[k] = v;
}

static void
insertheap(BLOCK **a, BLOCK *v)
{
    m_size_of_heap++;
    a[m_size_of_heap] = v;
    upheap(a, m_size_of_heap);
}

static void
downheap(BLOCK **a, int k)
{
    int j;
    BLOCK *v;
    v = a[k];
    while (k <= m_size_of_heap / 2) {
	j = k * 2;
	if (j < m_size_of_heap && two_level_compare(a[j], a[j + 1]) >= 0)
 	    j++;
	if (two_level_compare(v, a[j]) <= 0)
	    break;
	a[k] = a[j];
	k = j;
    }
    a[k] = v;
}

static BLOCK *
hremove(BLOCK **a)
{
    BLOCK *r = a[1];
    a[1] = a[m_size_of_heap];
    m_size_of_heap--;
    downheap(a, 1);
    return r;
}


/*
 * Scan a text file and write indexes to a array file
 */
SA_STAT
sa_write_index(const char *txt_fname, char *ary_fname,
	       SA_INDEX (*get_next_ip_func)(SA_STRING, SA_INDEX))
{
    SA_INDEX i;
    FILE *ary_file_fd = NULL;

    SA_MMAP *mmr;
    char fname_tmp[SA_FILE_NAME_MAX];

    SA_STRING txtstr;
    SA_INDEX size_of_text_file;


    if (ary_fname == NULL || ary_fname[0] == '\0')
	ary_fname = sa_add_suffix_to_file_name(fname_tmp, txt_fname, "ary");

    if ((ary_file_fd = fopen(ary_fname, "wb")) == NULL) {
	sa_error_no = FILE_OPEN_ERROR;
	return FAIL;
    }

    mmr = sa_open_mmap(txt_fname, SA_MMAP_RO);
    if (mmr == NULL) {
	sa_error_no = MMAP_ERROR;
	return FAIL;
    }

    size_of_text_file = sa_get_mmap_size(mmr);
    m_text_ptr = sa_get_mmap_ptr(mmr);
    txtstr.len = sa_get_mmap_size(mmr);
    txtstr.ptr = sa_get_mmap_ptr(mmr);
    /* printf("%ld %ld\n", txtstr.len, size_of_text_file); */


    if (sa_mki_mode & SA_VERBOSE)
	printf("Reading text file \"%s\"\n", txt_fname);

    if (sa_mki_mode & SA_CUT_TOP)
	i = 0;
    else
	i = -1;

    for (; i < size_of_text_file;) {

	if (sa_mki_mode & SA_VERBOSE)
	    print_progress_meter(i, size_of_text_file);

	i = get_next_ip_func(txtstr, i);
	
	if (i < size_of_text_file)
	    sa_fwrite(i, ary_file_fd);
    }

    if (sa_mki_mode & SA_VERBOSE) {
	print_progress_meter(size_of_text_file, size_of_text_file);
	printf("\n");
    }

    (void)fclose(ary_file_fd);
    sa_close_mmap(mmr);

    return SUCCESS;
}


/*
 * Multikey Quicksort:
 * Jon L. Bentley, Robert Sedgewick. "Fast Algorithms for Sorting and
 * Searching Strings," Proceedings of the Eighth Annual ACM-SIAM
 * Symposium on Discrete Algorithms, 1997.
 * <http://www.cs.princeton.edu/~rs/strings/>
 */
#define sa_min(a, b) ((a)<=(b) ? (a) : (b))
#define swap(x, a, b) { SA_INDEX t = x[a]; x[a] = x[b]; x[b] = t; }

static void 
swap_vector(SA_INDEX i, SA_INDEX j, SA_INDEX n, SA_INDEX *x)
{   
    while (n-- > 0) {
        swap(x, i, j);
        i++;
        j++;
    }
}

static unsigned char
get_char_inside_text(char *ptr)
{
    /* printf("%x %x %x 000\n",ptr, m_text_last_ptr, (ptr- m_text_last_ptr)); */
    if (ptr - m_text_last_ptr >= 0) {
	return 0;
    }
    return (unsigned char)*ptr;
}

static void
multikey_qsort(SA_INDEX *x, char *txt, SA_INDEX n, SA_INDEX depth)
{
    SA_INDEX a, b, c, d;
    int r, v;
/*
  if (n <= 1)
  return;
*/

    if (n <= 1000) {
	/* trick for speed up */
	/* m_text_ptr += depth; */
	qsort(x, (size_t)n, sizeof(SA_INDEX),
	      (int (*)(const void *, const void *))suffix_compare);
	/* m_text_ptr -= depth; */
	if (sa_mki_mode & SA_VERBOSE) {
	    m_progress_counter += n;
	    print_progress_meter(m_progress_counter, m_size_of_array_file);
	}
	return;
    }

    a = rand() % n;
    swap(x, 0, a);
    v = get_char_inside_text(txt + x[0] + depth);
    /* assert(v == (unsigned char)((txt + x[0])[depth])); */
    a = b = 1;
    c = d = n-1;
    for (;;) {
        while (b <= c && (r = 
			  get_char_inside_text(txt + x[b] + depth)
			  -v) <= 0) {
	    /* assert(r+v == (unsigned char)((txt + x[b])[depth])); */
            if (r == 0) {
                swap(x, a, b);
                a++;
            }
            b++;
        }
        while (b <= c && (r =
			  get_char_inside_text(txt + x[c] + depth)
			  -v) >= 0) {
	    /* assert(r+v == (unsigned char)((txt + x[c])[depth])); */
            if (r == 0) {
                swap(x, c, d);
                d--;
            }
            c--;
        }
        if (b > c)
            break;
        swap(x, b, c);
        b++;
        c--;
    }
    r = sa_min(a, b-a);
    swap_vector(0, b-r, r, x);
    r = sa_min(d-c, n-d-1);
    swap_vector(b, n-r, r, x);
    r = b-a;
    multikey_qsort(x, txt, r, depth);
    if ((txt + x[r])[depth] != '\0')
        multikey_qsort(x + r, txt, a + n-d-1, depth+1);
    r = d-c;
    multikey_qsort(x + n-r, txt, r, depth);
}


/*
 * array ファイルがソートされているかどうか調べる
 */
int
sa_is_sorted(const SUFARY *ary)
{
    SA_INDEX i;
    SA_INDEX max = sa_get_array_size(ary);
    char *pre_str = sa_aryidx2txtptr(ary, 0);

    m_text_last_ptr = sa_get_text_ptr(ary) + sa_get_text_size(ary);
    if (sa_mki_mode & SA_VERBOSE)
	print_progress_meter(0, max);
    for (i = 1; i < max; i++) {
	char *now_str = sa_aryidx2txtptr(ary, i);

	if (sa_mki_mode & SA_VERBOSE)
	    print_progress_meter(i, max);
	if (sa_strcmp(
/* sa_get_text_ptr(ary) + sa_get_text_size(ary), */
		      pre_str, now_str) > 0) {
	    if (sa_mki_mode & SA_VERBOSE) {
		printf("\n");
		printf("%ld\n", (long int)i);
		printf("last = %x, %x %x\n", 
		       (int)(sa_get_text_ptr(ary) + sa_get_text_size(ary)),
		       (int)pre_str, (int)now_str);
		printf("[[[%x]]]\n", pre_str[0]);
		printf("(((%x)))\n", now_str[0]);
		printf("***%d***\n", pre_str[0] - now_str[0]);
		printf("===%d===\n",
		       sa_strcmp(
/* sa_get_text_ptr(ary) + sa_get_text_size(ary), */
				 pre_str, now_str));
	    }
	    return 0;
	}
	pre_str = now_str;
    }
    if (sa_mki_mode & SA_VERBOSE) {
	print_progress_meter(i, max);
	printf("\n");
    }

    return 1;
}


/*
 * suffix compare within text
 *
 * return value
 *   0 : MATCH
 *   - : LESS   p1 < p2  ('abc...' < 'ccc...')
 *   + : ABOVE  p1 > p2  ('abc...' > 'aaa...')
 */
static int
sa_strcmp(const char *p1, const char *p2)
{
    size_t i;
    size_t len;

    assert(p1 != NULL && p2 != NULL);
    assert(p1 != p2);
    assert(m_text_last_ptr - p1 > 0 && m_text_last_ptr - p2 > 0);

    len = sa_min(m_text_last_ptr - p1, m_text_last_ptr - p2);
    assert(0 <= len);

    for (i = 0; i < len; i++) {
        if (*p1 != *p2)
            return ((unsigned char)*p1 - (unsigned char)*p2);
        p1++;
        p2++;
    }

	/*
	 * ^-------- p2 ------- p1 --$   -   p1 < p2 ('hoho$' > 'hohohoho$')
	 * ^----- p1 ------ p2 ------$   +   p1 > p2 ('hohohoho$' > 'hoho$')
	 */
    return (p1 - p2 > 0) ? -4649 : 4649;
}


/*
 * print all suffixes by alphabetical order
 */
#define DUMP_LINE_LEN 55
void
sa_dump_all_suffixes(const SUFARY *ary)
{
    SA_INDEX i;
    SA_INDEX max = sa_get_array_size(ary);
    char tmp[DUMP_LINE_LEN + 1];

    for (i = 0; i < max; i++) {
	char *now_str = sa_aryidx2txtptr(ary, i);
	int j;
	int tail_count = 0;
	int len = sa_get_text_size(ary) - sa_aryidx2txtidx(ary, i);
	len = len < DUMP_LINE_LEN ? len : DUMP_LINE_LEN;
	for (j = 0; j < len; j++) {
	    tmp[j] = (unsigned char)now_str[j] < 0x20 ? '!' : now_str[j];
	}

	/* 末尾文字化けつぶし from here */
	for (j = len - 1; j >= 0; j--) {
	    if (tmp[j] & 0x80)
		tail_count++;		
	    else 
		break;
	}
	if (tail_count % 2 == 1)
	    tmp[len - 1] = '%';
	/* 末尾文字化けつぶし to here */

	printf(" %8ld : %8ld : %.*s\n", (long)i,
	       (long)sa_aryidx2txtidx(ary, i), len, tmp);
    }
}

