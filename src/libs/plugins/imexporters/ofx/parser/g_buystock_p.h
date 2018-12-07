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


#ifndef AIO_OFX_G_BUYSTOCK_P_H
#define AIO_OFX_G_BUYSTOCK_P_H


#include "g_buystock_l.h"


typedef struct AIO_OFX_GROUP_BUYSTOCK AIO_OFX_GROUP_BUYSTOCK;
struct AIO_OFX_GROUP_BUYSTOCK {
  char *currentElement;

  AB_TRANSACTION *transaction;
};

static void GWENHYWFAR_CB AIO_OfxGroup_BUYSTOCK_FreeData(void *bp, void *p);


static int AIO_OfxGroup_BUYSTOCK_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_BUYSTOCK_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);
static int AIO_OfxGroup_BUYSTOCK_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


