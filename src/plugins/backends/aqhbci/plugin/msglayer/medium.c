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

#include <gwenhywfar/misc.h>

#include "medium_p.h"
#include "aqhbci_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/md.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(AH_MEDIUM, AH_Medium);



AH_MEDIUM *AH_Medium_new(AH_HBCI *hbci,
                         const char *typeName,
                         const char *subTypeName,
                         const char *mediumName) {
  AH_MEDIUM *m;

  assert(hbci);
  assert(typeName);
  GWEN_NEW_OBJECT(AH_MEDIUM, m);
  m->usage=1;
  GWEN_LIST_INIT(AH_MEDIUM, m);
  m->hbci=hbci;
  m->typeName=strdup(typeName);
  if (subTypeName)
    m->subTypeName=strdup(subTypeName);
  if (mediumName)
    m->mediumName=strdup(mediumName);

  m->contextList=AH_MediumCtx_List_new();

  m->selected=-1;

  return m;
}



void AH_Medium_Attach(AH_MEDIUM *m){
  assert(m);
  m->usage++;
}



void AH_Medium_free(AH_MEDIUM *m){
  if (m) {
    assert(m->usage);
    m->usage--;
    if (m->usage==0) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Destroying AH_MEDIUM");
      GWEN_LIST_FINI(AH_MEDIUM, m);
      AH_MediumCtx_List_free(m->contextList);
      free(m->subTypeName);
      free(m->typeName);
      free(m->mediumName);
      free(m);
    }
  }
}



const char *AH_Medium_GetDescriptiveName(const AH_MEDIUM *m) {
  assert(m);
  if (m->descriptiveName)
    return m->descriptiveName;
  else
    return m->mediumName;
}



void AH_Medium_SetDescriptiveName(AH_MEDIUM *m, const char *s){
  assert(m);
  free(m->descriptiveName);
  if (s) m->descriptiveName=strdup(s);
  else m->descriptiveName=0;
}



const char *AH_Medium_GetMediumName(const AH_MEDIUM *m){
  assert(m);
  return m->mediumName;
}



void AH_Medium_SetMediumName(AH_MEDIUM *m, const char *s){
  assert(m);
  free(m->mediumName);
  if (s) m->mediumName=strdup(s);
  else m->mediumName=0;
}



GWEN_TYPE_UINT32 AH_Medium_GetFlags(const AH_MEDIUM *m){
  assert(m);
  return m->flags;
}



void AH_Medium_SetFlags(AH_MEDIUM *m, GWEN_TYPE_UINT32 fl){
  assert(m);
  m->flags=fl;
}



void AH_Medium_AddFlags(AH_MEDIUM *m, GWEN_TYPE_UINT32 fl){
  assert(m);
  m->flags|=fl;
}



void AH_Medium_SubFlags(AH_MEDIUM *m, GWEN_TYPE_UINT32 fl){
  assert(m);
  m->flags&=~fl;
}



int AH_Medium__ReadContextsFromToken(AH_MEDIUM *m, GWEN_CRYPTTOKEN *ct){
  GWEN_CRYPTTOKEN_USER_LIST *ul;
  int rv;

  assert(m);

  /* first try to create by users */
  ul=GWEN_CryptToken_User_List_new();
  rv=GWEN_CryptToken_FillUserList(ct, ul);
  if (rv==0) {
    GWEN_CRYPTTOKEN_USER *u;

    /* there are users */
    DBG_ERROR(AQHBCI_LOGDOMAIN, "There are users");
    u=GWEN_CryptToken_User_List_First(ul);
    while(u) {
      GWEN_TYPE_UINT32 cid;

      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Checking user \"%s\"",
                GWEN_CryptToken_User_GetUserId(u));

      cid=GWEN_CryptToken_User_GetContextId(u);
      if (cid) {
        const GWEN_CRYPTTOKEN_CONTEXT *tctx;

        tctx=GWEN_CryptToken_GetContextById(ct, cid);
        if (tctx) {
          AH_MEDIUM_CTX *ctx;
          GWEN_CRYPTTOKEN_CONTEXT *nctx;
          const GWEN_CRYPTTOKEN_KEYINFO *ki;
          const GWEN_CRYPTTOKEN_SIGNINFO *si;
	  const GWEN_CRYPTTOKEN_CRYPTINFO *ci;
          GWEN_CRYPTTOKEN_CRYPTALGO algo;

          nctx=GWEN_CryptToken_Context_dup(tctx);
          assert(nctx);

          ki=GWEN_CryptToken_Context_GetSignKeyInfo(nctx);
	  assert(ki);
	  algo=GWEN_CryptToken_KeyInfo_GetCryptAlgo(ki);
	  if (algo==GWEN_CryptToken_CryptAlgo_RSA) {
            /* select cryptInfo and signInfo for RDH mode */
	    si=GWEN_CryptToken_GetSignInfoByAlgos(ct,
						  GWEN_CryptToken_HashAlgo_RMD160,
						  GWEN_CryptToken_PaddAlgo_ISO9796_1A4);
	    ci=GWEN_CryptToken_GetCryptInfoByAlgos(ct,
						   GWEN_CryptToken_CryptAlgo_RSA,
						   GWEN_CryptToken_PaddAlgo_LeftZero);
	  }
	  else if (algo==GWEN_CryptToken_CryptAlgo_DES_3K) {
	    /* select cryptInfo and signInfo for DDV mode */
	    si=GWEN_CryptToken_GetSignInfoByAlgos(ct,
						  GWEN_CryptToken_HashAlgo_RMD160,
						  GWEN_CryptToken_PaddAlgo_None);
	    ci=GWEN_CryptToken_GetCryptInfoByAlgos(ct,
						   GWEN_CryptToken_CryptAlgo_DES_3K,
						   GWEN_CryptToken_PaddAlgo_None);
	  }
	  else {
	    DBG_ERROR(AQHBCI_LOGDOMAIN,
		      "Invalid crypt algo \"%s\"",
		      GWEN_CryptToken_CryptAlgo_toString(algo));
	    si=0; ci=0;
	  }

	  if (ci && si) {
	    GWEN_CryptToken_Context_SetSignInfo(nctx, si);
	    GWEN_CryptToken_Context_SetCryptInfo(nctx, ci);

	    ctx=AH_MediumCtx_new();
	    AH_MediumCtx_SetUser(ctx, u);
	    AH_MediumCtx_SetTokenContext(ctx, nctx);
	    DBG_ERROR(AQHBCI_LOGDOMAIN,
		      "Adding user \"%s\"",
		      GWEN_CryptToken_User_GetUserId(u));
	    AH_MediumCtx_List_Add(ctx, m->contextList);
	  }
	  GWEN_CryptToken_Context_free(nctx);
	}
        else {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Context %d not found", (int)cid);
        }
      }
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No context id");
      }
      u=GWEN_CryptToken_User_List_Next(u);
    }
  }
  GWEN_CryptToken_User_List_free(ul);

  /* then try to read contexts only */
  if (AH_MediumCtx_List_GetCount(m->contextList)==0) {
    GWEN_CRYPTTOKEN_CONTEXT_LIST *cl;

    cl=GWEN_CryptToken_Context_List_new();
    rv=GWEN_CryptToken_FillContextList(ct, cl);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not fill context list (%d)", rv);
      GWEN_CryptToken_Context_List_free(cl);
      return AB_ERROR_NO_DATA;
    }
    else {
      GWEN_CRYPTTOKEN_CONTEXT *tctx;

      tctx=GWEN_CryptToken_Context_List_First(cl);
      while (tctx) {
        AH_MEDIUM_CTX *ctx;

        ctx=AH_MediumCtx_new();
        AH_MediumCtx_SetTokenContext(ctx, tctx);
        AH_MediumCtx_List_Add(ctx, m->contextList);
        tctx=GWEN_CryptToken_Context_List_Next(tctx);
      }
    }
    GWEN_CryptToken_Context_List_free(cl);
  }

  /* still no contexts? */
  if (AH_MediumCtx_List_GetCount(m->contextList)==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Could not read any kind of context from crypt token");
    return AB_ERROR_NO_DATA;
  }

  return 0;
}



