/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */


#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQNONE
# include "src/libs/plugins/backends/aqnone/provider_l.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQHBCI
# include "src/libs/plugins/backends/aqhbci/banking/provider.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQOFXCONNECT
# include "src/libs/plugins/backends/aqofxconnect/provider.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQPAYPAL
# include "src/libs/plugins/backends/aqpaypal/provider_l.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQEBICS
# include "src/libs/plugins/backends/aqebics/client/provider.h"
#endif




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int _sendCommandsInsideProgress(AB_BANKING *ab, AB_TRANSACTION_LIST2 *commandList,
                                       AB_IMEXPORTER_CONTEXT *ctx,
                                       uint32_t pid);

static int _sortCommandsByAccounts(AB_BANKING *ab,
                                   AB_TRANSACTION_LIST2 *commandList,
                                   AB_ACCOUNTQUEUE_LIST *aql,
                                   uint32_t pid);

static int _sortAccountQueuesByProvider(AB_BANKING *ab,
                                        AB_ACCOUNTQUEUE_LIST *aql,
                                        AB_PROVIDERQUEUE_LIST *pql,
                                        uint32_t pid);

static int _sendProviderQueues(AB_BANKING *ab,
                               AB_PROVIDERQUEUE_LIST *pql,
                               AB_IMEXPORTER_CONTEXT *ctx,
                               uint32_t pid);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */




AB_PROVIDER *AB_Banking__CreateInternalProvider(AB_BANKING *ab, const char *modname)
{
  if (modname && *modname) {
#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQHBCI
    if (strcasecmp(modname, "aqhbci")==0) {
      AB_PROVIDER *pro;

      DBG_INFO(AQBANKING_LOGDOMAIN, "Plugin [%s] compiled-in", modname);
      pro=AH_Provider_new(ab, modname);
      return pro;
    }
#endif

#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQNONE
    if (strcasecmp(modname, "aqnone")==0) {
      AB_PROVIDER *pro;

      DBG_INFO(AQBANKING_LOGDOMAIN, "Plugin [%s] compiled-in", modname);
      pro=AN_Provider_new(ab);
      return pro;
    }
#endif

#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQOFXCONNECT
    if (strcasecmp(modname, "aqofxconnect")==0) {
      AB_PROVIDER *pro;

      DBG_INFO(AQBANKING_LOGDOMAIN, "Plugin [%s] compiled-in", modname);
      pro=AO_Provider_new(ab);
      return pro;
    }
#endif

#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQPAYPAL
    if (strcasecmp(modname, "aqpaypal")==0) {
      AB_PROVIDER *pro;

      DBG_INFO(AQBANKING_LOGDOMAIN, "Plugin [%s] compiled-in", modname);
      pro=APY_Provider_new(ab);
      return pro;
    }
#endif

#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQEBICS
    if (strcasecmp(modname, "aqebics")==0) {
      AB_PROVIDER *pro;

      DBG_INFO(AQBANKING_LOGDOMAIN, "Plugin [%s] compiled-in", modname);
      pro=EBC_Provider_new(ab);
      return pro;
    }
#endif

  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Plugin [%s] not compiled-in", modname);
  }
  return NULL;
}



AB_PROVIDER *AB_Banking__FindProvider(AB_BANKING *ab, const char *name)
{
  AB_PROVIDER *pro;

  assert(ab);
  assert(name);
  pro=AB_Provider_List_First(ab_providers);
  while (pro) {
    if (strcasecmp(AB_Provider_GetName(pro), name)==0)
      break;
    pro=AB_Provider_List_Next(pro);
  } /* while */

  return pro;
}



AB_PROVIDER *AB_Banking__GetProvider(AB_BANKING *ab, const char *name)
{
  AB_PROVIDER *pro;

  assert(ab);
  assert(name);

  pro=AB_Banking__FindProvider(ab, name);
  if (pro)
    return pro;
  pro=AB_Banking__CreateInternalProvider(ab, name);
  if (pro)
    return pro;

  if (pro)
    AB_Provider_List_Add(pro, ab_providers);

  return pro;
}



