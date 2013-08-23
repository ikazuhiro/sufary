#include <stdio.h>
#include <stdlib.h>
#include "sufary.h"

int main(int argc, char *argv[])
{
    SUFARY *ary;
    SUF_RESULT sr;

    ary = sa_open(argv[2], NULL);
    if (ary == NULL)
        exit(1);

    sa_set_debug_mode(1);

    sr = sa_find(ary, 0, sa_get_array_size(ary) - 1, argv[1], strlen(argv[1]), 0);
    printf("%ld %ld\n", sr.left, sr.right);

    if (sr.stat == SUCCESS) {
        SA_INDEX tmp;
        for (tmp = sr.left; tmp <= sr.right; tmp++) { /* '<=' に注意 */
            SA_INDEX pos = sa_aryidx2txtidx(ary, tmp);
	    char *txt_ptr;
            printf("★テキストファイルの %ld 文字目からマッチ\n", pos);
            printf("★先頭二文字表示(1)\n%.*s\n", 2, sa_aryidx2txtptr(ary, tmp));
            txt_ptr = sa_get_string(ary, pos, 2);
            printf("★先頭二文字表示(2)\n%s\n", txt_ptr);
            free(txt_ptr);
            txt_ptr = sa_get_line(ary, pos);
            printf("★一行表示\n%s\n", txt_ptr);
            free(txt_ptr);
            txt_ptr = sa_get_lines(ary, pos, 1, 1);
            printf("★前後の一行も表示\n%s\n", txt_ptr);
            free(txt_ptr);
            txt_ptr = sa_get_block(ary, pos, "<DOC>", "</DOC>");
            printf("★DOC タグに囲まれた領域を表示\n%s\n", txt_ptr);
            free(txt_ptr);
        }
    }

    sa_close(ary);
    return 0;
}