int AH_Medium__MountCt(AH_MEDIUM *m){
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPTTOKEN *ct;
  int rv;

  assert(m);

  /* get crypt token */
  pm=GWEN_PluginManager_FindPluginManager("crypttoken");
  if (pm==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Plugin manager not found");
    return AB_ERROR_GENERIC;
  }

  pl=GWEN_PluginManager_GetPlugin(pm, m->typeName);
  if (pl==0) {
    DBG_ERROR(0, "Plugin not found");
    return AB_ERROR_NOT_FOUND;
  }

  ct=GWEN_CryptToken_Plugin_CreateToken(pl,
                                        m->subTypeName,
                                        m->mediumName);
  if (ct==0) {
    DBG_ERROR(0, "Could not create crypt token");
    return AB_ERROR_GENERIC;
  }

  /* set descriptive name as stored in AqHBCI */
  GWEN_CryptToken_SetDescriptiveName(ct,
                                     m->descriptiveName);

  /* open crypt token */
  rv=GWEN_CryptToken_Open(ct, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not open crypt token (%d)", rv);
    GWEN_CryptToken_free(ct);
    return AB_ERROR_GENERIC;
  }

  if (m->flags)
    GWEN_CryptToken_AddFlags(ct, m->flags);

  /* fill context list if necessary */
  if (AH_MediumCtx_List_GetCount(m->contextList)==0) {
    rv=AH_Medium__ReadContextsFromToken(m, ct);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_CryptToken_free(ct);
      return rv;
    }
  }

  m->cryptToken=ct;
  m->currentContext=0;
  m->selected=-1;

  return 0;
}



int AH_Medium_Mount(AH_MEDIUM *m){
  int rv;

  assert(m);

  if (m->mountCount==0) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Really mounting medium");
    m->selected=-1;

    rv=AH_Medium__MountCt(m);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error mounting medium (%d)", rv);
      return rv;
    }
  }
  else
    rv=0;
  m->mountCount++;
  return rv;
}



int AH_Medium_Create(AH_MEDIUM *m){
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPTTOKEN *ct;
  int rv;

  assert(m);
  if (AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is mounted");
    return -1;
  }

  /* reset */
  assert(m->cryptToken==0);
  DBG_ERROR(AQHBCI_LOGDOMAIN, "Clearing context list");
  AH_MediumCtx_List_Clear(m->contextList);
  m->currentContext=0;
  m->selected=-1;

  /* get crypt token */
  pm=GWEN_PluginManager_FindPluginManager("crypttoken");
  if (pm==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Plugin manager not found");
    return AB_ERROR_GENERIC;
  }

  pl=GWEN_PluginManager_GetPlugin(pm, m->typeName);
  if (pl==0) {
    DBG_ERROR(0, "Plugin not found");
    return AB_ERROR_NOT_FOUND;
  }

  ct=GWEN_CryptToken_Plugin_CreateToken(pl, m->subTypeName,
                                        AH_Medium_GetMediumName(m));
  if (ct==0) {
    DBG_ERROR(0, "Could not create crypt token");
    return AB_ERROR_GENERIC;
  }

  /* create crypt token */
  rv=GWEN_CryptToken_Create(ct);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create crypt token (%d)", rv);
    GWEN_CryptToken_free(ct);
    return rv;
  }

  /* read contexts */
  rv=AH_Medium__ReadContextsFromToken(m, ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_CryptToken_free(ct);
    return rv;
  }

  /* close crypt token immediately */
  rv=GWEN_CryptToken_Close(ct);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not close crypt token (%d)", rv);
    GWEN_CryptToken_free(ct);
    return rv;
  }

  return 0;
}



int AH_Medium_Unmount(AH_MEDIUM *m, int force){
  int rv;

  assert(m);

  if (m->mountCount) {
    if (m->mountCount==1 || force) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Really unmounting medium");

      /* close crypt token */
      rv=GWEN_CryptToken_Close(m->cryptToken);
    
      /* reset/free associated data */
      GWEN_CryptToken_free(m->cryptToken);
      m->cryptToken=0;
      m->currentContext=0;
      m->selected=-1;
    }
    else
      rv=0;

    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error unmounting medium (%d)", rv);
      return rv;
    }

    m->mountCount--;
    if (force)
      m->mountCount=0;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return -1;
  }

  return 0;
}



int AH_Medium_ChangePin(AH_MEDIUM *m){
  int rv;

  assert(m);
  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return -1;
  }

  rv=GWEN_CryptToken_ChangePin(m->cryptToken);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error changing pin (%d)", rv);
    return rv;
  }

  return 0;
}



