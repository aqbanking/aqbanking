/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */


AB_PROVIDER *AB_Banking_BeginUseProvider(AB_BANKING *ab, const char *modname){
  GWEN_PLUGIN *pl;

  pl=GWEN_PluginManager_GetPlugin(ab_pluginManagerProvider, modname);
  if (pl) {
    AB_PROVIDER *pro;
    GWEN_DB_NODE *db=NULL;
    int rv;

    pro=AB_Plugin_Provider_Factory(pl, ab);
    if (!pro) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in plugin [%s]: No provider created", modname);
      return NULL;
    }

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



int AB_Banking_EndUseProvider(AB_BANKING *ab, AB_PROVIDER *pro){
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



GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetProviderDescrs(AB_BANKING *ab){
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
    while(pd) {
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
			     GWEN_CRYPT_TOKEN **pCt) {
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
    while(ct) {
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



void AB_Banking_ClearCryptTokenList(AB_BANKING *ab) {
  GWEN_CRYPT_TOKEN_LIST2_ITERATOR *it;

  assert(ab);
  assert(ab->cryptTokenList);

  it=GWEN_Crypt_Token_List2_First(ab->cryptTokenList);
  if (it) {
    GWEN_CRYPT_TOKEN *ct;

    ct=GWEN_Crypt_Token_List2Iterator_Data(it);
    assert(ct);
    while(ct) {
      while(GWEN_Crypt_Token_IsOpen(ct)) {
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
			       GWEN_BUFFER *tokenName) {
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
                       uint32_t pid) {
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





