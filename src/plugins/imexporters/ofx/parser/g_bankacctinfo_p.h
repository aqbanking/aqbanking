/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_BANKACCTINFO_P_H
#define AIO_OFX_G_BANKACCTINFO_P_H


#include "g_bankacctinfo_l.h"


typedef struct AIO_OFX_GROUP_BANKACCTINFO AIO_OFX_GROUP_BANKACCTINFO;
struct AIO_OFX_GROUP_BANKACCTINFO {
  char *currentElement;
  char *bankId;
  char *accId;
  char *accType;
};

static void GWENHYWFAR_CB AIO_OfxGroup_BANKACCTINFO_FreeData(void *bp, void *p);


static int AIO_OfxGroup_BANKACCTINFO_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_BANKACCTINFO_EndSubGroup(AIO_OFX_GROUP *g,
						 AIO_OFX_GROUP *sg);
static int AIO_OfxGroup_BANKACCTINFO_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


