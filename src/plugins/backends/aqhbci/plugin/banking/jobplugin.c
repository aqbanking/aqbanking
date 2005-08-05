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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "jobplugin_p.h"
#include "aqhbci_l.h"
#include "wcb_l.h"
#include "hbci_l.h"
#include <aqbanking/job_be.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/debug.h>


GWEN_LIST_FUNCTIONS(AH_JOBPLUGIN, AH_JobPlugin);
GWEN_INHERIT_FUNCTIONS(AH_JOBPLUGIN);


AH_JOBPLUGIN *AH_JobPlugin_new(AH_PROVIDER *pro, const char *name){
  AH_JOBPLUGIN *jp;

  assert(pro);
  assert(name);
  GWEN_NEW_OBJECT(AH_JOBPLUGIN, jp);
  GWEN_INHERIT_INIT(AH_JOBPLUGIN, jp);
  GWEN_LIST_INIT(AH_JOBPLUGIN, jp);
  jp->provider=pro;
  jp->name=strdup(name);

  return jp;
}



void AH_JobPlugin_free(AH_JOBPLUGIN *jp){
  if (jp) {
    GWEN_INHERIT_FINI(AH_JOBPLUGIN, jp);
    if (jp->libLoader) {
      GWEN_LibLoader_CloseLibrary(jp->libLoader);
      GWEN_LibLoader_free(jp->libLoader);
    }
    free(jp->description);
    GWEN_LIST_FINI(AH_JOBPLUGIN, jp);
    GWEN_FREE_OBJECT(jp);
  }
}



GWEN_LIBLOADER *AH_JobPlugin_GetLibLoader(const AH_JOBPLUGIN *jp){
  assert(jp);
  return jp->libLoader;
}



void AH_JobPlugin_SetLibLoader(AH_JOBPLUGIN *jp, GWEN_LIBLOADER *ll){
  assert(jp);
  jp->libLoader=ll;
}



AH_PROVIDER *AH_JobPlugin_GetProvider(const AH_JOBPLUGIN *jp){
  assert(jp);
  return jp->provider;
}



AH_JOB *AH_JobPlugin_Factory(AH_JOBPLUGIN *jp,
                             AB_JOB_TYPE jt,
                             AH_CUSTOMER *cu,
                             AH_ACCOUNT *a){
  assert(jp);
  assert(jp->factoryFn);
  return jp->factoryFn(jp, jt, cu, a);
}



int AH_JobPlugin_CheckType(AH_JOBPLUGIN *jp,
                           AB_JOB_TYPE jt){
  assert(jp);
  assert(jp->checkFn);
  return jp->checkFn(jp, jt);
}



const char *AH_JobPlugin_GetDescription(const AH_JOBPLUGIN *jp){
  assert(jp);
  return jp->description;
}



void AH_JobPlugin_SetDescription(AH_JOBPLUGIN *jp, const char *s){
  assert(jp);
  free(jp->description);
  if (s) jp->description=strdup(s);
  else jp->description=0;
}



void AH_JobPlugin_SetFactoryFn(AH_JOBPLUGIN *jp, AH_JOBPLUGIN_FACTORYFN f){
  assert(jp);
  jp->factoryFn=f;
}



void AH_JobPlugin_SetCheckFn(AH_JOBPLUGIN *jp, AH_JOBPLUGIN_CHECKFN f){
  assert(jp);
  jp->checkFn=f;
}



const char *AH_JobPlugin_GetName(const AH_JOBPLUGIN *jp){
  assert(jp);
  return jp->name;
}






