/***************************************************************************
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_BANKTRAN_P_H
#define AIO_OFX_G_BANKTRAN_P_H


#include "g_banktran_l.h"


typedef struct AIO_OFX_GROUP_BANKTRAN AIO_OFX_GROUP_BANKTRAN;
struct AIO_OFX_GROUP_BANKTRAN {
  char *currentElement;

  AB_TRANSACTION *transaction;
};

static void GWENHYWFAR_CB AIO_OfxGroup_BANKTRAN_FreeData(void *bp, void *p);


static int AIO_OfxGroup_BANKTRAN_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_BANKTRAN_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);
static int AIO_OfxGroup_BANKTRAN_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


