/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_BAL_L_H
#define AIO_OFX_G_BAL_L_H


#include "ofxgroup_l.h"

#include <aqbanking/value.h>

#include <gwenhywfar/gwentime.h>



AIO_OFX_GROUP *AIO_OfxGroup_BAL_new(const char *groupName,
				    AIO_OFX_GROUP *parent,
				    GWEN_XML_CONTEXT *ctx);

const AB_VALUE *AIO_OfxGroup_BAL_GetValue(const AIO_OFX_GROUP *g);
void AIO_OfxGroup_BAL_SetValue(AIO_OFX_GROUP *g, const AB_VALUE *v);

const GWEN_TIME *AIO_OfxGroup_BAL_GetDate(const AIO_OFX_GROUP *g);
void AIO_OfxGroup_BAL_SetDate(AIO_OFX_GROUP *g, const GWEN_TIME *ti);



#endif
