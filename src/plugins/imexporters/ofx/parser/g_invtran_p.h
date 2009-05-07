/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Fri Apr 17 2009
 copyright   : (C) 2009 by Stephen R. Besch (C) 2008 by Martin Preuss
 email       : sbesch@buffalo.edu martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AIO_OFX_G_INVTRAN_P_H
#define AIO_OFX_G_INVTRAN_P_H

#include "g_invtran_l.h"

typedef struct AIO_OFX_GROUP_INVTRAN AIO_OFX_GROUP_INVTRAN;
struct AIO_OFX_GROUP_INVTRAN {
  char *currentElement;
  char *datum[iinvtranlastdatum];
};

static void GWENHYWFAR_CB AIO_OfxGroup_INVTRAN_FreeData(void *bp, void *p);
static int AIO_OfxGroup_INVTRAN_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_INVTRAN_AddData(AIO_OFX_GROUP *g, const char *data);

int AIO_OfxGroup_INVTRAN_SortTag(const char * s, const char ** sTags, int max);

#endif


