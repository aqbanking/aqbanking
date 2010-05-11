/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2010 by Martin Preuss
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

  char *apiPassword;
  char *apiSignature;

  int httpVMajor;
  int httpVMinor;
};

static void APY_User_freeData(void *bp, void *p);

static void APY_User_ReadDb(AB_USER *u, GWEN_DB_NODE *db);
static void APY_User_toDb(AB_USER *u, GWEN_DB_NODE *db);



#endif


