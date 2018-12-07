/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_SECINFO_P_H
#define AIO_OFX_G_SECINFO_P_H


#include "g_secinfo_l.h"


typedef struct AIO_OFX_GROUP_SECINFO AIO_OFX_GROUP_SECINFO;
struct AIO_OFX_GROUP_SECINFO {
  char *secname;
  char *ticker;
  char *uniqueId;
  char *nameSpace;

  char *currentElement;
};

static void GWENHYWFAR_CB AIO_OfxGroup_SECINFO_FreeData(void *bp, void *p);


static int AIO_OfxGroup_SECINFO_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_SECINFO_AddData(AIO_OFX_GROUP *g, const char *data);
static int AIO_OfxGroup_SECINFO_EndSubGroup(AIO_OFX_GROUP *g,
					    AIO_OFX_GROUP *sg);

#endif


