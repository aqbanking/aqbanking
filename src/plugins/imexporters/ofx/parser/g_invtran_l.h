/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Fri Apr 17 2009
 copyright   : (C) 2009 by Stephen R. Besch (C) 2008 by Martin Preuss
 email       : sbesch@buffalo.edu martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AIO_OFX_G_INVTRAN_L_H
#define AIO_OFX_G_INVTRAN_L_H

#include "ofxgroup_l.h"


enum AIO_OFX_GROUP_INVTRAN_DATUMS {
  itranfitid=0,
  itrandttrade,
  itranmemo,
  iinvtranlastdatum
};

AIO_OFX_GROUP *AIO_OfxGroup_INVTRAN_new(const char *groupName, AIO_OFX_GROUP *parent, GWEN_XML_CONTEXT *ctx);

char *AIO_OfxGroup_INVTRAN_GetDatum(const AIO_OFX_GROUP *g, int index);

#endif
