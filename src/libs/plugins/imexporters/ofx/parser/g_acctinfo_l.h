/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_ACCTINFO_L_H
#define AIO_OFX_G_ACCTINFO_L_H


#include "ofxgroup_l.h"



AIO_OFX_GROUP *AIO_OfxGroup_ACCTINFO_new(const char *groupName,
					 AIO_OFX_GROUP *parent,
					 GWEN_XML_CONTEXT *ctx);

const char *AIO_OfxGroup_ACCTINFO_GetBankId(const AIO_OFX_GROUP *g);
const char *AIO_OfxGroup_ACCTINFO_GetAccId(const AIO_OFX_GROUP *g);
const char *AIO_OfxGroup_ACCTINFO_GetAccType(const AIO_OFX_GROUP *g);
const char *AIO_OfxGroup_ACCTINFO_GetAccDescr(const AIO_OFX_GROUP *g);

#endif
