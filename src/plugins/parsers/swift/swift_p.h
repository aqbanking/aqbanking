/***************************************************************************
 begin       : Fri Apr 02 2004
 copyright   : (C) 2004,2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCIBANK_SWIFT_P_H
#define AQHBCIBANK_SWIFT_P_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/fastbuffer.h>


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



struct AHB_SWIFT_SUBTAG {
  GWEN_LIST_ELEMENT(AHB_SWIFT_SUBTAG);
  int id;
  char *content;
};




/**
 * This function reads a line from a buffered IO stream. It stops when either
 * the line or the stream ends. The end of line is signalled by an LF
 * character or a series of two "@" characters (for historical reasons).
 */
int AHB_SWIFT_ReadLine(GWEN_FAST_BUFFER *fb,
                       char *buffer,
                       unsigned int s);

int AHB_SWIFT__ReadDocument(GWEN_FAST_BUFFER *fb,
			    AHB_SWIFT_TAG_LIST *tl,
                            unsigned int maxTags);


int AHB_SWIFT_ReadDocument(GWEN_FAST_BUFFER *fb,
			   AHB_SWIFT_TAG_LIST *tl,
			   unsigned int maxTags);


int AHB_SWIFT_Export(GWEN_DBIO *dbio,
		     GWEN_SYNCIO *sio,
		     GWEN_DB_NODE *data,
		     GWEN_DB_NODE *cfg,
		     uint32_t flags);

int AHB_SWIFT_Import(GWEN_DBIO *dbio,
		     GWEN_SYNCIO *sio,
		     GWEN_DB_NODE *data,
		     GWEN_DB_NODE *cfg,
		     uint32_t flags);

GWEN_DBIO_CHECKFILE_RESULT AHB_SWIFT_CheckFile(GWEN_DBIO *dbio,
                                               const char *fname);

GWEN_DBIO *GWEN_DBIO_SWIFT_Factory(GWEN_PLUGIN *pl);
GWEN_PLUGIN *dbio_swift_factory(GWEN_PLUGIN_MANAGER *pm,
                                const char *modName,
                                const char *fileName);



#endif /* AQHBCIBANK_SWIFT_P_H */