int AH_Medium_IsMounted(AH_MEDIUM *m){
  assert(m);
  return m->mountCount!=0;
}



AH_MEDIUM_RESULT AH_Medium_Sign(AH_MEDIUM *m,
                                GWEN_BUFFER *msgbuf,
                                GWEN_BUFFER *signbuf){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  int rv;
  AH_MEDIUM_RESULT res;

  assert(m);
  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return -1;
  }

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return 0;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);

  rv=GWEN_CryptToken_Sign(m->cryptToken,
                          tctx,
                          GWEN_Buffer_GetStart(msgbuf),
                          GWEN_Buffer_GetUsedBytes(msgbuf),
                          signbuf);
  switch(rv) {
  case 0:
    res=AH_MediumResultOk;
    break;
  case GWEN_ERROR_CT_NOT_IMPLEMENTED:
  case GWEN_ERROR_CT_NOT_SUPPORTED:
    res=AH_MediumResultNotSupported;
    break;
  case GWEN_ERROR_CT_NO_KEY:
    res=AH_MediumResultNoKey;
    break;
  default:
    res=AH_MediumResultGenericError;
  }

  return res;


}



int AH_Medium_GetNextSignSeq(AH_MEDIUM *m){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  int rv;
  GWEN_TYPE_UINT32 signSeq;

  assert(m);
  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return -1;
  }

  assert(m);

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return 0;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);

  ki=GWEN_CryptToken_Context_GetSignKeyInfo(tctx);
  if (!ki) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No local sign key");
    return 0;
  }

  rv=GWEN_CryptToken_GetSignSeq(m->cryptToken,
                                GWEN_CryptToken_KeyInfo_GetKeyId(ki),
                                &signSeq);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error reading sign seq (%d)", rv);
    return 0;
  }

  return (int)signSeq;
}



AH_MEDIUM_RESULT AH_Medium_Decrypt(AH_MEDIUM *m,
                                   GWEN_BUFFER *msgbuf,
                                   GWEN_BUFFER *decryptbuf,
                                   GWEN_BUFFER *msgKeyBuffer){
  GWEN_ERRORCODE err;
  GWEN_BUFFER *kbuf;
  GWEN_CRYPTKEY *sessionKey;
  AH_MEDIUM_RESULT res;

  assert(m);

  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return AH_MediumResultGenericError;
  }

  /* create session key */
  GWEN_Buffer_Rewind(msgKeyBuffer);

  /* decrypt session key */
  kbuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=AH_Medium_DecryptKey(m, msgKeyBuffer, kbuf);
  if (res!=AH_MediumResultOk) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error decrypting message key");
    GWEN_Buffer_free(kbuf);
    return res;
  }

  if (GWEN_Buffer_GetUsedBytes(kbuf)<16) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Bad size of session key (%d bytes)",
              GWEN_Buffer_GetUsedBytes(kbuf));
    GWEN_Buffer_free(kbuf);
    return AH_MediumResultBadKey;
  }
  GWEN_Buffer_Crop(kbuf, GWEN_Buffer_GetUsedBytes(kbuf)-16, 16);

  sessionKey=GWEN_CryptKey_Factory("DES");
  assert(sessionKey);
  GWEN_CryptKey_SetData(sessionKey,
                        GWEN_Buffer_GetStart(kbuf),
                        16);
  GWEN_Buffer_free(kbuf);

  /* now decrypt the message using the new session key */
  err=GWEN_CryptKey_Decrypt(sessionKey,
			    msgbuf,
			    decryptbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_CryptKey_free(sessionKey);
    return AH_MediumResultBadKey;
  }
  GWEN_CryptKey_free(sessionKey);

  /* unpadd message */
  if (GWEN_Padd_UnpaddWithANSIX9_23(decryptbuf)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return AH_MediumResultBadKey;
  }

  return AH_MediumResultOk;
}



AH_MEDIUM_RESULT AH_Medium_Verify(AH_MEDIUM *m,
                                  GWEN_BUFFER *msgbuf,
                                  GWEN_BUFFER *signbuf,
                                  int signseq){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  int rv;
  AH_MEDIUM_RESULT res;
  const GWEN_KEYSPEC *ks;

  assert(m);
  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return -1;
  }

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return 0;
  }

  ks=AH_MediumCtx_GetRemoteSignKeySpec(m->currentContext);
  if (ks) {
    if (GWEN_KeySpec_GetStatus(ks)!=GWEN_CRYPTTOKEN_KEYSTATUS_ACTIVE)
      return AH_MediumResultNoKey;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No remote sign key");
    return AH_MediumResultNoKey;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);
  rv=GWEN_CryptToken_Verify(m->cryptToken,
                            tctx,
                            GWEN_Buffer_GetStart(msgbuf),
                            GWEN_Buffer_GetUsedBytes(msgbuf),
                            GWEN_Buffer_GetStart(signbuf),
                            GWEN_Buffer_GetUsedBytes(signbuf));
  switch(rv) {
  case 0:
    res=AH_MediumResultOk;
    break;
  case GWEN_ERROR_VERIFY:
    res=AH_MediumResultInvalidSignature;
    break;
  case GWEN_ERROR_CT_NOT_IMPLEMENTED:
  case GWEN_ERROR_CT_NOT_SUPPORTED:
    res=AH_MediumResultNotSupported;
    break;
  case GWEN_ERROR_CT_NO_KEY:
    res=AH_MediumResultNoKey;
    break;
  default:
    res=AH_MediumResultGenericError;
  }

  return res;
}



