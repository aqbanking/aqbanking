/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Fri Apr 17 2009
 copyright   : (C) 2009 by Stephen R. Besch (C) 2008 by Martin Preuss
 email       : sbesch@buffalo.edu martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AIO_OFX_G_BUYSTOCK_P_H
#define AIO_OFX_G_BUYSTOCK_P_H

#include "g_buystock_l.h"

typedef enum {
  ibuytype=0,
  iselltype,
  ibuystocklastdatum
}AIO_OFX_GROUP_BUYSTOCK_DATUMS;

typedef enum {
  iinvbuy=0,
  iinvsell,
  ibuystocklastgroup
}AIO_OFX_GROUP_BUYSTOCK_GROUPS;

typedef struct AIO_OFX_GROUP_BUYSTOCK AIO_OFX_GROUP_BUYSTOCK;
struct AIO_OFX_GROUP_BUYSTOCK {
  char *currentElement;
  AB_TRANSACTION *transaction;
};

typedef void (*AB_Transaction_Set_Value_Func)(AB_TRANSACTION *st, const AB_VALUE *d);

void  AIO_OfxGroup_BUYSTOCK_SetABValue(const AIO_OFX_GROUP *sg, AB_Transaction_Set_Value_Func F, AB_TRANSACTION *t, int index);
int AIO_OfxGroup_BUYSTOCK_SortTag(const char * s, char ** sTags, int max);

static void GWENHYWFAR_CB AIO_OfxGroup_BUYSTOCK_FreeData(void *bp, void *p);
static int AIO_OfxGroup_BUYSTOCK_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_BUYSTOCK_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);
static int AIO_OfxGroup_BUYSTOCK_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


