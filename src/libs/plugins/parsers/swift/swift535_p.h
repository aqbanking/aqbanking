/***************************************************************************
 begin       : Fri Apr 02 2004
 copyright   : (C) 2004-2015 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCIBANK_SWIFT535_P_H
#define AQHBCIBANK_SWIFT535_P_H


#include "swift_l.h"
#include <gwenhywfar/buffer.h>


enum {
  AHB_SWIFT535_LEVEL_TOP=0,
  AHB_SWIFT535_LEVEL_GENL,
  AHB_SWIFT535_LEVEL_SUBSAFE,
  AHB_SWIFT535_LEVEL_FIN,
  AHB_SWIFT535_LEVEL_SUBBAL,
  AHB_SWIFT535_LEVEL_ADDINFO
};


int AHB_SWIFT535_Parse_97A(const AHB_SWIFT_TAG *tg,
                           uint32_t flags,
                           GWEN_DB_NODE *data,
                           GWEN_DB_NODE *cfg);

int AHB_SWIFT535_Parse_35B(const AHB_SWIFT_TAG *tg,
                           uint32_t flags,
                           GWEN_DB_NODE *data,
                           GWEN_DB_NODE *cfg);

int AHB_SWIFT535_Parse_90B(const AHB_SWIFT_TAG *tg,
                           uint32_t flags,
                           GWEN_DB_NODE *data,
                           GWEN_DB_NODE *cfg);

int AHB_SWIFT535_Parse_98A(const AHB_SWIFT_TAG *tg,
                           uint32_t flags,
                           GWEN_DB_NODE *data,
                           GWEN_DB_NODE *cfg);

int AHB_SWIFT535_Parse_93B(const AHB_SWIFT_TAG *tg,
                           uint32_t flags,
                           GWEN_DB_NODE *data,
                           GWEN_DB_NODE *cfg);


#endif /* AQHBCIBANK_SWIFT535_P_H */

