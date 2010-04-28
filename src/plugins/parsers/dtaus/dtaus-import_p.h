/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Apr 29 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQHBCIBANK_DTAUS_IMPORT_P_H
#define AQHBCIBANK_DTAUS_IMPORT_P_H

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/dbio.h>

#include <aqbanking/value.h>


int AHB_DTAUS__SetCharValue(GWEN_DB_NODE *db,
			    uint32_t flags,
                            const char *name,
                            const char *s);

int AHB_DTAUS__ReadWord(GWEN_BUFFER *src,
                        GWEN_BUFFER *dst,
                        unsigned int pos,
                        unsigned int size);
int AHB_DTAUS__ParseExtSet(GWEN_BUFFER *src,
                           unsigned int pos,
                           GWEN_DB_NODE *xa);

/**
 * Completes the given template DB node.
 * @return -1 on error, size of A-set if ok
 */
int AHB_DTAUS__ParseSetA(GWEN_BUFFER *src,
                         unsigned int pos,
                         GWEN_DB_NODE *xa);

/**
 * @return -1 on error, size of C-set if ok
 */
int AHB_DTAUS__ParseSetC(GWEN_BUFFER *src,
                         unsigned int pos,
                         GWEN_DB_NODE *xa,
			 AB_VALUE *sumEUR,
                         AB_VALUE *sumDEM,
                         AB_VALUE *sumBankCodes,
                         AB_VALUE *sumAccountIds);

/**
 * @return -1 on error, size of E-set if ok
 */
int AHB_DTAUS__ParseSetE(GWEN_BUFFER *src,
                         unsigned int pos,
                         unsigned int csets,
                         AB_VALUE *sumEUR,
                         AB_VALUE *sumDEM,
			 AB_VALUE *sumBankCodes,
                         AB_VALUE *sumAccountIds);

/**
 * @return -1 on error, size of DTAUS record if ok
 */
int AHB_DTAUS__ReadDocument(GWEN_BUFFER *src,
                            unsigned int pos,
                            GWEN_DB_NODE *cfg);

int AHB_DTAUS__Import(GWEN_DBIO *dbio,
		      GWEN_SYNCIO *sio,
		      GWEN_DB_NODE *data,
		      GWEN_DB_NODE *cfg,
                      uint32_t flags);




#endif /* AQHBCIBANK_DTAUS_IMPORT_P_H */

