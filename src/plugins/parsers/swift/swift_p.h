/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Fri Apr 02 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCIBANK_SWIFT_P_H
#define AQHBCIBANK_SWIFT_P_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/dbio.h>


#define AHB_SWIFT_MAXLINELEN 512

#include "swift_l.h"


struct AHB_SWIFT_TAG {
  GWEN_LIST_ELEMENT(AHB_SWIFT_TAG);
  char *id;
  char *content;
};



AHB_SWIFT_TAG *AHB_SWIFT_Tag_new(const char *id,
				 const char *content);
void AHB_SWIFT_Tag_free(AHB_SWIFT_TAG *tg);

/**
 * This function reads a line from a buffered IO stream. It stops when either
 * the line or the stream ends. The end of line is signalled by an LF
 * character or a series of two "@" characters (for historical reasons).
 */
int AHB_SWIFT_ReadLine(GWEN_BUFFEREDIO *bio,
                       char *buffer,
                       unsigned int s);

int AHB_SWIFT_ReadDocument(GWEN_BUFFEREDIO *bio, AHB_SWIFT_TAG_LIST *tl);



int AHB_SWIFT_Export(GWEN_DBIO *dbio,
		     GWEN_BUFFEREDIO *bio,
		     GWEN_TYPE_UINT32 flags,
		     GWEN_DB_NODE *data,
		     GWEN_DB_NODE *cfg);

int AHB_SWIFT_Import(GWEN_DBIO *dbio,
		     GWEN_BUFFEREDIO *bio,
		     GWEN_TYPE_UINT32 flags,
		     GWEN_DB_NODE *data,
		     GWEN_DB_NODE *cfg);



#endif /* AQHBCIBANK_SWIFT_P_H */

