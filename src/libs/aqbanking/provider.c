/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_p.h"
#include "provider_be.h"
#include "banking_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


GWEN_INHERIT_FUNCTIONS(AB_PROVIDER)
GWEN_LIST_FUNCTIONS(AB_PROVIDER, AB_Provider)

GWEN_INHERIT(GWEN_PLUGIN, AB_PLUGIN_PROVIDER)




AB_PROVIDER *AB_Provider_new(AB_BANKING *ab, const char *name){
  AB_PROVIDER *pro;
  GWEN_BUFFER *nbuf;

  assert(ab);
  assert(name);

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (GWEN_Text_EscapeToBufferTolerant(name, nbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Bad backend name, aborting.");
    GWEN_Buffer_free(nbuf);
    abort();
  }
  else {
    char *s;

    s=GWEN_Buffer_GetStart(nbuf);
    while(*s) {
      *s=tolower(*s);
      s++;
    }
  }

  GWEN_NEW_OBJECT(AB_PROVIDER, pro);
  pro->usage=1;
  GWEN_INHERIT_INIT(AB_PROVIDER, pro);
  GWEN_LIST_INIT(AB_PROVIDER, pro);

  pro->banking=ab;
  pro->name=strdup(name);
  pro->escName=strdup(GWEN_Buffer_GetStart(nbuf));
  GWEN_Buffer_free(nbuf);

  return pro;
}



void AB_Provider_free(AB_PROVIDER *pro){
  if (pro) {
    assert(pro->usage);
    if (--(pro->usage)==0) {
      DBG_VERBOUS(AQBANKING_LOGDOMAIN, "Destroying AB_PROVIDER (%s)",
                  pro->name);
      GWEN_INHERIT_FINI(AB_PROVIDER, pro);
      GWEN_Plugin_free(pro->plugin);
      free(pro->name);
      free(pro->escName);
      GWEN_LIST_FINI(AB_PROVIDER, pro);
      GWEN_FREE_OBJECT(pro);
    }
  }
}



void AB_Provider_SetPlugin(AB_PROVIDER *pro, GWEN_PLUGIN *pl) {
  assert(pro);
  pro->plugin=pl;
}



int AB_Provider_Init(AB_PROVIDER *pro, uint32_t guiid){
  assert(pro);
  if (pro->isInit) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider already is initialized");
    return GWEN_ERROR_INVALID;
  }
  if (pro->initFn) {
    int rv;
    GWEN_DB_NODE *dbData=NULL;

    rv=AB_Banking_LoadPluginConfig(pro->banking,
				   AB_CFG_GROUP_BACKENDS,
				   pro->name, &dbData,
				   guiid);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    else {
      assert(dbData);

      rv=pro->initFn(pro, dbData);
      if (!rv)
	pro->isInit=1;
      GWEN_DB_Group_free(dbData);
      return rv;
    }
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No init function set");
    return GWEN_ERROR_NOT_IMPLEMENTED;
  }
}



int AB_Provider_Fini(AB_PROVIDER *pro, uint32_t guiid){
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return GWEN_ERROR_INVALID;
  }
  if (pro->finiFn) {
    int rv;
    GWEN_DB_NODE *dbData;

    rv=AB_Banking_LockPluginConfig(pro->banking,
                                   AB_CFG_GROUP_BACKENDS,
				   pro->name,
				   guiid);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=AB_Banking_LoadPluginConfig(pro->banking,
                                   AB_CFG_GROUP_BACKENDS,
				   pro->name, &dbData,
				   guiid);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_UnlockPluginConfig(pro->banking,
                                    AB_CFG_GROUP_BACKENDS,
				    pro->name,
				    guiid);
      return rv;
    }

    pro->isInit=0;
    rv=pro->finiFn(pro, dbData);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_UnlockPluginConfig(pro->banking,
				    AB_CFG_GROUP_BACKENDS,
				    pro->name,
				    guiid);
      GWEN_DB_Group_free(dbData);
      return rv;
    }

    rv=AB_Banking_SavePluginConfig(pro->banking,
				   AB_CFG_GROUP_BACKENDS,
				   pro->name, dbData, guiid);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_UnlockPluginConfig(pro->banking,
				    AB_CFG_GROUP_BACKENDS,
				    pro->name,
				    guiid);
      GWEN_DB_Group_free(dbData);
      return rv;
    }

    rv=AB_Banking_UnlockPluginConfig(pro->banking,
				     AB_CFG_GROUP_BACKENDS,
				     pro->name, guiid);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_DB_Group_free(dbData);
      return rv;
    }
    GWEN_DB_Group_free(dbData);
    return 0;
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No fini function set");
    pro->isInit=0;
    return GWEN_ERROR_NOT_IMPLEMENTED;
  }
}



const char *AB_Provider_GetName(const AB_PROVIDER *pro){
  assert(pro);
  return pro->name;
}



const char *AB_Provider_GetEscapedName(const AB_PROVIDER *pro){
  assert(pro);
  return pro->escName;
}



AB_BANKING *AB_Provider_GetBanking(const AB_PROVIDER *pro){
  assert(pro);
  return pro->banking;
}




void AB_Provider_SetInitFn(AB_PROVIDER *pro, AB_PROVIDER_INIT_FN f){
  assert(pro);
  pro->initFn=f;
}



void AB_Provider_SetFiniFn(AB_PROVIDER *pro, AB_PROVIDER_FINI_FN f){
  assert(pro);
  pro->finiFn=f;
}




void AB_Provider_SetUpdateJobFn(AB_PROVIDER *pro, AB_PROVIDER_UPDATEJOB_FN f){
  assert(pro);
  pro->updateJobFn=f;
}



