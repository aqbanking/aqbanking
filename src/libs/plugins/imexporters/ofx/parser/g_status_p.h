/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_STATUS_P_H
#define AIO_OFX_G_STATUS_P_H


#include "g_status_l.h"


typedef struct AIO_OFX_GROUP_STATUS AIO_OFX_GROUP_STATUS;
struct AIO_OFX_GROUP_STATUS {
  char *description;
  int code;
  char *severity;

  char *currentElement;

  AIO_OFX_GROUP_ENDTAG_FN oldEndTagFn;
};

static void GWENHYWFAR_CB AIO_OfxGroup_STATUS_FreeData(void *bp, void *p);


static int AIO_OfxGroup_STATUS_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_STATUS_EndTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_STATUS_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


