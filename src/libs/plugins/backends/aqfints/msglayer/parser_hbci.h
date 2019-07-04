/***************************************************************************
 begin       : Fri Jun 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_PARSER_HBCI_H
#define AQFINTS_PARSER_HBCI_H


#include "msglayer/element.h"
#include "msglayer/segment.h"



int AQFINTS_Parser_Hbci_Buffer_Read(AQFINTS_SEGMENT_LIST *targetSegmentList,
                                    const uint8_t *ptrBuf,
                                    uint32_t lenBuf);

#endif

