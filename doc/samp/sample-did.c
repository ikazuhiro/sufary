#include <stdio.h>
#include <stdlib.h>
#include "sufary.h"

int main(int argc, char *argv[]) 
{
    SUFARY *ary;
    DID *did;
    SUF_RESULT sr;

    ary = sa_open(argv[2], NULL);
    did = sa_open_did(argv[3]);
    if (ary == NULL || did == NULL)
        exit(1);

    printf("★いくつあるかな ... %ld\n", sa_get_did_size(did));

    sr = sa_find(ary, 0, sa_get_array_size(ary) - 1, argv[1], strlen(argv[1]), 0);
    if (sr.stat == SUCCESS) {
        SA_INDEX tmp;
        for (tmp = sr.left; tmp <= sr.right; tmp++) { /* '<=' に注意 */
            SA_INDEX pos = sa_aryidx2txtidx(ary, tmp);
            DID_RESULT dr = sa_didsearch(did, pos);
            if (dr.stat == FAIL)
                continue;
            printf("★ no. %ld, start %ld, size %ld\n",
                   dr.no, dr.start, dr.size);
            printf("★テキストエリア表示(1)\n");
            printf("%.*s\n", dr.size, sa_txtidx2txtptr(ary, dr.start));
            printf("★テキストエリア表示(2)\n");
            printf("%.*s\n", dr.size,
                   sa_txtidx2txtptr(ary, sa_get_start_position(did, dr.no)));
        }
    }

    sa_close_did(did);
    sa_close(ary);
    return 0;
} 
