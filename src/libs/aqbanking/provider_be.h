/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_PROVIDER_BE_H
#define AQBANKING_PROVIDER_BE_H

#include <aqbanking/provider.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef AB_PROVIDER* (*AB_PROVIDER_FACTORY_FN)(AB_BANKING *ab,
                                               GWEN_DB_NODE *db);

/** @name Prototypes For Virtual Functions
 *
 */
/*@{*/
typedef int (*AB_PROVIDER_INIT_FN)(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
typedef int (*AB_PROVIDER_FINI_FN)(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);

typedef int (*AB_PROVIDER_UPDATEJOB_FN)(AB_PROVIDER *pro, AB_JOB *j);
typedef int (*AB_PROVIDER_ADDJOB_FN)(AB_PROVIDER *pro, AB_JOB *j);
typedef int (*AB_PROVIDER_EXECUTE_FN)(AB_PROVIDER *pro);
typedef AB_ACCOUNT_LIST2* (*AB_PROVIDER_GETACCOUNTLIST_FN)(AB_PROVIDER *pro);
typedef int (*AB_PROVIDER_UPDATEACCOUNT_FN)(AB_PROVIDER *pro, AB_ACCOUNT *a);
typedef int (*AB_PROVIDER_ADDACCOUNT_FN)(AB_PROVIDER *pro, AB_ACCOUNT *a);
typedef int (*AB_PROVIDER_IMPORTTRANSACTIONS_FN)(AB_PROVIDER *pro,
                                                 AB_TRANSACTION_LIST2 *tl,
                                                 GWEN_BUFFEREDIO *bio);
/*@}*/




AQBANKING_API
AB_PROVIDER *AB_Provider_new(AB_BANKING *ab,
                             const char *name);

int AB_Provider_IsInit(const AB_PROVIDER *pro);


/** @name Setters For Virtual Functions
 *
 */
/*@{*/
AQBANKING_API
void AB_Provider_SetInitFn(AB_PROVIDER *pro, AB_PROVIDER_INIT_FN f);
AQBANKING_API
void AB_Provider_SetFiniFn(AB_PROVIDER *pro, AB_PROVIDER_FINI_FN f);

AQBANKING_API
void AB_Provider_SetUpdateJobFn(AB_PROVIDER *pro, AB_PROVIDER_UPDATEJOB_FN f);
AQBANKING_API
void AB_Provider_SetAddJobFn(AB_PROVIDER *pro, AB_PROVIDER_ADDJOB_FN f);
AQBANKING_API
void AB_Provider_SetExecuteFn(AB_PROVIDER *pro, AB_PROVIDER_EXECUTE_FN f);
AQBANKING_API
void AB_Provider_SetGetAccountListFn(AB_PROVIDER *pro,
                                     AB_PROVIDER_GETACCOUNTLIST_FN f);
AQBANKING_API
void AB_Provider_SetUpdateAccountFn(AB_PROVIDER *pro,
                                    AB_PROVIDER_UPDATEACCOUNT_FN f);
AQBANKING_API
void AB_Provider_SetAddAccountFn(AB_PROVIDER *pro,
                                 AB_PROVIDER_ADDACCOUNT_FN f);
AQBANKING_API
void AB_Provider_SetImportTransactionsFn(AB_PROVIDER *pro,
                                         AB_PROVIDER_IMPORTTRANSACTIONS_FN f);
/*@}*/


#ifdef __cplusplus
}
#endif




#endif /* AQBANKING_PROVIDER_BE_H */









