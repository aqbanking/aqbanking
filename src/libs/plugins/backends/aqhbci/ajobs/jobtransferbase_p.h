/***************************************************************************
    begin       : Tue Dec 31 2013
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBTRANSFERBASE_P_H
#define AH_JOBTRANSFERBASE_P_H


#include "jobtransferbase_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_TRANSFERBASE AH_JOB_TRANSFERBASE;
struct AH_JOB_TRANSFERBASE {
  AB_TRANSACTION_TYPE transactionType;
  AB_TRANSACTION_SUBTYPE transactionSubType;
  char *fiid;
  char *descriptor;
  char *profileName;
  char *localInstrumentationCode;
};


#endif /* AH_JOBTRANSFERBASE_P_H */



