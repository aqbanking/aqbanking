/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Fri Apr 17 2009
 copyright   : (C) 2009 by Stephen R. Besch (C) 2008 by Martin Preuss
 email       : sbesch@buffalo.edu martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AIO_OFX_G_BUYSTOCK_L_H
#define AIO_OFX_G_BUYSTOCK_L_H

#include "ofxgroup_l.h"

AIO_OFX_GROUP *AIO_OfxGroup_BUYSTOCK_new(const char *groupName,
					     AIO_OFX_GROUP *parent,
					     GWEN_XML_CONTEXT *ctx);

AB_TRANSACTION *AIO_OfxGroup_BUYSTOCK_TakeTransaction(const AIO_OFX_GROUP *g);
char * AIO_OfxGroup_BUYSTOCK_GetDatum(const AIO_OFX_GROUP *sg, int index);
#endif
