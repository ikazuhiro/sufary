/*
 * $Id: index-point.c,v 1.2 2001/03/24 08:34:30 tatuo-y Exp $
 *
 * handling index-points
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sufary.h"
#include "util.h"
#include "config.h"
#include "make-index.h"


static char *ignored_chars = NULL;
static char *my_chars = NULL;
static char *my_delimiters = NULL;

/*
 * access functions
 */
void
sa_set_ignored_chars(char *p)
{
    ignored_chars = p;
}

char *
sa_get_ignored_chars(void)
{
    return ignored_chars;
}

void
sa_set_my_chars(char *p)
{
    my_chars = p;
}

char *
sa_get_my_chars(void)
{
    return my_chars;
}

void
sa_set_my_delimiters(char *p)
{
    my_delimiters = p;
}

char *
sa_get_my_delimiters(void)
{
    return my_delimiters;
}



static int
sa_is_skip(SA_STRING sstr, SA_INDEX ip)
{
    if (sa_get_ignored_chars() != NULL &&
	strchr(sa_get_ignored_chars(), sstr.ptr[ip]) != NULL)
	return 1;
    else if ((sa_mki_mode & SA_IP_00_7F) && MSB_ON(sstr.ptr[ip]))
	return 1;
    else if ((sa_mki_mode & SA_IP_80_FF) && !MSB_ON(sstr.ptr[ip]))
	return 1;
    else
        return 0;
}


extern SA_INDEX
sa_get_next_ip_after_delimiter(SA_STRING sstr, SA_INDEX ip)
{
    assert(-1 <= ip && ip < (SA_INDEX)sstr.len);

    /* top of file problem: complicated! */
    if (ip == -1)
	return 0;

    for ( ; ip < (SA_INDEX)sstr.len; ip++) {
	if (strchr(sa_get_my_delimiters(), sstr.ptr[ip]) != NULL) {

	    /* skip a sequence of delimiters */
	    for ( ; ip < (SA_INDEX)sstr.len; ip++) {
		if (strchr(sa_get_my_delimiters(), sstr.ptr[ip]) == NULL)
		    break;
	    }
	    if (ip == (SA_INDEX)sstr.len)
		return sstr.len;
	    if (! sa_is_skip(sstr, ip))
		return ip;
	}
    }
    return sstr.len;		/* fail */
}


extern SA_INDEX
sa_get_next_ip_line(SA_STRING sstr, SA_INDEX ip)
{
    sa_set_my_delimiters("\n");
    return sa_get_next_ip_after_delimiter(sstr, ip);
}


extern SA_INDEX
sa_get_next_ip_word(SA_STRING sstr, SA_INDEX ip)
{
    sa_set_my_delimiters(" \t\n\r\f{}.()~-`'[]");
    return sa_get_next_ip_after_delimiter(sstr, ip);
}


static SA_INDEX
sa_skip_one_char(SA_STRING sstr, SA_INDEX ip)
{
    if (MSB_ON(sstr.ptr[ip]))
	return ip + 2;
    else 
	return ip + 1;
}


static int
sa_is_acceptable(SA_STRING sstr, SA_INDEX ip)
{
    if (sa_get_my_chars() != NULL
	&& strchr(sa_get_my_chars(), sstr.ptr[ip]) != NULL)
        return 1;
    else
        return 0;
}


extern SA_INDEX
sa_get_next_ip_char(SA_STRING sstr, SA_INDEX ip)
{
    assert(-1 <= ip && ip < (SA_INDEX)sstr.len);
    ip = (ip == -1) ? 0 : sa_skip_one_char(sstr, ip);
    for ( ; ip < (SA_INDEX)sstr.len; ip = sa_skip_one_char(sstr, ip)) {
	if (sa_is_acceptable(sstr, ip) || ! sa_is_skip(sstr, ip))
  	    return ip;
    }
    return sstr.len;		/* fail */
}


extern SA_INDEX
sa_get_next_ip_byte(SA_STRING sstr, SA_INDEX ip)
{
    assert(-1 <= ip && ip < (SA_INDEX)sstr.len);
    ip++;
    for ( ; ip < (SA_INDEX)sstr.len; ip++) {
	if (sa_is_acceptable(sstr, ip) || ! sa_is_skip(sstr, ip))
  	    return ip;
    }
    return sstr.len;		/* fail */
}


/*
 * Excerpt from RFC 2044:
 *
 * UCS-4 range (hex.)    UTF-8 octet sequence (binary)
 * 0000 0000-0000 007F   0xxxxxxx
 * 0000 0080-0000 07FF   110xxxxx 10xxxxxx
 * 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
 * 
 * 0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 0400 0000-7FFF FFFF   1111110x 10xxxxxx ... 10xxxxxx
 *
 * ref. sary-1.0.0 (http://sary.namazu.org/)
 */
static SA_INDEX sa_skip_one_char_utf8(SA_STRING sstr, SA_INDEX ip)
{
    unsigned char cur = sstr.ptr[ip];
    SA_INDEX eof = sstr.len;
    if (cur < 0x80) {
	return ip + 1;
    } else if ((ip + 1 < eof) && (cur & 0xe0) == 0xc0) {
	return ip + 2;
    } else if ((ip + 2 < eof) && (cur & 0xf0) == 0xe0) {
	return ip + 3;
    } else if ((ip + 3 < eof) && (cur & 0xf8) == 0xf0) {
	return ip + 4;
    } else if ((ip + 4 < eof) && (cur & 0xfc) == 0xf8) {
	return ip + 5;
    } else if ((ip + 6 < eof) && (cur & 0xfe) == 0xfc) {
	return ip + 4;
    } else {
        /* invalid character */
//        printf("invalid character at %d", cursor - bof);
	assert("utf8 invalid character" && 0);
	return 1;
    }
}

extern SA_INDEX
sa_get_next_ip_char_utf8(SA_STRING sstr, SA_INDEX ip)
{
    assert(-1 <= ip && ip < (SA_INDEX)sstr.len);
    ip = (ip == -1) ? 0 : sa_skip_one_char_utf8(sstr, ip);
    for ( ; ip < (SA_INDEX)sstr.len; ip = sa_skip_one_char_utf8(sstr, ip)) {
	if (sa_is_acceptable(sstr, ip) || ! sa_is_skip(sstr, ip))
  	    return ip;
    }
    return sstr.len;		/* fail */
}
