/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Fri Apr 02 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCIBANK_SWIFT940_P_H
#define AQHBCIBANK_SWIFT940_P_H


#include "swift_l.h"
#include <gwenhywfar/buffer.h>


int AHB_SWIFT940_Parse_25(const AHB_SWIFT_TAG *tg,
                          uint32_t flags,
                          GWEN_DB_NODE *data,
                          GWEN_DB_NODE *cfg);


int AHB_SWIFT940_Parse_86(const AHB_SWIFT_TAG *tg,
                          uint32_t flags,
                          GWEN_DB_NODE *data,
                          GWEN_DB_NODE *cfg);

int AHB_SWIFT940_Parse_61(const AHB_SWIFT_TAG *tg,
                          uint32_t flags,
                          GWEN_DB_NODE *data,
                          GWEN_DB_NODE *cfg);

int AHB_SWIFT940_Parse_6_0_2(const AHB_SWIFT_TAG *tg,
                             uint32_t flags,
                             GWEN_DB_NODE *data,
                             GWEN_DB_NODE *cfg);



#endif /* AQHBCIBANK_SWIFT940_P_H */