AH_MEDIUM_RESULT AH_Medium_EncryptKey(AH_MEDIUM *m,
                                      GWEN_BUFFER *srckey,
                                      GWEN_BUFFER *enckey){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  int rv;
  AH_MEDIUM_RESULT res;
  const GWEN_KEYSPEC *ks;

  assert(m);
  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return AH_MediumResultGenericError;
  }

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return 0;
  }

  ks=AH_MediumCtx_GetRemoteCryptKeySpec(m->currentContext);
  if (ks) {
    if (GWEN_KeySpec_GetStatus(ks)!=GWEN_CRYPTTOKEN_KEYSTATUS_ACTIVE)
      return AH_MediumResultNoKey;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);
  rv=GWEN_CryptToken_Encrypt(m->cryptToken,
                             tctx,
                             GWEN_Buffer_GetStart(srckey),
                             GWEN_Buffer_GetUsedBytes(srckey),
                             enckey);
  switch(rv) {
  case 0:
    res=AH_MediumResultOk;
    break;
  case GWEN_ERROR_CT_NOT_IMPLEMENTED:
  case GWEN_ERROR_CT_NOT_SUPPORTED:
    res=AH_MediumResultNotSupported;
    break;
  case GWEN_ERROR_CT_NO_KEY:
    res=AH_MediumResultNoKey;
    break;
  case GWEN_ERROR_ENCRYPT:
  default:
    res=AH_MediumResultGenericError;
  }

  return res;
}



AH_MEDIUM_RESULT AH_Medium_DecryptKey(AH_MEDIUM *m,
                                      GWEN_BUFFER *srckey,
                                      GWEN_BUFFER *deckey){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  GWEN_CRYPTTOKEN_CONTEXT *nctx;
  const GWEN_CRYPTTOKEN_CRYPTINFO *ci;
  int rv;
  AH_MEDIUM_RESULT res;

  assert(m);
  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return AH_MediumResultGenericError;
  }

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return 0;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);
  nctx=GWEN_CryptToken_Context_dup(tctx);
  ci=GWEN_CryptToken_Context_GetCryptInfo(nctx);
  assert(ci);

  if (GWEN_CryptToken_CryptInfo_GetPaddAlgo(ci)==
      GWEN_CryptToken_PaddAlgo_LeftZero) {
    GWEN_CRYPTTOKEN_CRYPTINFO *nci;

    DBG_ERROR(0, "Setting cryptinfo");
    nci=GWEN_CryptToken_CryptInfo_dup(ci);
    GWEN_CryptToken_CryptInfo_SetPaddAlgo(nci,
                                          GWEN_CryptToken_PaddAlgo_None);
    GWEN_CryptToken_Context_SetCryptInfo(nctx, nci);
    GWEN_CryptToken_CryptInfo_free(nci);
  }

  rv=GWEN_CryptToken_Decrypt(m->cryptToken,
                             nctx,
                             GWEN_Buffer_GetStart(srckey),
                             GWEN_Buffer_GetUsedBytes(srckey),
                             deckey);
  GWEN_CryptToken_Context_free(nctx);

  switch(rv) {
  case 0:
    res=AH_MediumResultOk;
    break;
  case GWEN_ERROR_CT_NOT_IMPLEMENTED:
  case GWEN_ERROR_CT_NOT_SUPPORTED:
    res=AH_MediumResultNotSupported;
    break;
  case GWEN_ERROR_CT_NO_KEY:
    res=AH_MediumResultNoKey;
    break;
  case GWEN_ERROR_DECRYPT:
  default:
    res=AH_MediumResultGenericError;
  }

  if (res==AH_MediumResultOk)
    GWEN_Buffer_Crop(deckey, GWEN_Buffer_GetUsedBytes(deckey)-16, 16);

  return res;
}



GWEN_BUFFER *AH_Medium_GenerateMsgKey(AH_MEDIUM *m) {
  GWEN_ERRORCODE err;
  GWEN_CRYPTKEY *sessionKey;
  GWEN_BUFFER *kbuf;
  unsigned char skbuffer[16];
  unsigned int sksize;

  assert(m);
  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return 0;
  }

  /* generate session key */
  sessionKey=GWEN_CryptKey_Factory("DES");
  if (!sessionKey) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create DES session key");
    return 0;
  }
  err=GWEN_CryptKey_Generate(sessionKey, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(AQHBCI_LOGDOMAIN, err);
    GWEN_CryptKey_free(sessionKey);
    return 0;
  }

  sksize=sizeof(skbuffer);
  err=GWEN_CryptKey_GetData(sessionKey,
                            (char*)skbuffer,
                            &sksize);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(AQHBCI_LOGDOMAIN, err);
    GWEN_CryptKey_free(sessionKey);
    return 0;
  }
  GWEN_CryptKey_free(sessionKey);

  kbuf=GWEN_Buffer_new(0, sksize, 0, 1);
  GWEN_Buffer_AppendBytes(kbuf, (char*)skbuffer, sksize);
  return kbuf;
}




AH_MEDIUM_RESULT AH_Medium_Encrypt(AH_MEDIUM *m,
                                   GWEN_BUFFER *msgbuf,
                                   GWEN_BUFFER *encryptbuf,
                                   GWEN_BUFFER *msgKeyBuffer){
  GWEN_CRYPTKEY *sessionKey;
  GWEN_BUFFER *kbuf;
  GWEN_ERRORCODE err;
  AH_MEDIUM_RESULT res;

  assert(m);

  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return AH_MediumResultGenericError;
  }

  sessionKey=GWEN_CryptKey_Factory("DES");
  if (!sessionKey) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create DES session key");
    return AH_MediumResultNoKey;
  }

  kbuf=AH_Medium_GenerateMsgKey(m);
  if (!kbuf) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not generate message key");
    return AH_MediumResultNoKey;
  }

  err=GWEN_CryptKey_SetData(sessionKey,
                            GWEN_Buffer_GetStart(kbuf),
                            GWEN_Buffer_GetUsedBytes(kbuf));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(AQHBCI_LOGDOMAIN, err);
    GWEN_Buffer_free(kbuf);
    GWEN_CryptKey_free(sessionKey);
    return AH_MediumResultNoKey;
  }

  /* padd message */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Padding with ANSI X9.23");
  if (GWEN_Padd_PaddWithANSIX9_23(msgbuf)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_Buffer_free(kbuf);
    GWEN_CryptKey_free(sessionKey);
    return AH_MediumResultGenericError;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Padding with ANSI X9.23: done");

  /* encrypt message */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Encrypting with session key");
  err=GWEN_CryptKey_Encrypt(sessionKey,
                            msgbuf,
                            encryptbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_Buffer_free(kbuf);
    GWEN_CryptKey_free(sessionKey);
    return AH_MediumResultGenericError;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Encrypting with session key: done");
  GWEN_CryptKey_free(sessionKey);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Encrypting key");
  res=AH_Medium_EncryptKey(m,
                           kbuf,
                           msgKeyBuffer);
  if (res!=AH_MediumResultOk) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not encrypt message key");
    GWEN_Buffer_free(kbuf);
    return res;
  }
  GWEN_Buffer_free(kbuf);
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Encrypting key: done");

  return AH_MediumResultOk;
}



