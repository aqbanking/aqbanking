/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_BANKACC_P_H
#define AIO_OFX_G_BANKACC_P_H


#include "g_bankacc_l.h"


typedef struct AIO_OFX_GROUP_BANKACC AIO_OFX_GROUP_BANKACC;
struct AIO_OFX_GROUP_BANKACC {
  char *bankId;
  char *accId;
  char *accType;

  char *currentElement;
};

static void GWENHYWFAR_CB AIO_OfxGroup_BANKACC_FreeData(void *bp, void *p);


static int AIO_OfxGroup_BANKACC_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_BANKACC_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


