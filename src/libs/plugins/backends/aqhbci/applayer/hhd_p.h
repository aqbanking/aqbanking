/***************************************************************************
    begin       : Sat Dec 10 2011
    copyright   : (C) 2011 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_HHD_P_H
#define AQHBCI_HHD_P_H


#include "hhd_l.h"


static int AH_HHD14_ReadBytesDec(const char *p, int len);
static int AH_HHD14_ReadBytesHex(const char *p, int len);
static unsigned int AH_HHD14_Quersumme(unsigned int i);
static int AH_HHD14_CalcLuhnSum(const char *code, int len);
static int AH_HHD14_CalcXorSum(const char *code, int len);

static int AH_HHD14_ExtractDataForLuhnSum(const char *code, GWEN_BUFFER *xbuf);
static void AH_HHD14_CompressCode(const uint8_t *code, GWEN_BUFFER *cbuf);
static void AH_HHD14_ExtractCode(GWEN_BUFFER *cbuf);

static int AH_HHD14__Translate(const char *code, GWEN_BUFFER *cbuf);



#endif
