/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_INVPOSLIST_L_H
#define AIO_OFX_G_INVPOSLIST_L_H


#include "ofxgroup_l.h"



AIO_OFX_GROUP *AIO_OfxGroup_INVPOSLIST_new(const char *groupName,
					   AIO_OFX_GROUP *parent,
					   GWEN_XML_CONTEXT *ctx);


#endif
