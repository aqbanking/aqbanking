/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Fri Apr 17 2009
 copyright   : (C) 2009 by Stephen R. Besch (C) 2008 by Martin Preuss
 email       : sbesch@buffalo.edu martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_INVBUY_L_H
#define AIO_OFX_G_INVBUY_L_H

#include "ofxgroup_l.h"

typedef enum {
  icommission=0,
  itotal,
  iunits,
  iunitprice,
  isubacctsec,
  isubacctfund,
  iinvbuylastdatum=6,
  iuniqueid=6,
  iuniqueidtype,
  ifitid,
  idttrade,
  imemo,
  iinvbuylastget
} AIO_OFX_GROUP_INVBUY_DATUMS;

typedef enum {
  isecid=0,
  iinvtran,
  iinvbuylastgroup
} AIO_OFX_GROUP_INVBUY_GROUPS;


AIO_OFX_GROUP *AIO_OfxGroup_INVBUY_new(const char *groupName, AIO_OFX_GROUP *parent, GWEN_XML_CONTEXT *ctx);

const char *AIO_OfxGroup_INVBUY_GetDatum(const AIO_OFX_GROUP *g, int index);
const char *AIO_OfxGroup_INVBUY_DatumName(int index);

#endif
