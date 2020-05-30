/***************************************************************************
 begin       : Sun Jul 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_UPD_WRITE_H
#define AQFINTS_UPD_WRITE_H


#include "servicelayer/upd/accountdata.h"
#include "servicelayer/upd/userdata.h"

#include "parser/parser.h"
#include "parser/segment.h"

#include <gwenhywfar/db.h>



int AQFINTS_Upd_Write(const AQFINTS_USERDATA *userData,
		      AQFINTS_PARSER *parser,
		      int refSegNum,
		      AQFINTS_SEGMENT_LIST *segmentList);



#endif

