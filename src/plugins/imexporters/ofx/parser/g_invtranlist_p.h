/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de
 begin       : Fri Apr 17 2009
 copyright   : (C) 2009 by Stephen R. Besch
 email       : sbesch@buffalo.edu
 begin       : Sat May 18 2013
 copyright   : (C) 2013 by Paul Conrady
 email       : c.p.conrady@gmail.com

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AIO_OFX_G_INVTRANLIST_P_H
#define AIO_OFX_G_INVTRANLIST_P_H

#include "g_invtranlist_l.h"

typedef struct AIO_OFX_GROUP_INVTRANLIST AIO_OFX_GROUP_INVTRANLIST;
struct AIO_OFX_GROUP_INVTRANLIST {
  char *currentElement;
  char *dtstart;
  char *dtend;
  AB_TRANSACTION_LIST2 *transactionList;
};

static void GWENHYWFAR_CB AIO_OfxGroup_INVTRANLIST_FreeData(void *bp, void *p);
static int AIO_OfxGroup_INVTRANLIST_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_INVTRANLIST_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);
static int AIO_OfxGroup_INVTRANLIST_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