const char *AH_Medium_GetMediumTypeName(const AH_MEDIUM *m){
  assert(m);
  return m->typeName;
}



const char *AH_Medium_GetMediumSubTypeName(const AH_MEDIUM *m){
  assert(m);
  return m->subTypeName;
}



int AH_Medium_ToDB(const AH_MEDIUM *m, GWEN_DB_NODE *db){
  AH_MEDIUM_CTX *ctx;
  GWEN_DB_NODE *dbX;

  GWEN_DB_SetCharValue(db,
                       GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "mediumTypeName", m->typeName);
  if (m->subTypeName)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "mediumSubTypeName", m->subTypeName);
  if (m->mediumName)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "mediumName", m->mediumName);

  GWEN_CryptToken_Flags_toDb(db, "flags", m->flags);

  if (m->descriptiveName)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "descriptiveName", m->descriptiveName);

  /* write context list */
  dbX=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "contextList");
  assert(dbX);
  ctx=AH_MediumCtx_List_First(m->contextList);
  while(ctx) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(dbX, GWEN_PATH_FLAGS_CREATE_GROUP, "context");
    assert(dbT);
    AH_MediumCtx_toDb(ctx, dbT);
    ctx=AH_MediumCtx_List_Next(ctx);
  }

  return 0;
}



int AH_Medium_FromDB(AH_MEDIUM *m, GWEN_DB_NODE *db){
  const char *p;
  GWEN_DB_NODE *dbT;

  /* mediumName */
  p=GWEN_DB_GetCharValue(db, "mediumName", 0, 0);
  if (p) {
    free(m->mediumName);
    m->mediumName=strdup(p);
  }

  /* descriptiveName */
  p=GWEN_DB_GetCharValue(db, "descriptiveName", 0, 0);
  if (p) {
    free(m->descriptiveName);
    m->descriptiveName=strdup(p);
  }

  /* flags */
  m->flags=GWEN_CryptToken_Flags_fromDb(db, "flags");

  /* read context list */
  dbT=GWEN_DB_FindFirstGroup(db, "contextList");
  if (dbT)
    dbT=GWEN_DB_FindFirstGroup(dbT, "context");
  while(dbT) {
    AH_MEDIUM_CTX *ctx;

    ctx=AH_MediumCtx_fromDb(dbT);
    if (ctx)
      AH_MediumCtx_List_Add(ctx, m->contextList);
    dbT=GWEN_DB_FindNextGroup(dbT, "context");
  }

  return 0;
}



AH_HBCI *AH_Medium_GetHBCI(const AH_MEDIUM *m){
  assert(m);
  return m->hbci;
}



int AH_Medium_SelectContext(AH_MEDIUM *m, int idx){

  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return AB_ERROR_INVALID;
  }

  if (idx==-1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid context idx %d", idx);
    return AB_ERROR_INVALID;
  }

  if (idx==m->selected && m->currentContext)
    return 0;
  else {
    AH_MEDIUM_CTX *ctx;
    int i;
    int rv;

    i=idx;
    ctx=AH_MediumCtx_List_First(m->contextList);
    while(i--)
      ctx=AH_MediumCtx_List_Next(ctx);

    if (!ctx) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Context %d not found", idx);
      return AB_ERROR_NOT_FOUND;
    }
    m->selected=idx;
    m->currentContext=ctx;

    /* read keyspecs */
    rv=AH_Medium__ReadKeySpecs(m);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Error reading keyspecs");
      m->selected=0;
      m->currentContext=0;
      return rv;
    }
  }

  return 0;
}



int AH_Medium_ReadContext(AH_MEDIUM *m,
                          int idx,
                          int *country,
                          GWEN_BUFFER *bankId,
                          GWEN_BUFFER *userId,
                          GWEN_BUFFER *server,
                          int *port){
  AH_MEDIUM_CTX *ctx;
  const GWEN_CRYPTTOKEN_USER *u;
  int i;
  const char *s;

  assert(m);

  i=idx;
  ctx=AH_MediumCtx_List_First(m->contextList);
  while(i--)
    ctx=AH_MediumCtx_List_Next(ctx);

  if (!ctx) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Context %d not found", idx);
    return AB_ERROR_NOT_FOUND;
  }

  u=AH_MediumCtx_GetUser(ctx);
  assert(u);

  if (country)
    *country=280;
  if (userId && (s=GWEN_CryptToken_User_GetUserId(u)))
    GWEN_Buffer_AppendString(userId, s);
  if (bankId && (s=GWEN_CryptToken_User_GetServiceId(u)))
    GWEN_Buffer_AppendString(bankId, s);
  if (server && (s=GWEN_CryptToken_User_GetAddress(u)))
    GWEN_Buffer_AppendString(server, s);
  if (port)
    *port=GWEN_CryptToken_User_GetPort(u);

  return 0;
}



