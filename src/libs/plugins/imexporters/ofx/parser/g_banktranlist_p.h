/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_BANKTRANLIST_P_H
#define AIO_OFX_G_BANKTRANLIST_P_H


#include "g_banktranlist_l.h"


typedef struct AIO_OFX_GROUP_BANKTRANLIST AIO_OFX_GROUP_BANKTRANLIST;
struct AIO_OFX_GROUP_BANKTRANLIST {
  char *currentElement;

  char *dtstart;
  char *dtend;

  AB_TRANSACTION_LIST2 *transactionList;
};

static void GWENHYWFAR_CB AIO_OfxGroup_BANKTRANLIST_FreeData(void *bp, void *p);


static int AIO_OfxGroup_BANKTRANLIST_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_BANKTRANLIST_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);
static int AIO_OfxGroup_BANKTRANLIST_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


