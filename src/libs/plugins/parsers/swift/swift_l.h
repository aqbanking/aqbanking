/***************************************************************************
 begin       : Fri Apr 02 2004
 copyright   : (C) 2022 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCIBANK_SWIFT_l_H
#define AQHBCIBANK_SWIFT_l_H

#include "swift_tag.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/gwendate.h>


#define AHB_SWIFT_MAXLINELEN 2048

#define AHB_SWIFT_CENTURY_CUTOFF_YEAR 79



int AHB_SWIFT_Condense(char *buffer, int keepDoubleBlanks);

int AHB_SWIFT_SetCharValue(GWEN_DB_NODE *db, uint32_t flags, const char *name, const char *s);

GWEN_DATE *AHB_SWIFT_ReadDateYYMMDD(const char **pCurrentChar, unsigned int *pBytesLeft);
GWEN_DATE *AHB_SWIFT_ReadDateMMDDWithReference(const char **pCurrentChar, unsigned int *pBytesLeft, const GWEN_DATE *refDate);



#endif /* AQHBCIBANK_SWIFT_L_H */