int AH_Medium_WriteContext(AH_MEDIUM *m,
                           int idx,
                           int country,
                           const char *bankId,
                           const char *userId,
                           const char *server,
                           int port){
  AH_MEDIUM_CTX *ctx;
  GWEN_CRYPTTOKEN_USER *u;
  int i;
  int rv;

  assert(m);

  i=idx;
  ctx=AH_MediumCtx_List_First(m->contextList);
  while(i--)
    ctx=AH_MediumCtx_List_Next(ctx);

  if (!ctx) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Context %d not found", idx);
    return AB_ERROR_NOT_FOUND;
  }

  u=AH_MediumCtx_GetUser(ctx);
  assert(u);

  GWEN_CryptToken_User_SetUserId(u, userId);
  GWEN_CryptToken_User_SetServiceId(u, bankId);
  GWEN_CryptToken_User_SetAddress(u, server);
  GWEN_CryptToken_User_SetPort(u, port);

  rv=GWEN_CryptToken_ModifyUser(m->cryptToken, u);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AH_Medium_InputPin(AH_MEDIUM *m,
                       char *pwbuffer,
                       int minLen,
                       int maxLen,
                       int flags){
  int rv;
  const char *name;
  const char *numeric_warning = "";
  char buffer[512];

  assert(m);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;
  if (flags & AB_BANKING_INPUT_FLAGS_NUMERIC) {
    numeric_warning = I18N(" You must only enter numbers, not letters.");
  }
  if (flags & AB_BANKING_INPUT_FLAGS_CONFIRM) {
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please enter a new password for \n"
		  "%s\n"
		  "The password must be at least %d characters long.%s"
		  "<html>"
		  "Please enter a new password for <i>%s</i>. "
		  "The password must be at least %d characters long.%s"
		  "</html>"),
	     AH_Medium_GetDescriptiveName(m),
	     minLen,
	     numeric_warning,
	     AH_Medium_GetDescriptiveName(m),
	     minLen,
	     numeric_warning);
  }
  else {
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please enter the password for \n"
		  "%s\n"
		  "%s<html>"
		  "Please enter the password for <i>%s</i>.%s"
		  "</html>"),
	     AH_Medium_GetDescriptiveName(m),
	     numeric_warning,
	     AH_Medium_GetDescriptiveName(m),
	     numeric_warning);
  }

  name=AH_Medium_GetMediumName(m);
  if (name) {
    GWEN_BUFFER *nbuf;

    nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
    GWEN_Buffer_AppendString(nbuf, "PASSWORD::");
    GWEN_Buffer_AppendString(nbuf, name);
    rv=AB_Banking_GetPin(AH_HBCI_GetBankingApi(m->hbci),
                         flags,
                         GWEN_Buffer_GetStart(nbuf),
			 I18N("Enter Password"),
			 buffer,
			 pwbuffer,
                         minLen,
                         maxLen);
    GWEN_Buffer_free(nbuf);
  }
  else {
    rv=AB_Banking_InputBox(AH_HBCI_GetBankingApi(m->hbci),
                           flags,
			   I18N("Enter Password"),
                           buffer,
			   pwbuffer,
                           minLen,
                           maxLen);
  }
  return rv;
}



int AH_Medium_SetPinStatus(AH_MEDIUM *m,
                           const char *pin,
                           AB_BANKING_PINSTATUS status){
  const char *name;

  name=AH_Medium_GetMediumName(m);
  if (name) {
    GWEN_BUFFER *nbuf;
    int rv;

    nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
    GWEN_Buffer_AppendString(nbuf, "PASSWORD::");
    GWEN_Buffer_AppendString(nbuf, name);
    rv=AB_Banking_SetPinStatus(AH_HBCI_GetBankingApi(m->hbci),
                               GWEN_Buffer_GetStart(nbuf),
                               pin,
                               status);
    GWEN_Buffer_free(nbuf);
    return rv;
  }

  return 0;
}



int AH_Medium_InputTan(AH_MEDIUM *m,
		       char *pwbuffer,
                       int minLen,
                       int maxLen){
  int rv;
  const char *name;
  char buffer[512];

  assert(m);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;
  snprintf(buffer, sizeof(buffer)-1,
	   I18N("Please enter the next TAN\n"
		"for %s"
		"<html>"
		"Please enter the next TAN for <i>%s</i>"
		"</html>"),
	   AH_Medium_GetDescriptiveName(m),
	   AH_Medium_GetDescriptiveName(m));

  name=AH_Medium_GetMediumName(m);
  if (name) {
    rv=AB_Banking_GetTan(AH_HBCI_GetBankingApi(m->hbci),
			 name,
			 I18N("Enter TAN"),
			 buffer,
			 pwbuffer,
			 minLen,
			 maxLen);
  }
  else {
    rv=AB_Banking_InputBox(AH_HBCI_GetBankingApi(m->hbci),
			   AB_BANKING_INPUT_FLAGS_NUMERIC|
			   AB_BANKING_INPUT_FLAGS_SHOW,
			   I18N("Enter TAN"),
			   buffer,
			   pwbuffer,
			   minLen,
			   maxLen);
  }
  return rv;
}



int AH_Medium_SetTanStatus(AH_MEDIUM *m,
                           const char *tan,
                           AB_BANKING_TANSTATUS status){
  const char *name;

  name=AH_Medium_GetMediumName(m);
  if (name) {
    return AB_Banking_SetTanStatus(AH_HBCI_GetBankingApi(m->hbci),
                                   name,
                                   tan,
                                   status);
  }

  return 0;
}



AB_BANKING *AH_Medium_GetBankingApi(const AH_MEDIUM *m){
  assert(m);
  return AH_HBCI_GetBankingApi(m->hbci);
}



int AH_Medium_CreateKeys(AH_MEDIUM *m){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  const GWEN_CRYPTTOKEN_KEYINFO *kiS;
  const GWEN_CRYPTTOKEN_KEYINFO *kiC;
  GWEN_CRYPTKEY *key;
  int rv;

  assert(m);

  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return -1;
  }

  assert(key);

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return AB_ERROR_INVALID;
  }

  /* get keyinfos for local keys */
  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);

  kiS=GWEN_CryptToken_Context_GetSignKeyInfo(tctx);
  if (!kiS) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No local sign key specified");
    return AB_ERROR_NOT_FOUND;
  }

  kiC=GWEN_CryptToken_Context_GetDecryptKeyInfo(tctx);
  if (!kiC) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No local crypt key specified");
    return AB_ERROR_NOT_FOUND;
  }

  /* create local sign key */
  key=0;
  rv=GWEN_CryptToken_GenerateKey(m->cryptToken, kiS, &key);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error generating local sign key (%d)", rv);
    return rv;
  }
  GWEN_CryptKey_free(key);

  /* create local crypt key */
  key=0;
  rv=GWEN_CryptToken_GenerateKey(m->cryptToken, kiC, &key);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error generating local crypt key (%d)", rv);
    return rv;
  }

  GWEN_CryptKey_free(key);

  return 0;

}



AH_MEDIUM_CTX *AH_Medium_GetCurrentContext(AH_MEDIUM *m) {
  assert(m);

  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return 0;
  }

  return m->currentContext;
}



