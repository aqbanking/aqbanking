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

#include "pintan_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>


GWEN_INHERIT(GWEN_CRYPTTOKEN, AH_CT_PINTAN)



GWEN_PLUGIN *crypttoken_pintan_factory(GWEN_PLUGIN_MANAGER *pm,
                                       const char *modName,
                                       const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AH_CryptTokenPinTan_Plugin_new(pm, modName, fileName);
  assert(pl);

  return pl;
}



GWEN_PLUGIN *AH_CryptTokenPinTan_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
                                            const char *modName,
                                            const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=GWEN_CryptToken_Plugin_new(pm,
                                GWEN_CryptToken_Device_None,
                                modName,
                                fileName);

  /* set virtual functions */
  GWEN_CryptToken_Plugin_SetCreateTokenFn(pl,
                                          AH_CryptTokenPinTan_Plugin_CreateToken);

  return pl;
}



GWEN_CRYPTTOKEN *AH_CryptTokenPinTan_Plugin_CreateToken(GWEN_PLUGIN *pl,
                                                        const char *subTypeName,
                                                        const char *name) {
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_CRYPTTOKEN *ct;

  assert(pl);

  pm=GWEN_Plugin_GetManager(pl);
  assert(pm);

  ct=AH_CryptTokenPinTan_new(pm, name);
  assert(ct);

  return ct;
}



int AH_CryptTokenPinTan_Plugin_CheckToken(GWEN_PLUGIN *pl,
                                          GWEN_BUFFER *subTypeName,
                                          GWEN_BUFFER *name) {

  return GWEN_ERROR_IO;
}









GWEN_CRYPTTOKEN *AH_CryptTokenPinTan_new(GWEN_PLUGIN_MANAGER *pm,
                                         const char *name) {
  AH_CT_PINTAN *lct;
  GWEN_CRYPTTOKEN *ct;

  DBG_DEBUG(0, "Creating crypttoken (PinTan)");

  /* create crypt token */
  ct=GWEN_CryptToken_new(pm,
                         GWEN_CryptToken_Device_Card,
                         "pintan", 0, name);

  /* inherit CryptToken: Set our own data */
  GWEN_NEW_OBJECT(AH_CT_PINTAN, lct);
  GWEN_INHERIT_SETDATA(GWEN_CRYPTTOKEN, AH_CT_PINTAN, ct, lct,
                       AH_CryptTokenPinTan_FreeData);
  lct->pluginManager=pm;

  /* set virtual functions */
  GWEN_CryptToken_SetOpenFn(ct, AH_CryptTokenPinTan_Open);
  GWEN_CryptToken_SetCreateFn(ct, AH_CryptTokenPinTan_Create);
  GWEN_CryptToken_SetCloseFn(ct, AH_CryptTokenPinTan_Close);
  GWEN_CryptToken_SetGetSignSeqFn(ct, AH_CryptTokenPinTan_GetSignSeq);
  GWEN_CryptToken_SetReadKeySpecFn(ct, AH_CryptTokenPinTan_ReadKeySpec);
  GWEN_CryptToken_SetFillUserListFn(ct, AH_CryptTokenPinTan_FillUserList);
  return ct;
}



void GWENHYWFAR_CB AH_CryptTokenPinTan_FreeData(void *bp, void *p) {
  AH_CT_PINTAN *lct;

  lct=(AH_CT_PINTAN*)p;
  GWEN_FREE_OBJECT(lct);
}



int AH_CryptTokenPinTan_Open(GWEN_CRYPTTOKEN *ct, int manage) {
  AH_CT_PINTAN *lct;
  int rv;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN_DESCRIPTION *pd;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, AH_CT_PINTAN, ct);
  assert(lct);

  pm=GWEN_CryptToken_GetCryptManager(ct);
  assert(pm);
  pd=GWEN_PluginManager_GetPluginDescr(pm, GWEN_CryptToken_GetTokenType(ct));
  if (pd) {
    GWEN_XMLNODE *n;
    GWEN_XMLNODE *ctNode=0;

    n=GWEN_PluginDescription_GetXmlNode(pd);
    assert(n);
    n=GWEN_XMLNode_FindFirstTag(n, "crypttokens", 0, 0);
    if (n) {
      ctNode=GWEN_XMLNode_FindFirstTag(n, "crypttoken", 0, 0);
    }
    if (ctNode) {
      rv=GWEN_CryptToken_ReadXml(ct, ctNode);
      if (rv) {
	DBG_ERROR(GWEN_LOGDOMAIN,
		  "Error reading CryptToken data from XML (%d)",
		  rv);
	GWEN_PluginDescription_free(pd);
	return rv;
      }
      GWEN_PluginDescription_free(pd);
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Plugin description for crypt token type \"%s\" does "
		"not contain \"crypttoken\" element.",
		GWEN_CryptToken_GetTokenType(ct));
      return GWEN_ERROR_INVALID;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Could not find plugin description for crypt token type \"%s\"",
	      GWEN_CryptToken_GetTokenType(ct));
    return GWEN_ERROR_INVALID;
  }

  return 0;
}



int AH_CryptTokenPinTan_Create(GWEN_CRYPTTOKEN *ct) {
  int rv;

  rv=AH_CryptTokenPinTan_Open(ct, 0);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AH_CryptTokenPinTan_Close(GWEN_CRYPTTOKEN *ct) {
  AH_CT_PINTAN *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, AH_CT_PINTAN, ct);
  assert(lct);

  return 0;
}




int AH_CryptTokenPinTan_GetSignSeq(GWEN_CRYPTTOKEN *ct,
                                   uint32_t kid,
                                   uint32_t *signSeq) {
  AH_CT_PINTAN *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, AH_CT_PINTAN, ct);
  assert(lct);

  *signSeq=++(lct->localSignSeq);

  return 0;
}



int AH_CryptTokenPinTan_ReadKeySpec(GWEN_CRYPTTOKEN *ct,
                                    uint32_t kid,
                                    GWEN_KEYSPEC **pks) {
  AH_CT_PINTAN *lct;
  GWEN_KEYSPEC *ks;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, AH_CT_PINTAN, ct);
  assert(lct);

  /* create a dummy keyspec */
  ks=GWEN_KeySpec_new();
  GWEN_KeySpec_SetStatus(ks, GWEN_CRYPTTOKEN_KEYSTATUS_ACTIVE);
  GWEN_KeySpec_SetKeyType(ks, "RSA");
  GWEN_KeySpec_SetNumber(ks, 1);
  GWEN_KeySpec_SetVersion(ks, 1);

  *pks=ks;

  return 0;
}



int AH_CryptTokenPinTan_FillUserList(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_USER_LIST *ul) {
  AH_CT_PINTAN *lct;
  GWEN_CRYPTTOKEN_USER *u;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, AH_CT_PINTAN, ct);
  assert(lct);

  u=GWEN_CryptToken_User_new();
  GWEN_CryptToken_User_SetId(u, 1);
  GWEN_CryptToken_User_SetContextId(u, 1);
  GWEN_CryptToken_User_List_Add(u, ul);

  return 0;
}






