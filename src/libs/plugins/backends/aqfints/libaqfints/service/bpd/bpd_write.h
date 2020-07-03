/***************************************************************************
 begin       : Sat Augl 03 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQFINTS_BPD_WRITE_H
#define AQFINTS_BPD_WRITE_H


#include "service/bpd/bpd.h"
#include "parser/parser.h"
#include "parser/segment.h"

#include <gwenhywfar/db.h>




int AQFINTS_Bpd_Write(const AQFINTS_BPD *bpd, AQFINTS_PARSER *parser, int hbciVersion, int refSegNum,
                      AQFINTS_SEGMENT_LIST *segmentList);



#endif

