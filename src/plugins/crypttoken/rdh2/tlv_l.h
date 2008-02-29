/***************************************************************************
    begin       : Mon Feb 25 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_PLUGIN_CT_RDH2_TLV_L_H
#define AQBANKING_PLUGIN_CT_RDH2_TLV_L_H

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/misc.h>


typedef struct TLV TLV;

GWEN_LIST_FUNCTION_DEFS(TLV, Tlv)


TLV *Tlv_new();
void Tlv_DirectlyToBuffer(unsigned int tagType,
			  const char *p,
			  int size,
			  GWEN_BUFFER *buf);

void Tlv_free(TLV *tlv);

TLV *Tlv_fromBuffer(GWEN_BUFFER *mbuf);

unsigned int Tlv_GetTagType(const TLV *tlv);
unsigned int Tlv_GetTagLength(const TLV *tlv);
const void *Tlv_GetTagData(const TLV *tlv);

unsigned int Tlv_GetTagSize(const TLV *tlv);




#endif /* GWENHYWFAR_OHBCI_TAG16_H */

