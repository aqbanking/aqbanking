/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_MSG_MSG_P_H
#define AQEBICS_MSG_MSG_P_H


#include "msg.h"

#include <gwenhywfar/mdigest.h>



struct EB_MSG {
  GWEN_INHERIT_ELEMENT(EB_MSG)
  xmlDocPtr doc;
  xmlXPathContextPtr xpathCtx;
  char *hVersion;
  uint32_t usage;
};

static void EB_Msg__initWithDoc(EB_MSG *m);

static xmlDocPtr EB_Msg__generateRequest(int willSign, const char *hVersion);
static xmlDocPtr EB_Msg__generateResponse(int willSign, const char *rName, const char *hVersion);

static int EB_Msg__prepareSignature(xmlDocPtr doc);

/*static xmlNodeSetPtr EB_Xml_GetNodes(EB_MSG *m, const char *xpathExpr);*/


#endif

