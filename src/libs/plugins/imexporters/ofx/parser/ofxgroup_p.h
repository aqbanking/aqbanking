/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_OFXGROUP_P_H
#define AIO_OFX_OFXGROUP_P_H


#include "ofxgroup_l.h"


struct AIO_OFX_GROUP {
  GWEN_INHERIT_ELEMENT(AIO_OFX_GROUP)

  AIO_OFX_GROUP *parent;
  GWEN_XML_CONTEXT *xmlContext;

  char *groupName;

  AIO_OFX_GROUP_STARTTAG_FN startTagFn;
  AIO_OFX_GROUP_ENDTAG_FN endTagFn;
  AIO_OFX_GROUP_ADDDATA_FN addDataFn;
  AIO_OFX_GROUP_ENDSUBGROUP_FN endSubGroupFn;
};



#endif

