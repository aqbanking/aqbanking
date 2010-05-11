/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AN_PROVIDER_P_H
#define AN_PROVIDER_P_H

#include "provider_l.h"

int AN_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AN_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AN_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
int AN_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
int AN_Provider_ResetQueue(AB_PROVIDER *pro);
int AN_Provider_Execute(AB_PROVIDER *pro,
			AB_IMEXPORTER_CONTEXT *ctx);
int AN_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
			   AB_PROVIDER_EXTEND_MODE em,
			   GWEN_DB_NODE *dbBackend);
int AN_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
			      AB_PROVIDER_EXTEND_MODE em,
			      GWEN_DB_NODE *dbBackend);

#endif

