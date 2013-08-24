/*
 * $Id: mmp.c,v 1.4 2000/10/28 05:17:21 tatuo-y Exp $
 *
 * memory mapped file
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <fcntl.h>

#ifdef _SUFARY_H_
#include "config.h"
#endif

#include "mmp.h"
#include "my-malloc.h"



#ifdef WIN32
typedef struct {
    HANDLE hFile;
    HANDLE hMap;
} MMAP_HANDLES;
#endif


static int
sa_open_mmap_aux(SA_MMAP *mm, const char *fname, const SA_MMAP_MODE mode);


/* 
 * access functions
 */
off_t
sa_get_mmap_size(const SA_MMAP *mmr)
{
    return mmr->size;
}

void *
sa_get_mmap_ptr(const SA_MMAP *mmr)
{
    return mmr->map;
}


/*
 * open and map a file
 */
SA_MMAP *
sa_open_mmap(const char *fname, const SA_MMAP_MODE mode)
{
    SA_MMAP *mm; 

    mm = sa_malloc(sizeof(SA_MMAP));
    if (mm == NULL)
        return NULL;

    if (sa_open_mmap_aux(mm, fname, mode)) {
      sa_free(mm);
      return NULL;
    }

    return mm;
}


#ifdef WIN32
int
sa_open_mmap_aux(SA_MMAP *mm, const char *fname, const SA_MMAP_MODE mode)
{
    MMAP_HANDLES mmp_handles;
    unsigned long mode1;
    unsigned long mode2;
    unsigned long mode3;
    unsigned long mode4;

    /* set parameters */
    if (mode == SA_MMAP_RO) {
	mode1 = GENERIC_READ;
	mode2 = PAGE_READONLY;
	mode3 = FILE_MAP_READ;
	mode4 = FILE_SHARE_READ;
    } else if (mode == SA_MMAP_RW) {
	mode1 = GENERIC_READ | GENERIC_WRITE;
	mode2 = PAGE_READWRITE;
	mode3 = FILE_MAP_ALL_ACCESS;
	mode4 = 0;
    } else {
	return 1;
    }
 
    mm->other = sa_malloc(sizeof(MMAP_HANDLES));
    if (mm->other == NULL)
        return 1;

    mmp_handles.hFile = CreateFile(fname, mode1, mode4, NULL,
				   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (mmp_handles.hFile == INVALID_HANDLE_VALUE)
        return 1;
    mm->size = GetFileSize(mmp_handles.hFile, NULL);

    mmp_handles.hMap = CreateFileMapping(mmp_handles.hFile, NULL,
					 mode2, 0, 0, NULL);
    if (mmp_handles.hMap == NULL) {
	CloseHandle(mmp_handles.hFile);
	return 1;
    }
				
    mm->map = MapViewOfFile(mmp_handles.hMap, mode3, 0, 0, 0);
    if (mm->map == NULL) {
	CloseHandle(mmp_handles.hFile);
	CloseHandle(mmp_handles.hMap);
	return 1;
    }

    ((MMAP_HANDLES *)(mm->other))->hFile = mmp_handles.hFile;
    ((MMAP_HANDLES *)(mm->other))->hMap = mmp_handles.hMap;

    return 0;
}
#else
int
sa_open_mmap_aux(SA_MMAP *mm, const char *fname, const SA_MMAP_MODE mode)
{
    int fd;
    struct stat stat_buf;
    int oflag;
    int prot;

    /* set parameters */
    if (mode == SA_MMAP_RO) {
	oflag = O_RDONLY;
	prot = PROT_READ;
    } else if (mode == SA_MMAP_RW) {
	oflag = O_RDWR;
	prot = PROT_READ | PROT_WRITE;
    } else {
	return 1;
    }
    
    /* open the addressed file */
    fd = open(fname, oflag);
    if (fd == -1) {
	return 1;
    }

    /* size of the file */
    if (fstat(fd, &stat_buf) != 0) {
        close(fd);
        return 1;
    }

    mm->size = stat_buf.st_size;
    mm->other = NULL;

    /* ready to load the file */
    mm->map = mmap((caddr_t)0, mm->size, prot, MAP_SHARED, fd, 0);
    close(fd);
    if (mm->map == (caddr_t) -1) {
	return 1;
    }

    return 0;
}
#endif



/*
 * un-mmap and close a file
 */
#ifdef WIN32
void
sa_close_mmap(SA_MMAP *mms)
{
    if (mms != NULL) {
	UnmapViewOfFile(mms->map);
	CloseHandle(((MMAP_HANDLES *)(mms->other))->hMap);
	CloseHandle(((MMAP_HANDLES *)(mms->other))->hFile);
	if (mms->other != NULL)
	  sa_free((MMAP_HANDLES *)(mms->other));
	sa_free(mms);
    }
}
#else
void
sa_close_mmap(SA_MMAP *mms)
{
    if (mms != NULL) {
	munmap(mms->map, mms->size);
	sa_free(mms);
    }
}
#endif