AB_PROVIDER *AB_Banking_BeginUseProvider(AB_BANKING *ab, const char *modname)
{
  AB_PROVIDER *pro;

  pro=AB_Banking__GetProvider(ab, modname);
  if (pro) {
    GWEN_DB_NODE *db=NULL;
    int rv;

    rv=AB_Banking_ReadNamedConfigGroup(ab, AB_CFG_GROUP_BACKENDS, modname, 1, 1, &db);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      AB_Provider_free(pro);
      return NULL;
    }

    rv=AB_Provider_Init(pro, db);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_DB_Group_free(db);
      AB_Provider_free(pro);
      return NULL;
    }
    GWEN_DB_Group_free(db);

    return pro;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Plugin [%s] not found", modname);
    return NULL;
  }
}



int AB_Banking_EndUseProvider(AB_BANKING *ab, AB_PROVIDER *pro)
{
  int rv;
  GWEN_DB_NODE *db=NULL;

  assert(pro);

  rv=AB_Banking_ReadNamedConfigGroup(ab, AB_CFG_GROUP_BACKENDS, AB_Provider_GetName(pro), 1, 0, &db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    AB_Provider_free(pro);
    return rv;
  }

  rv=AB_Provider_Fini(pro, db);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_ConfigMgr_UnlockGroup(ab->configMgr, AB_CFG_GROUP_BACKENDS, AB_Provider_GetName(pro));
    GWEN_DB_Group_free(db);
    AB_Provider_free(pro);
    return rv;
  }

  rv=AB_Banking_WriteNamedConfigGroup(ab, AB_CFG_GROUP_BACKENDS, AB_Provider_GetName(pro), 0, 1, db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_ConfigMgr_UnlockGroup(ab->configMgr, AB_CFG_GROUP_BACKENDS, AB_Provider_GetName(pro));
    GWEN_DB_Group_free(db);
    AB_Provider_free(pro);
    return rv;
  }
  GWEN_DB_Group_free(db);
  AB_Provider_free(pro);

  return 0;
}



int AB_Banking_ProviderControl(AB_BANKING *ab, const char *backendName, int argc, char **argv)
{
  AB_PROVIDER *pro;

  pro=AB_Banking_BeginUseProvider(ab, backendName);
  if (pro==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Provider \"%s\" not available", backendName?backendName:"<no name>");
    return GWEN_ERROR_NOT_FOUND;
  }
  else {
    int rv;

    rv=AB_Provider_Control(pro, argc, argv);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    }
    else if (rv>0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Error in provider control function (%d)", rv);
    }
    AB_Banking_EndUseProvider(ab, pro);
    return rv;
  }
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetProviderDescrs(AB_BANKING *ab)
{
  GWEN_PLUGIN_DESCRIPTION_LIST2 *l;
  GWEN_PLUGIN_MANAGER *pm;

  pm = GWEN_PluginManager_FindPluginManager("provider");
  if (!pm) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not find plugin manager for \"%s\"",
              "provider");
    return 0;
  }

  l = GWEN_PluginManager_GetPluginDescrs(pm);
  if (l) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;
    GWEN_PLUGIN_DESCRIPTION *pd;

    it=GWEN_PluginDescription_List2_First(l);
    assert(it);
    pd=GWEN_PluginDescription_List2Iterator_Data(it);
    assert(pd);
    while (pd) {
      GWEN_PluginDescription_SetIsActive(pd, 1);
      pd=GWEN_PluginDescription_List2Iterator_Next(it);
    }
    GWEN_PluginDescription_List2Iterator_free(it);
  }

  return l;
}



