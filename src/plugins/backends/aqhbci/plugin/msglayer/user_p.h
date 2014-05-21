/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_USER_P_H
#define AH_USER_P_H

#include "user_l.h"

#include <gwenhywfar/buffer.h>

#define AH_USER_MAX_TANMETHODS 16

#define AH_USER_MAX_TRANSFERS_PER_JOB  64
#define AH_USER_MAX_DEBITNOTES_PER_JOB 64


typedef struct AH_USER AH_USER;
struct AH_USER {
  AH_HBCI *hbci;
  GWEN_MSGENGINE *msgEngine;

  AH_CRYPT_MODE cryptMode;
  AH_USER_STATUS status;

  int hbciVersion;

  GWEN_URL *serverUrl;
  AH_BPD *bpd;
  GWEN_DB_NODE *dbUpd;

  char *peerId;
  char *systemId;

  int updVersion;

  int httpVMajor;
  int httpVMinor;
  char *httpUserAgent;
  char *httpContentType;

  uint32_t flags;
  uint32_t tanMethods;

  char *tokenType;
  char *tokenName;
  uint32_t tokenContextId;
  int rdhType;

  int tanMethodList[AH_USER_MAX_TANMETHODS+1];
  int tanMethodCount;

  int selectedTanMethod;

  char *prompt;

  AH_TAN_METHOD_LIST *tanMethodDescriptions;
  char *tanMediumId;
  GWEN_STRINGLIST *sepaDescriptors;
  char *sepaTransferProfile;
  char *sepaDebitNoteProfile;

  int maxTransfersPerJob;
  int maxDebitNotesPerJob;
};

static void GWENHYWFAR_CB AH_User_freeData(void *bp, void *p);

static void AH_User_ReadDb(AB_USER *u, GWEN_DB_NODE *db);
static void AH_User_toDb(AB_USER *u, GWEN_DB_NODE *db);

static void AH_User_LoadTanMethods(AB_USER *u);
static void AH_User_LoadSepaDescriptors(AB_USER *u);


static int AH_User_AddTextWithoutTags(const char *s, GWEN_BUFFER *obuf);


#endif /* AH_USER_P_H */


