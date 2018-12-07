/***************************************************************************
 $RCSfile$
 -------------------
begin       : Thur Apr 23 2009
copyright   : (C) 2009 by Stephen R. Besch (C) 2008 by Martin Preuss
email       : sbesch@buffalo.edu martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/*This is the public part of the include file for g_invacc.c*/

#ifndef AIO_OFX_G_INVACC_P_H

#define AIO_OFX_G_INVACC_P_H
#include "g_invacc_l.h"                                         /*Include the private part of the file*/

/*The <INVACCTFROM> and <INVACCTTO> groups include <BROKERID> and <ACCTID> tags. To these we
add an additional field for the account type and the Object required currentElement field.*/

typedef struct AIO_OFX_GROUP_INVACC AIO_OFX_GROUP_INVACC;
struct AIO_OFX_GROUP_INVACC {
  char *brokerId;
  char *accId;
  char *accType;
  char *currentElement;
};

static void GWENHYWFAR_CB AIO_OfxGroup_INVACC_FreeData(void *bp, void *p);
static int AIO_OfxGroup_INVACC_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_INVACC_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