int AB_Banking_GetCryptToken(AB_BANKING *ab,
                             const char *tname,
                             const char *cname,
                             GWEN_CRYPT_TOKEN **pCt)
{
  GWEN_CRYPT_TOKEN *ct=NULL;
  GWEN_CRYPT_TOKEN_LIST2_ITERATOR *it;

  assert(ab);

  assert(pCt);

  if (!tname || !cname) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error in your configuration: TokenType \"%s\" or TokenName \"%s\" is NULL. Maybe you need to remove your configuration and create it again? Aborting.",
              tname ? tname : "NULL",
              cname ? cname : "NULL");
    return GWEN_ERROR_IO;
  }

  it=GWEN_Crypt_Token_List2_First(ab->cryptTokenList);
  if (it) {
    ct=GWEN_Crypt_Token_List2Iterator_Data(it);
    assert(ct);
    while (ct) {
      const char *s1;
      const char *s2;

      s1=GWEN_Crypt_Token_GetTypeName(ct);
      s2=GWEN_Crypt_Token_GetTokenName(ct);
      assert(s1);
      assert(s2);
      if (strcasecmp(s1, tname)==0 &&
          strcasecmp(s2, cname)==0)
        break;
      ct=GWEN_Crypt_Token_List2Iterator_Next(it);
    }
    GWEN_Crypt_Token_List2Iterator_free(it);
  }

  if (ct==NULL) {
    GWEN_PLUGIN_MANAGER *pm;
    GWEN_PLUGIN *pl;

    /* get crypt token */
    pm=GWEN_PluginManager_FindPluginManager(GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
    if (pm==0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "CryptToken plugin manager not found");
      return GWEN_ERROR_INTERNAL;
    }

    pl=GWEN_PluginManager_GetPlugin(pm, tname);
    if (pl==0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Plugin \"%s\" not found", tname);
      return GWEN_ERROR_NOT_FOUND;
    }

    ct=GWEN_Crypt_Token_Plugin_CreateToken(pl, cname);
    if (ct==0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create crypt token");
      return GWEN_ERROR_IO;
    }

    if (GWEN_Gui_GetFlags(GWEN_Gui_GetGui()) & GWEN_GUI_FLAGS_NONINTERACTIVE)
      /* in non-interactive mode, so don't use the secure pin input of card readers because
       * that wouldn't give us a chance to inject the pin via a pinfile
       */
      GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_FORCE_PIN_ENTRY);

    /* add to internal list */
    GWEN_Crypt_Token_List2_PushBack(ab->cryptTokenList, ct);
  }

  *pCt=ct;
  return 0;
}



void AB_Banking_ClearCryptTokenList(AB_BANKING *ab)
{
  GWEN_CRYPT_TOKEN_LIST2_ITERATOR *it;

  assert(ab);
  assert(ab->cryptTokenList);

  it=GWEN_Crypt_Token_List2_First(ab->cryptTokenList);
  if (it) {
    GWEN_CRYPT_TOKEN *ct;

    ct=GWEN_Crypt_Token_List2Iterator_Data(it);
    assert(ct);
    while (ct) {
      while (GWEN_Crypt_Token_IsOpen(ct)) {
        int rv;

        rv=GWEN_Crypt_Token_Close(ct, 0, 0);
        if (rv) {
          DBG_WARN(AQBANKING_LOGDOMAIN,
                   "Could not close crypt token [%s:%s], abandoning (%d)",
                   GWEN_Crypt_Token_GetTypeName(ct),
                   GWEN_Crypt_Token_GetTokenName(ct),
                   rv);
          GWEN_Crypt_Token_Close(ct, 1, 0);
        }
      }
      GWEN_Crypt_Token_free(ct);
      ct=GWEN_Crypt_Token_List2Iterator_Next(it);
    }
    GWEN_Crypt_Token_List2Iterator_free(it);
  }
  GWEN_Crypt_Token_List2_Clear(ab->cryptTokenList);
}



int AB_Banking_CheckCryptToken(AB_BANKING *ab,
                               GWEN_CRYPT_TOKEN_DEVICE devt,
                               GWEN_BUFFER *typeName,
                               GWEN_BUFFER *tokenName)
{
  GWEN_PLUGIN_MANAGER *pm;
  int rv;

  /* get crypt token */
  pm=GWEN_PluginManager_FindPluginManager(GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
  if (pm==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "CryptToken plugin manager not found");
    return GWEN_ERROR_NOT_FOUND;
  }

  /* try to determine the type and name */
  rv=GWEN_Crypt_Token_PluginManager_CheckToken(pm,
                                               devt,
                                               typeName,
                                               tokenName,
                                               0);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_GetCert(AB_BANKING *ab,
                       const char *url,
                       const char *defaultProto,
                       int defaultPort,
                       uint32_t *httpFlags,
                       uint32_t pid)
{
  int rv;
  GWEN_HTTP_SESSION *sess;

  sess=GWEN_HttpSession_new(url, defaultProto, defaultPort);
  GWEN_HttpSession_SetFlags(sess, *httpFlags);

  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Gui_ProgressLog2(pid,
                          GWEN_LoggerLevel_Error,
                          I18N("Could not init HTTP session  (%d)"), rv);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  rv=GWEN_HttpSession_ConnectionTest(sess);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not connect to server (%d)", rv);
    GWEN_Gui_ProgressLog2(pid,
                          GWEN_LoggerLevel_Error,
                          I18N("Could not connect to server, giving up (%d)"), rv);
    return rv;
  }

  *httpFlags=GWEN_HttpSession_GetFlags(sess);

  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  GWEN_Gui_ProgressLog(pid,
                       GWEN_LoggerLevel_Notice,
                       I18N("Connection ok, certificate probably received"));

  return 0;
}




