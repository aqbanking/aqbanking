/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_P_H
#define AH_PROVIDER_P_H

#define AH_PROVIDER_DATADIR ".libaqhbci"

#include "provider_l.h"
#include "job_l.h"
#include "outbox_l.h"


#include <aqbanking/backendsupport/userqueue.h>


typedef struct AH_PROVIDER AH_PROVIDER;
struct AH_PROVIDER {
  AH_HBCI *hbci;
  GWEN_DB_NODE *dbTempConfig;
};
static void GWENHYWFAR_CB AH_Provider_FreeData(void *bp, void *p);


/** @name Overwritten Virtual Functions
 *
 */
/*@{*/
static int AH_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int AH_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);

static AB_ACCOUNT *AH_Provider_CreateAccountObject(AB_PROVIDER *pro);
static AB_USER *AH_Provider_CreateUserObject(AB_PROVIDER *pro);


/*@}*/

#endif /* AH_PROVIDER_P_H */