int AH_Medium__ReadKeySpec(AH_MEDIUM *m,
                           GWEN_TYPE_UINT32 kid,
                           GWEN_KEYSPEC **pks) {
  int rv;
  GWEN_KEYSPEC *ks;

  assert(m);
  assert(pks);

  /* maybe cache this data later */
  rv=GWEN_CryptToken_ReadKeySpec(m->cryptToken, kid, &ks);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Unable to read keyspec %x (%d)",
             kid, rv);
    return rv;
  }
  assert(ks);

  if (GWEN_KeySpec_GetStatus(ks)!=GWEN_CRYPTTOKEN_KEYSTATUS_ACTIVE) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Key %x is not active (%d)",
             kid,
             GWEN_KeySpec_GetStatus(ks));
    GWEN_KeySpec_free(ks);
    ks=0;
    return GWEN_ERROR_CT_NO_KEY;
  }

  *pks=ks;
  return 0;
}



int AH_Medium__ReadKeySpecs(AH_MEDIUM *m) {
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  GWEN_KEYSPEC *ks;
  int rv;

  assert(m);

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return AB_ERROR_INVALID;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);

  AH_MediumCtx_SetLocalSignKeySpec(m->currentContext, 0);
  AH_MediumCtx_SetLocalCryptKeySpec(m->currentContext, 0);
  AH_MediumCtx_SetRemoteSignKeySpec(m->currentContext, 0);
  AH_MediumCtx_SetRemoteCryptKeySpec(m->currentContext, 0);


  /* local signKey spec */
  ki=GWEN_CryptToken_Context_GetSignKeyInfo(tctx);
  if (!ki) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No local sign key");
    return AB_ERROR_NO_DATA;
  }
  else {
    ks=0;
    rv=AH_Medium__ReadKeySpec(m, GWEN_CryptToken_KeyInfo_GetKeyId(ki), &ks);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error reading keyspec (%d)", rv);
    }
    else {
      assert(ks);
      AH_MediumCtx_SetLocalSignKeySpec(m->currentContext, ks);
      GWEN_KeySpec_free(ks);
    }
  }

  /* local cryptKey spec */
  ki=GWEN_CryptToken_Context_GetDecryptKeyInfo(tctx);
  if (!ki) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No local crypt key");
    return AB_ERROR_NO_DATA;
  }
  else {
    ks=0;
    rv=AH_Medium__ReadKeySpec(m, GWEN_CryptToken_KeyInfo_GetKeyId(ki), &ks);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error reading keyspec (%d)", rv);
    }
    else {
      assert(ks);
      AH_MediumCtx_SetLocalCryptKeySpec(m->currentContext, ks);
      GWEN_KeySpec_free(ks);
    }
  }

  /* remote signKey spec */
  ki=GWEN_CryptToken_Context_GetVerifyKeyInfo(tctx);
  if (!ki) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No remote sign key");
    return AB_ERROR_NO_DATA;
  }
  else {
    ks=0;
    rv=AH_Medium__ReadKeySpec(m, GWEN_CryptToken_KeyInfo_GetKeyId(ki), &ks);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error reading keyspec (%d)", rv);
    }
    else {
      assert(ks);
      AH_MediumCtx_SetRemoteSignKeySpec(m->currentContext, ks);
      GWEN_KeySpec_free(ks);
    }
  }

  /* remote cryptKey spec */
  ki=GWEN_CryptToken_Context_GetEncryptKeyInfo(tctx);
  if (!ki) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No remote crypt key");
    return AB_ERROR_NO_DATA;
  }
  else {
    ks=0;
    rv=AH_Medium__ReadKeySpec(m, GWEN_CryptToken_KeyInfo_GetKeyId(ki), &ks);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error reading keyspec (%d)", rv);
    }
    else {
      assert(ks);
      AH_MediumCtx_SetRemoteCryptKeySpec(m->currentContext, ks);
      GWEN_KeySpec_free(ks);
    }
  }

  return 0;
}



int AH_Medium_GetTokenIdData(AH_MEDIUM *m, GWEN_BUFFER *buf) {
  int rv;

  assert(m);
  assert(buf);

  if (!AH_Medium_IsMounted(m)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Medium is not mounted");
    return -1;
  }

  rv=GWEN_CryptToken_GetTokenIdData(m->cryptToken, buf);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not get token id data (%d)", rv);
    return AB_ERROR_GENERIC;
  }

  return 0;
}



GWEN_CRYPTKEY *AH_Medium_GetLocalPubSignKey(AH_MEDIUM *m){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  GWEN_CRYPTKEY *key;
  int rv;

  assert(m);

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return 0;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);

  ki=GWEN_CryptToken_Context_GetSignKeyInfo(tctx);
  if (!ki) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No local sign key");
    return 0;
  }

  rv=GWEN_CryptToken_ReadKey(m->cryptToken,
                             GWEN_CryptToken_KeyInfo_GetKeyId(ki),
                             &key);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error reading key (%d)", rv);
    return 0;
  }
  assert(key);

  if (GWEN_CryptKey_GetStatus(key)!=GWEN_CRYPTTOKEN_KEYSTATUS_ACTIVE) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Key is not active (%d)",
             GWEN_CryptKey_GetStatus(key));
    GWEN_CryptKey_free(key);
    return 0;
  }

  return key;
}



GWEN_CRYPTKEY *AH_Medium_GetLocalPubCryptKey(AH_MEDIUM *m){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  GWEN_CRYPTKEY *key;
  int rv;

  assert(m);

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return 0;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);
  ki=GWEN_CryptToken_Context_GetDecryptKeyInfo(tctx);
  if (!ki) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No local crypt key");
    return 0;
  }

  rv=GWEN_CryptToken_ReadKey(m->cryptToken,
                             GWEN_CryptToken_KeyInfo_GetKeyId(ki),
                             &key);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error reading key (%d)", rv);
    return 0;
  }
  assert(key);
  if (GWEN_CryptKey_GetStatus(key)!=GWEN_CRYPTTOKEN_KEYSTATUS_ACTIVE) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Key is not active (%d)",
             GWEN_CryptKey_GetStatus(key));
    GWEN_CryptKey_free(key);
    return 0;
  }

  return key;
}



GWEN_CRYPTKEY *AH_Medium_GetPubSignKey(AH_MEDIUM *m){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  GWEN_CRYPTKEY *key;
  int rv;

  assert(m);

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return 0;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);
  ki=GWEN_CryptToken_Context_GetVerifyKeyInfo(tctx);
  if (!ki) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No remote sign key");
    return 0;
  }

  rv=GWEN_CryptToken_ReadKey(m->cryptToken,
                             GWEN_CryptToken_KeyInfo_GetKeyId(ki),
                             &key);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error reading key (%d)", rv);
    return 0;
  }
  assert(key);
  if (GWEN_CryptKey_GetStatus(key)!=GWEN_CRYPTTOKEN_KEYSTATUS_ACTIVE) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Key is not active (%d)",
             GWEN_CryptKey_GetStatus(key));
    GWEN_CryptKey_free(key);
    return 0;
  }

  return key;
}