int AB_Banking_SendCommands(AB_BANKING *ab, AB_TRANSACTION_LIST2 *commandList, AB_IMEXPORTER_CONTEXT *ctx)
{
  uint32_t pid;
  int rv;

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
                             GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                             GWEN_GUI_PROGRESS_SHOW_LOG |
                             GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
                             GWEN_GUI_PROGRESS_KEEP_OPEN |
                             GWEN_GUI_PROGRESS_SHOW_ABORT,
                             I18N("Executing Jobs"),
                             I18N("Now the jobs are send via their "
                                  "backends to the credit institutes."),
                             0, /* no progress count */
                             0);
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice, "AqBanking v"AQBANKING_VERSION_FULL_STRING);
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Notice, I18N("Sending jobs to the bank(s)"));

  rv=_sendCommandsInsideProgress(ab, commandList, ctx, pid);
  AB_Banking_ClearCryptTokenList(ab);
  if (rv) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
  }

  GWEN_Gui_ProgressEnd(pid);
  return rv;
}



int _sendCommandsInsideProgress(AB_BANKING *ab, AB_TRANSACTION_LIST2 *commandList, AB_IMEXPORTER_CONTEXT *ctx,
                                uint32_t pid)
{
  AB_ACCOUNTQUEUE_LIST *aql;
  AB_PROVIDERQUEUE_LIST *pql;
  int rv;

  /* sort commands by account */
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Info, I18N("Sorting commands by account"));
  aql=AB_AccountQueue_List_new();
  rv=_sortCommandsByAccounts(ab, commandList, aql, pid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    AB_AccountQueue_List_free(aql);
    return rv;
  }

  /* sort account queues by provider */
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Info, I18N("Sorting commands by provider"));
  pql=AB_ProviderQueue_List_new();
  rv=_sortAccountQueuesByProvider(ab, aql, pql, pid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    AB_ProviderQueue_List_free(pql);
    AB_AccountQueue_List_free(aql);
    return rv;
  }
  AB_AccountQueue_List_free(aql); /* no longer needed */

  /* send to each backend */
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Info, I18N("Send commands to providers"));
  rv=_sendProviderQueues(ab, pql, ctx, pid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  AB_ProviderQueue_List_free(pql);

  /* done */
  return 0;
}



int _sortCommandsByAccounts(AB_BANKING *ab,
                            AB_TRANSACTION_LIST2 *commandList,
                            AB_ACCOUNTQUEUE_LIST *aql,
                            uint32_t pid)
{
  AB_TRANSACTION_LIST2_ITERATOR *jit;
  AB_ACCOUNTQUEUE *aq;

  /* sort commands by account */
  GWEN_Gui_ProgressLog(pid, GWEN_LoggerLevel_Info, I18N("Sorting commands by account"));
  jit=AB_Transaction_List2_First(commandList);
  if (jit) {
    AB_TRANSACTION *t;

    t=AB_Transaction_List2Iterator_Data(jit);
    while (t) {
      AB_TRANSACTION_STATUS tStatus;

      tStatus=AB_Transaction_GetStatus(t);
      if (tStatus==AB_Transaction_StatusUnknown || tStatus==AB_Transaction_StatusNone ||
          tStatus==AB_Transaction_StatusEnqueued) {
        uint32_t uid;

        uid=AB_Transaction_GetUniqueAccountId(t);
        if (uid==0) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "No unique account id given in transaction, aborting");
          return GWEN_ERROR_BAD_DATA;
        }

        /* get or create account queue */
        aq=AB_AccountQueue_List_GetByAccountId(aql, uid);
        if (aq==NULL) {
          aq=AB_AccountQueue_new();
          AB_AccountQueue_SetAccountId(aq, uid);
          AB_AccountQueue_List_Add(aq, aql);
        }

        /* assign unique id to job (if none) */
        if (AB_Transaction_GetUniqueId(t)==0)
          AB_Transaction_SetUniqueId(t, AB_Banking_GetNamedUniqueId(ab, "jobid", 1));
        AB_Transaction_SetRefUniqueId(t, 0);
        /* set status */
        AB_Transaction_SetStatus(t, AB_Transaction_StatusEnqueued);
        /* add to queue */
        AB_AccountQueue_AddTransaction(aq, t);
      } /* if status matches */
      else {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Transaction with bad status, not enqueuing (%d: %s)",
                  tStatus, AB_Transaction_Status_toString(tStatus));
        /* TODO: change status, add to im-/export context */
      }

      t=AB_Transaction_List2Iterator_Next(jit);
    }
    AB_Transaction_List2Iterator_free(jit);
  } /* if (jit) */

  return 0;
}



