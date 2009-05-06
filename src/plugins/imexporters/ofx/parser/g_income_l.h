/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Fri Apr 17 2009
 copyright   : (C) 2009 by Stephen R. Besch (C) 2008 by Martin Preuss
 email       : sbesch@buffalo.edu martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AIO_OFX_G_INCOME_L_H
#define AIO_OFX_G_INCOME_L_H

#include "ofxgroup_l.h"

typedef enum {
  iincometotal=0,
  iincometype,
  iincomesubacctsec,
  iincomesubacctfund,
  iincomelastdatum
} AIO_OFX_GROUP_INCOME_DATUMS;

typedef enum {
  iincomesecid=0,
  iincomeinvtran,
  iincomelastgroup
} AIO_OFX_GROUP_INCOME_GROUPS;

AIO_OFX_GROUP *AIO_OfxGroup_INCOME_new(const char *groupName,
					     AIO_OFX_GROUP *parent,
					     GWEN_XML_CONTEXT *ctx);

AB_TRANSACTION *AIO_OfxGroup_INCOME_TakeTransaction(const AIO_OFX_GROUP *g);

#endif //AIO_OFX_GROUP_INVBUY_DATUMS