void AB_Provider_SetAddJobFn(AB_PROVIDER *pro, AB_PROVIDER_ADDJOB_FN f){
  assert(pro);
  pro->addJobFn=f;
}



void AB_Provider_SetExecuteFn(AB_PROVIDER *pro, AB_PROVIDER_EXECUTE_FN f){
  assert(pro);
  pro->executeFn=f;
}



void AB_Provider_SetResetQueueFn(AB_PROVIDER *pro, AB_PROVIDER_RESETQUEUE_FN f){
  assert(pro);
  pro->resetQueueFn=f;
}



void AB_Provider_SetExtendUserFn(AB_PROVIDER *pro,
                                 AB_PROVIDER_EXTEND_USER_FN f){
  assert(pro);
  pro->extendUserFn=f;
}



void AB_Provider_SetExtendAccountFn(AB_PROVIDER *pro,
                                    AB_PROVIDER_EXTEND_ACCOUNT_FN f){
  assert(pro);
  pro->extendAccountFn=f;
}



void AB_Provider_SetUpdateFn(AB_PROVIDER *pro, AB_PROVIDER_UPDATE_FN f) {
  assert(pro);
  pro->updateFn=f;
}



int AB_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j, uint32_t guiid){
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return GWEN_ERROR_INVALID;
  }
  if (pro->updateJobFn) {
    return pro->updateJobFn(pro, j, guiid);
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No updateJob function set");
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AB_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j, uint32_t guiid){
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return GWEN_ERROR_INVALID;
  }
  if (pro->addJobFn) {
    return pro->addJobFn(pro, j, guiid);
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No addJob function set");
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AB_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx,
			uint32_t guiid){
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return GWEN_ERROR_INVALID;
  }
  if (pro->executeFn) {
    return pro->executeFn(pro, ctx, guiid);
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No execute function set");
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AB_Provider_ResetQueue(AB_PROVIDER *pro){
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return GWEN_ERROR_INVALID;
  }
  if (pro->resetQueueFn) {
    return pro->resetQueueFn(pro);
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No resetQueue function set");
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AB_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
			   AB_PROVIDER_EXTEND_MODE em,
			   GWEN_DB_NODE *db) {
  assert(pro);
  assert(u);
  if (em!=AB_ProviderExtendMode_Save && pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return AB_ERROR_NOT_INIT;
  }

  if (pro->extendUserFn)
    return pro->extendUserFn(pro, u, em, db);
  DBG_INFO(AQBANKING_LOGDOMAIN, "No extendUser function set");
  return 0;
}



int AB_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
			      AB_PROVIDER_EXTEND_MODE em,
			      GWEN_DB_NODE *db) {
  assert(pro);
  assert(a);
  if (em!=AB_ProviderExtendMode_Save && pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider is not initialized");
    return AB_ERROR_NOT_INIT;
  }

  if (pro->extendAccountFn)
    return pro->extendAccountFn(pro, a, em, db);
  DBG_INFO(AQBANKING_LOGDOMAIN, "No extendAccount function set");
  return 0;
}



int AB_Provider_Update(AB_PROVIDER *pro,
                       uint32_t lastVersion,
                       uint32_t currentVersion) {
  assert(pro);
  if (pro->isInit==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Provider \"%s\" is not initialized",
              AB_Provider_GetName(pro));
    return AB_ERROR_NOT_INIT;
  }

  if (pro->updateFn)
    return pro->updateFn(pro, lastVersion, currentVersion);
  DBG_INFO(AQBANKING_LOGDOMAIN, "No update function set");
  return 0;
}



int AB_Provider_IsInit(const AB_PROVIDER *pro){
  assert(pro);
  return (pro->isInit!=0);
}



uint32_t AB_Provider_GetFlags(const AB_PROVIDER *pro){
  assert(pro);
  return pro->flags;
}



int AB_Provider_GetUserDataDir(const AB_PROVIDER *pro, GWEN_BUFFER *buf){
  assert(pro);
  assert(buf);
  assert(pro->banking);
  assert(pro->escName);

  return AB_Banking_GetProviderUserDataDir(pro->banking,
                                           pro->escName,
                                           buf);
}



GWEN_PLUGIN *AB_Plugin_Provider_new(GWEN_PLUGIN_MANAGER *pm,
				    const char *name,
				    const char *fileName) {
  GWEN_PLUGIN *pl;
  AB_PLUGIN_PROVIDER *xpl;

  pl=GWEN_Plugin_new(pm, name, fileName);
  GWEN_NEW_OBJECT(AB_PLUGIN_PROVIDER, xpl);
  GWEN_INHERIT_SETDATA(GWEN_PLUGIN, AB_PLUGIN_PROVIDER, pl, xpl,
		       AB_Plugin_Provider_FreeData);

  return pl;

}



void GWENHYWFAR_CB AB_Plugin_Provider_FreeData(void *bp, void *p) {
  AB_PLUGIN_PROVIDER *xpl;

  xpl=(AB_PLUGIN_PROVIDER*)p;
  GWEN_FREE_OBJECT(xpl);
}



AB_PROVIDER *AB_Plugin_Provider_Factory(GWEN_PLUGIN *pl, AB_BANKING *ab) {
  AB_PLUGIN_PROVIDER *xpl;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, AB_PLUGIN_PROVIDER, pl);
  assert(xpl);

  assert(xpl->pluginFactoryFn);
  return xpl->pluginFactoryFn(pl, ab);
}



void AB_Plugin_Provider_SetFactoryFn(GWEN_PLUGIN *pl,
				     AB_PLUGIN_PROVIDER_FACTORY_FN fn) {
  AB_PLUGIN_PROVIDER *xpl;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, AB_PLUGIN_PROVIDER, pl);
  assert(xpl);

  xpl->pluginFactoryFn=fn;
}