int _sortAccountQueuesByProvider(AB_BANKING *ab,
                                 AB_ACCOUNTQUEUE_LIST *aql,
                                 AB_PROVIDERQUEUE_LIST *pql,
                                 uint32_t pid)
{
  AB_ACCOUNTQUEUE *aq;
  AB_PROVIDERQUEUE *pq;
  int rv;

  /* sort account queues by provider */
  while ((aq=AB_AccountQueue_List_First(aql))) {
    uint32_t uid;
    AB_ACCOUNT_SPEC *as=NULL;
    const char *s;

    uid=AB_AccountQueue_GetAccountId(aq);
    rv=AB_Banking_GetAccountSpecByUniqueId(ab, uid, &as);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to load account spec for account %lu (%d)", (unsigned long int)uid, rv);
      return GWEN_ERROR_BAD_DATA;
    }
    AB_AccountQueue_SetAccountSpec(aq, as);

    s=AB_AccountSpec_GetBackendName(as);
    if (!(s && *s)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Account spec for account %lu has no backend setting", (unsigned long int)uid);
      return GWEN_ERROR_BAD_DATA;
    }

    pq=AB_ProviderQueue_List_GetByProviderName(pql, s);
    if (pq==NULL) {
      pq=AB_ProviderQueue_new();
      AB_ProviderQueue_SetProviderName(pq, s);

      AB_ProviderQueue_List_Add(pq, pql);
    }

    AB_AccountQueue_List_Del(aq);
    AB_ProviderQueue_AddAccountQueue(pq, aq);
  }

  return 0;
}



int _sendProviderQueues(AB_BANKING *ab,
                        AB_PROVIDERQUEUE_LIST *pql,
                        AB_IMEXPORTER_CONTEXT *ctx,
                        uint32_t pid)
{
  AB_PROVIDERQUEUE *pq;
  int rv;

  pq=AB_ProviderQueue_List_First(pql);
  while (pq) {
    AB_PROVIDERQUEUE *pqNext;
    const char *providerName;

    pqNext=AB_ProviderQueue_List_Next(pq);
    AB_ProviderQueue_List_Del(pq);

    providerName=AB_ProviderQueue_GetProviderName(pq);
    if (providerName && *providerName) {
      AB_PROVIDER *pro;

      pro=AB_Banking_BeginUseProvider(ab, providerName);
      if (pro) {
        AB_IMEXPORTER_CONTEXT *localCtx;

        GWEN_Gui_ProgressLog2(pid, GWEN_LoggerLevel_Info, I18N("Send commands to provider \"%s\""), providerName);
        localCtx=AB_ImExporterContext_new();
        rv=AB_Provider_SendCommands(pro, pq, localCtx);
        if (rv<0) {
          GWEN_Gui_ProgressLog2(pid, GWEN_LoggerLevel_Error, I18N("Error Sending commands to provider \"%s\":%d"), providerName,
                                rv);
          DBG_INFO(AQBANKING_LOGDOMAIN, "Error sending commands to provider \"%s\" (%d)", AB_Provider_GetName(pro), rv);
        }
        AB_ImExporterContext_AddContext(ctx, localCtx);
        AB_Banking_EndUseProvider(ab, pro);
      }
      else {
        GWEN_Gui_ProgressLog2(pid, GWEN_LoggerLevel_Info, I18N("Provider \"%s\" is not available."), providerName);
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not start using provider \"%s\"", providerName);
      }
    }
    AB_ProviderQueue_free(pq);

    pq=pqNext;
  }
  return 0;
}




uint32_t AB_Banking_ReserveJobId(AB_BANKING *ab)
{
  return AB_Banking_GetNamedUniqueId(ab, "jobid", 1);
}


