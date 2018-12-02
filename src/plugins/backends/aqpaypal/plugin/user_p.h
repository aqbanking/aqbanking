/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_USER_P_H
#define AQPAYPAL_USER_P_H


#include "user_l.h"

#include <gwenhywfar/db.h>



typedef struct APY_USER APY_USER;
struct APY_USER {
  char *serverUrl;

  char *apiUserId;
  char *apiPassword;
  char *apiSignature;

  int httpVMajor;
  int httpVMinor;

  AB_USER_READFROMDB_FN readFromDbFn;
  AB_USER_WRITETODB_FN writeToDbFn;
};

static void GWENHYWFAR_CB APY_User_freeData(void *bp, void *p);


int APY_User_ReadFromDb(AB_USER *u, GWEN_DB_NODE *db);
int APY_User_WriteToDb(const AB_USER *u, GWEN_DB_NODE *db);



#endif


