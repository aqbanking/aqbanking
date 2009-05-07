/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Fri Apr 17 2009
 copyright   : (C) 2009 by Stephen R. Besch (C) 2008 by Martin Preuss
 email       : sbesch@buffalo.edu martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AIO_OFX_G_INCOME_P_H
#define AIO_OFX_G_INCOME_P_H

#include "g_income_l.h"

typedef struct AIO_OFX_GROUP_INCOME AIO_OFX_GROUP_INCOME;
struct AIO_OFX_GROUP_INCOME {
  char *currentElement;
  AB_TRANSACTION *transaction;
};

static void GWENHYWFAR_CB AIO_OfxGroup_INCOME_FreeData(void *bp, void *p);
static int AIO_OfxGroup_INCOME_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_INCOME_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);
static int AIO_OfxGroup_INCOME_AddData(AIO_OFX_GROUP *g, const char *data);
int AIO_OfxGroup_INCOME_SortTag(const char * s, const char ** sTags, int max);

#endif


