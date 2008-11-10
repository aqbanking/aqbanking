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


#ifndef AQHBCIBANK_SWIFT_l_H
#define AQHBCIBANK_SWIFT_l_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/dbio.h>


#define AHB_SWIFT_MAXLINELEN 512


typedef struct AHB_SWIFT_TAG AHB_SWIFT_TAG;
typedef struct AHB_SWIFT_FIELD AHB_SWIFT_FIELD;


GWEN_LIST_FUNCTION_DEFS(AHB_SWIFT_TAG, AHB_SWIFT_Tag);


const char *AHB_SWIFT_Tag_GetId(const AHB_SWIFT_TAG *tg);
const char *AHB_SWIFT_Tag_GetData(const AHB_SWIFT_TAG *tg);


int AHB_SWIFT_Condense(char *buffer, int keepDoubleBlanks);


#endif /* AQHBCIBANK_SWIFT_L_H */



