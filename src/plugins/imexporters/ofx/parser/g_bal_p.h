/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_BAL_P_H
#define AIO_OFX_G_BAL_P_H


#include "g_bal_l.h"


typedef struct AIO_OFX_GROUP_BAL AIO_OFX_GROUP_BAL;
struct AIO_OFX_GROUP_BAL {
  AB_VALUE *value;
  GWEN_TIME *date;

  char *currentElement;
};

static void GWENHYWFAR_CB AIO_OfxGroup_BAL_FreeData(void *bp, void *p);


static int AIO_OfxGroup_BAL_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_BAL_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


