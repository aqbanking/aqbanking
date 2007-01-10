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


#ifndef CHIPCARD_CT_DDV_P_H
#define CHIPCARD_CT_DDV_P_H


#include <gwenhywfar/crypttoken.h>


typedef struct AH_CT_PINTAN AH_CT_PINTAN;

struct AH_CT_PINTAN {
  GWEN_PLUGIN_MANAGER *pluginManager;
  unsigned int localSignSeq;
};

static GWEN_CRYPTTOKEN *AH_CryptTokenPinTan_new(GWEN_PLUGIN_MANAGER *pm,
                                                const char *name);

static void GWENHYWFAR_CB AH_CryptTokenPinTan_FreeData(void *bp, void *p);


static int AH_CryptTokenPinTan_Open(GWEN_CRYPTTOKEN *ct, int manage);
static int AH_CryptTokenPinTan_Create(GWEN_CRYPTTOKEN *ct);
static int AH_CryptTokenPinTan_Close(GWEN_CRYPTTOKEN *ct);


static int AH_CryptTokenPinTan_GetSignSeq(GWEN_CRYPTTOKEN *ct,
                                          GWEN_TYPE_UINT32 kid,
                                          GWEN_TYPE_UINT32 *signSeq);

static int AH_CryptTokenPinTan_ReadKeySpec(GWEN_CRYPTTOKEN *ct,
                                           GWEN_TYPE_UINT32 kid,
                                           GWEN_KEYSPEC **ks);

static int AH_CryptTokenPinTan_FillUserList(GWEN_CRYPTTOKEN *ct,
                                            GWEN_CRYPTTOKEN_USER_LIST *ul);




static GWEN_CRYPTTOKEN*
  AH_CryptTokenPinTan_Plugin_CreateToken(GWEN_PLUGIN *pl,
                                         const char *subTypeName,
                                         const char *name);

static GWEN_PLUGIN*
  AH_CryptTokenPinTan_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
                                 const char *modName,
                                 const char *fileName);

static GWEN_CRYPTTOKEN*
  AH_CryptTokenPinTan_Plugin_CreateToken(GWEN_PLUGIN *pl,
                                         const char *subTypeName,
                                         const char *name);




#endif

