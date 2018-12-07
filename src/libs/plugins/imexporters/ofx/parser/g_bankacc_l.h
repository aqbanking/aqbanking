/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_BANKACC_L_H
#define AIO_OFX_G_BANKACC_L_H


#include "ofxgroup_l.h"



AIO_OFX_GROUP *AIO_OfxGroup_BANKACC_new(const char *groupName,
					AIO_OFX_GROUP *parent,
					GWEN_XML_CONTEXT *ctx);

const char *AIO_OfxGroup_BANKACC_GetBankId(const AIO_OFX_GROUP *g);
void AIO_OfxGroup_BANKACC_SetBankId(AIO_OFX_GROUP *g, const char *s);

const char *AIO_OfxGroup_BANKACC_GetAccId(const AIO_OFX_GROUP *g);
void AIO_OfxGroup_BANKACC_SetAccId(AIO_OFX_GROUP *g, const char *s);

const char *AIO_OfxGroup_BANKACC_GetAccType(const AIO_OFX_GROUP *g);
void AIO_OfxGroup_BANKACC_SetAccType(AIO_OFX_GROUP *g, const char *s);


#endif
