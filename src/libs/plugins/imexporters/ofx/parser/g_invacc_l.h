/***************************************************************************
 $RCSfile$
 -------------------
begin       : Thur Apr 23 2009
copyright   : (C) 2009 by Stephen R. Besch (C) 2008 by Martin Preuss
email       : sbesch@buffalo.edu martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AIO_OFX_G_INVACC_L_H
#define AIO_OFX_G_INVACC_L_H

#include "ofxgroup_l.h"

AIO_OFX_GROUP *AIO_OfxGroup_INVACC_new(const char *groupName,
					AIO_OFX_GROUP *parent,
					GWEN_XML_CONTEXT *ctx);

const char *AIO_OfxGroup_INVACC_GetBrokerId(const AIO_OFX_GROUP *g);
void AIO_OfxGroup_INVACC_SetBrokerId(AIO_OFX_GROUP *g, const char *s);

const char *AIO_OfxGroup_INVACC_GetAccId(const AIO_OFX_GROUP *g);
void AIO_OfxGroup_INVACC_SetAccId(AIO_OFX_GROUP *g, const char *s);

const char *AIO_OfxGroup_INVACC_GetAccType(const AIO_OFX_GROUP *g);
void AIO_OfxGroup_INVACC_SetAccType(AIO_OFX_GROUP *g, const char *s);

#endif

