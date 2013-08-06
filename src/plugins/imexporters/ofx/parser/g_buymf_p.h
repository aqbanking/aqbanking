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


#ifndef AIO_OFX_G_BUYMF_P_H
#define AIO_OFX_G_BUYMF_P_H


#include "g_buymf_l.h"


typedef struct AIO_OFX_GROUP_BUYMF AIO_OFX_GROUP_BUYMF;
struct AIO_OFX_GROUP_BUYMF {
  char *currentElement;

  AB_TRANSACTION *transaction;
};

static void GWENHYWFAR_CB AIO_OfxGroup_BUYMF_FreeData(void *bp, void *p);


static int AIO_OfxGroup_BUYMF_StartTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_BUYMF_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);
static int AIO_OfxGroup_BUYMF_AddData(AIO_OFX_GROUP *g, const char *data);

#endif


