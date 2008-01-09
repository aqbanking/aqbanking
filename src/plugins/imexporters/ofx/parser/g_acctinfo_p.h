/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_ACCTINFO_P_H
#define AIO_OFX_G_ACCTINFO_P_H


#include "g_acctinfo_l.h"


typedef struct AIO_OFX_GROUP_ACCTINFO AIO_OFX_GROUP_ACCTINFO;
struct AIO_OFX_GROUP_ACCTINFO {
  char *currentElement;
  char *description;
  char *bankId;
  char *accId;
  char *accType;

  AB_IMEXPORTER_ACCOUNTINFO *accountInfo;
};

static void GWENHYWFAR_CB AIO_OfxGroup_ACCTINFO_FreeData(void *bp, void *p);


static int AIO_OfxGroup_ACCTINFO_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_ACCTINFO_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);
static int AIO_OfxGroup_ACCTINFO_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


