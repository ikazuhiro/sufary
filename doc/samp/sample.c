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
        for (tmp = sr.left; tmp <= sr.right; tmp++) { /* '<=' ����� */
            SA_INDEX pos = sa_aryidx2txtidx(ary, tmp);
	    char *txt_ptr;
            printf("���ƥ����ȥե������ %ld ʸ���ܤ���ޥå�\n", pos);
            printf("����Ƭ��ʸ��ɽ��(1)\n%.*s\n", 2, sa_aryidx2txtptr(ary, tmp));
            txt_ptr = sa_get_string(ary, pos, 2);
            printf("����Ƭ��ʸ��ɽ��(2)\n%s\n", txt_ptr);
            free(txt_ptr);
            txt_ptr = sa_get_line(ary, pos);
            printf("�����ɽ��\n%s\n", txt_ptr);
            free(txt_ptr);
            txt_ptr = sa_get_lines(ary, pos, 1, 1);
            printf("������ΰ�Ԥ�ɽ��\n%s\n", txt_ptr);
            free(txt_ptr);
            txt_ptr = sa_get_block(ary, pos, "<DOC>", "</DOC>");
            printf("��DOC �����˰Ϥޤ줿�ΰ��ɽ��\n%s\n", txt_ptr);
            free(txt_ptr);
        }
    }

    sa_close(ary);
    return 0;
}
