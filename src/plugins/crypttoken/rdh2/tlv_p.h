/***************************************************************************
    begin       : Mon Feb 25 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_PLUGIN_CT_RDH2_TLV_P_H
#define AQBANKING_PLUGIN_CT_RDH2_TLV_P_H


#include "tlv_l.h"


struct TLV {
  GWEN_LIST_ELEMENT(TLV)
  unsigned int tagSize;
  unsigned int tagType;
  unsigned int tagLength;
  void *tagData;
};




#endif
