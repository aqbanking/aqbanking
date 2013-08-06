/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de
 copyright   : (C) 2013 by Paul Conrady
 email       : c.p.conrady@gmail.com

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_INVBUY_P_H
#define AIO_OFX_G_INVBUY_P_H


#include "g_invbuy_l.h"


typedef struct AIO_OFX_GROUP_INVBUY AIO_OFX_GROUP_INVBUY;
struct AIO_OFX_GROUP_INVBUY {
  char *currentElement;
  char *currency;

  AB_TRANSACTION *transaction;
};

static void GWENHYWFAR_CB AIO_OfxGroup_INVBUY_FreeData(void *bp, void *p);


static int AIO_OfxGroup_INVBUY_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_INVBUY_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);
static int AIO_OfxGroup_INVBUY_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


