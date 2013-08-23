/*
 * $Id: DID.xs,v 1.9 2000/11/17 02:51:50 tatuo-y Exp $
 *
 * written by T.Nakayama
 */
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "sufary.h"
#include "did.h"
#include "util.h"

MODULE = DID PACKAGE = DID
PROTOTYPES: ENABLE

DID *
suf_opendid(s)
	char *s
	CODE:
	RETVAL = sa_open_did(s);
	OUTPUT:
	RETVAL

void
suf_closedid(did)
	DID *did
	CODE:
	sa_close_did(did);


void
suf_didsearch(did,tgt)
	DID *did
	off_t tgt
	PREINIT:
	DID_RESULT dr;
	PPCODE:
	dr = sa_didsearch(did, tgt);
    
	if (dr.stat == SUCCESS) {
		EXTEND(SP, 3);
		PUSHs(sv_2mortal(newSVnv(dr.no)));
		PUSHs(sv_2mortal(newSVnv(dr.start)));
		PUSHs(sv_2mortal(newSVnv(dr.size)));
	}


off_t
suf_did_size(did)
	DID *did
	CODE:
	RETVAL = sa_get_did_size(did);
	OUTPUT:
	RETVAL


int
suf_memory_leak_check()
	CODE:
	RETVAL = sa_memory_leak_check();
	OUTPUT:
	RETVAL
