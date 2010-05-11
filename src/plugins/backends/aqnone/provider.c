/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_p.h"



AB_PROVIDER *AN_Provider_new(AB_BANKING *ab){
  AB_PROVIDER *pro;

  pro=AB_Provider_new(ab, "none");
  AB_Provider_SetInitFn(pro, AN_Provider_Init);
  AB_Provider_SetFiniFn(pro, AN_Provider_Fini);
  AB_Provider_SetUpdateJobFn(pro, AN_Provider_UpdateJob);
  AB_Provider_SetAddJobFn(pro, AN_Provider_AddJob);
  AB_Provider_SetExecuteFn(pro, AN_Provider_Execute);
  AB_Provider_SetResetQueueFn(pro, AN_Provider_ResetQueue);
  AB_Provider_SetExtendUserFn(pro, AN_Provider_ExtendUser);
  AB_Provider_SetExtendAccountFn(pro, AN_Provider_ExtendAccount);

  return pro;
}



int AN_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData) {
  return 0;
}



int AN_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData){
  return 0;
}



int AN_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j){
  return GWEN_ERROR_NOT_SUPPORTED;
}



int AN_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j){
  return GWEN_ERROR_NOT_SUPPORTED;
}



int AN_Provider_ResetQueue(AB_PROVIDER *pro){
  return 0;
}



int AN_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx){
  return 0;
}


int AN_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
                           AB_PROVIDER_EXTEND_MODE em,
			   GWEN_DB_NODE *dbBackend) {
  return 0;
}



int AN_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
                              AB_PROVIDER_EXTEND_MODE em,
			      GWEN_DB_NODE *dbBackend){
  return 0;
}



