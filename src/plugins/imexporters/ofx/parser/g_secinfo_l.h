/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_SECINFO_L_H
#define AIO_OFX_G_SECINFO_L_H


#include "ofxgroup_l.h"



AIO_OFX_GROUP *AIO_OfxGroup_SECINFO_new(const char *groupName,
					AIO_OFX_GROUP *parent,
					GWEN_XML_CONTEXT *ctx);

const char *AIO_OfxGroup_SECINFO_GetTicker(const AIO_OFX_GROUP *g);
void AIO_OfxGroup_SECINFO_SetTicker(AIO_OFX_GROUP *g, const char *s);

const char *AIO_OfxGroup_SECINFO_GetSecurityName(const AIO_OFX_GROUP *g);
void AIO_OfxGroup_SECINFO_SetSecurityName(AIO_OFX_GROUP *g, const char *s);

const char *AIO_OfxGroup_SECINFO_GetUniqueId(const AIO_OFX_GROUP *g);
void AIO_OfxGroup_SECINFO_SetUniqueId(AIO_OFX_GROUP *g, const char *s);

const char *AIO_OfxGroup_SECINFO_GetNameSpace(const AIO_OFX_GROUP *g);
void AIO_OfxGroup_SECINFO_SetNameSpace(AIO_OFX_GROUP *g, const char *s);







#endif