GWEN_CRYPTKEY *AH_Medium_GetPubCryptKey(AH_MEDIUM *m){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  GWEN_CRYPTKEY *key;
  int rv;

  assert(m);

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return 0;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);
  ki=GWEN_CryptToken_Context_GetEncryptKeyInfo(tctx);
  if (!ki) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No remote crypt key");
    return 0;
  }

  rv=GWEN_CryptToken_ReadKey(m->cryptToken,
                             GWEN_CryptToken_KeyInfo_GetKeyId(ki),
                             &key);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error reading key (%d)", rv);
    return 0;
  }
  assert(key);

  if (GWEN_CryptKey_GetStatus(key)!=GWEN_CRYPTTOKEN_KEYSTATUS_ACTIVE) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Key is not active (%d)",
             GWEN_CryptKey_GetStatus(key));
    GWEN_CryptKey_free(key);
    return 0;
  }

  return key;
}



int AH_Medium_SetPubSignKey(AH_MEDIUM *m, const GWEN_CRYPTKEY *key){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  int rv;

  assert(m);
  assert(key);

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return AB_ERROR_INVALID;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);
  ki=GWEN_CryptToken_Context_GetVerifyKeyInfo(tctx);
  if (!ki) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No remote sign key");
    return AB_ERROR_NOT_FOUND;
  }

  rv=GWEN_CryptToken_WriteKey(m->cryptToken,
                              GWEN_CryptToken_KeyInfo_GetKeyId(ki),
                              key);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error writing key (%d)", rv);
    return rv;
  }

  AH_MediumCtx_SetRemoteSignKeySpec(m->currentContext,
                                    GWEN_CryptKey_GetKeySpec(key));

  return 0;
}



int AH_Medium_SetPubCryptKey(AH_MEDIUM *m, const GWEN_CRYPTKEY *key){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  int rv;

  assert(m);
  assert(key);

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return AB_ERROR_INVALID;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);
  ki=GWEN_CryptToken_Context_GetEncryptKeyInfo(tctx);
  if (!ki) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No remote crypt key");
    return AB_ERROR_NOT_FOUND;
  }

  rv=GWEN_CryptToken_WriteKey(m->cryptToken,
                              GWEN_CryptToken_KeyInfo_GetKeyId(ki),
                              key);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error writing key %x (%d)",
             GWEN_CryptToken_KeyInfo_GetKeyId(ki),
             rv);
    return rv;
  }

  AH_MediumCtx_SetRemoteCryptKeySpec(m->currentContext,
                                     GWEN_CryptKey_GetKeySpec(key));

  return 0;
}



int AH_Medium_ResetServerKeys(AH_MEDIUM *m){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  int rv;

  assert(m);

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return AB_ERROR_INVALID;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);

  /* sign key */
  ki=GWEN_CryptToken_Context_GetVerifyKeyInfo(tctx);
  if (ki) {
    rv=GWEN_CryptToken_WriteKey(m->cryptToken,
                                GWEN_CryptToken_KeyInfo_GetKeyId(ki),
                                0);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error resetting sign key (%d)", rv);
      return rv;
    }
  }
  AH_MediumCtx_SetRemoteSignKeySpec(m->currentContext, 0);

  /* crypt key */
  ki=GWEN_CryptToken_Context_GetEncryptKeyInfo(tctx);
  if (ki) {
    rv=GWEN_CryptToken_WriteKey(m->cryptToken,
                                GWEN_CryptToken_KeyInfo_GetKeyId(ki),
                                0);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error writing key (%d)", rv);
      return rv;
    }
  }
  AH_MediumCtx_SetRemoteCryptKeySpec(m->currentContext, 0);

  return 0;
}



int AH_Medium__ResetKey(AH_MEDIUM *m, int kid){
  GWEN_KEYSPEC *ks=0;
  int rv;

  /* get key */
  rv=GWEN_CryptToken_ReadKeySpec(m->cryptToken, kid, &ks);
  if (rv) {
    if (rv==GWEN_ERROR_NO_DATA) {
      DBG_ERROR(0, "No keyspec for key %d", kid);
      return rv;
    }
    else {
      DBG_ERROR(0, "Could not read key spec (%d)", rv);
      return rv;
    }
  }
  assert(ks);
  GWEN_KeySpec_SetStatus(ks, GWEN_CRYPTTOKEN_KEYSTATUS_FREE);
  rv=GWEN_CryptToken_WriteKeySpec(m->cryptToken, kid, ks);
  if (rv) {
    DBG_ERROR(0, "Could not write key spec (%d)", rv);
    GWEN_KeySpec_free(ks);
    return rv;
  }
  GWEN_KeySpec_free(ks);

  return 0;
}



int AH_Medium_ResetUserKeys(AH_MEDIUM *m){
  const GWEN_CRYPTTOKEN_CONTEXT *tctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  int rv;

  assert(m);

  if (m->selected==-1 || !m->currentContext) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context selected");
    return AB_ERROR_INVALID;
  }

  tctx=AH_MediumCtx_GetTokenContext(m->currentContext);
  assert(tctx);

  /* sign key */
  ki=GWEN_CryptToken_Context_GetSignKeyInfo(tctx);
  if (ki) {
    rv=AH_Medium__ResetKey(m, GWEN_CryptToken_KeyInfo_GetKeyId(ki));
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error resetting sign key (%d)", rv);
      return rv;
    }
  }
  AH_MediumCtx_SetLocalSignKeySpec(m->currentContext, 0);

  /* crypt key */
  ki=GWEN_CryptToken_Context_GetDecryptKeyInfo(tctx);
  if (ki) {
    rv=AH_Medium__ResetKey(m, GWEN_CryptToken_KeyInfo_GetKeyId(ki));
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error resetting crypt spec (%d)", rv);
      return rv;
    }
  }
  AH_MediumCtx_SetLocalCryptKeySpec(m->currentContext, 0);

  return 0;
}







