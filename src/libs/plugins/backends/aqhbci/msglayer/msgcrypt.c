/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "msgcrypt.h"
#include "msgcrypt_rxh_common.h"
#include "msgcrypt_rxh_decrypt.h"
#include "msgcrypt_rxh_encrypt.h"
#include "msgcrypt_pintan.h"
#include "msgcrypt_pintan_verify.h"





int AH_Msg__Sign(AH_MSG *hmsg,
                 GWEN_BUFFER *rawBuf,
                 const char *signer)
{
  AB_USER *u;

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(u);
  switch (AH_User_GetCryptMode(u)) {
  case AH_CryptMode_Ddv:
    return AH_Msg_SignDdv(hmsg, rawBuf, signer);
  case AH_CryptMode_Rdh:
  case AH_CryptMode_Rah:
    return AH_Msg_SignRxh(hmsg, rawBuf, signer);
  case AH_CryptMode_Pintan:
    return AH_Msg_SignPinTan(hmsg, rawBuf, signer);
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "CryptMode %d not supported",
              AH_User_GetCryptMode(u));
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int AH_Msg__Encrypt(AH_MSG *hmsg)
{
  AB_USER *u;

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(u);
  switch (AH_User_GetCryptMode(u)) {
  case AH_CryptMode_Ddv:
    return AH_Msg_EncryptDdv(hmsg);
  case AH_CryptMode_Rdh:
  case AH_CryptMode_Rah:
    return AH_Msg_EncryptRxh(hmsg);
  case AH_CryptMode_Pintan:
    return AH_Msg_EncryptPinTan(hmsg);
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "CryptMode %d not supported",
              AH_User_GetCryptMode(u));
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int AH_Msg__Decrypt(AH_MSG *hmsg, GWEN_DB_NODE *gr)
{
  AB_USER *u;

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(u);
  switch (AH_User_GetCryptMode(u)) {
  case AH_CryptMode_Ddv:
    return AH_Msg_DecryptDdv(hmsg, gr);
  case AH_CryptMode_Rdh:
  case AH_CryptMode_Rah:
    return AH_Msg_DecryptRxh(hmsg, gr);
  case AH_CryptMode_Pintan:
    return AH_Msg_DecryptPinTan(hmsg, gr);
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "CryptMode %d not supported",
              AH_User_GetCryptMode(u));
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int AH_Msg__Verify(AH_MSG *hmsg, GWEN_DB_NODE *gr)
{
  AB_USER *u;

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(u);
  switch (AH_User_GetCryptMode(u)) {
  case AH_CryptMode_Ddv:
    return AH_Msg_VerifyDdv(hmsg, gr);
  case AH_CryptMode_Rdh:
  case AH_CryptMode_Rah:
    return AH_Msg_VerifyRxh(hmsg, gr);
  case AH_CryptMode_Pintan:
    return AH_Msg_VerifyPinTan(hmsg, gr);
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "CryptMode %d not supported",
              AH_User_GetCryptMode(u));
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}





/* helper functions */


int AH_Msg_SampleSignHeadsAndTailsFromDecodedMsg(GWEN_DB_NODE *gr, GWEN_LIST *sigheads, GWEN_LIST *sigtails)
{
  GWEN_DB_NODE *n;

  n=GWEN_DB_GetFirstGroup(gr);
  if (n && strcasecmp(GWEN_DB_GroupName(n), "MsgHead")==0)
    n=GWEN_DB_GetNextGroup(n);                             /* skip MsgHead, if any */
  while (n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigHead")==0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Adding signature head");
      GWEN_List_PushBack(sigheads, n);
    }
    else
      break;
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (n==NULL) {
    if (GWEN_List_GetSize(sigheads)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Found signature heads but no other segments");
      return GWEN_ERROR_BAD_DATA;
    }
    else {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "No signatures");
      return 0;
    }
  }

  /* find first signature tail */
  while(n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigTail")==0)
      break;
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Have signature heads but no signature tails");
    return GWEN_ERROR_BAD_DATA;
  }

  /* sample signature tails */
  while (n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigTail")!=0)
      break;
    GWEN_List_PushBack(sigtails, n);
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Message tail expected");
    return GWEN_ERROR_BAD_DATA;
  }

  if (strcasecmp(GWEN_DB_GroupName(n), "MsgTail")!=0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unexpected segment (msg tail expected)");
    return GWEN_ERROR_BAD_DATA;
  }

  n=GWEN_DB_GetNextGroup(n);
  if (n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unexpected segment (end expected)");
    return GWEN_ERROR_BAD_DATA;
  }

  if (GWEN_List_GetSize(sigheads)!=GWEN_List_GetSize(sigtails)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Number of signature heads (%d) does not match number of signature tails (%d)",
              GWEN_List_GetSize(sigheads),
              GWEN_List_GetSize(sigtails));
    return GWEN_ERROR_BAD_DATA;
  }

  return 0;
}



int AH_Msg_GetStartPosOfSignedData(const GWEN_LIST *sigheads)
{
  GWEN_DB_NODE *n;

  n=(GWEN_DB_NODE *)GWEN_List_GetBack(sigheads); /* look behind last signature head */
  if (n) {
    int segBegin;
    int segLen;

    segBegin=GWEN_DB_GetIntValue(n, "segment/pos", 0, 0);
    if (!segBegin) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No position specification in segment");
      return GWEN_ERROR_BAD_DATA;
    }

    segLen=GWEN_DB_GetIntValue(n, "segment/length", 0, 0);
    if (!segLen) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No length specification in segment");
      return GWEN_ERROR_BAD_DATA;
    }

    return segBegin+segLen;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No signature head");
    return GWEN_ERROR_INVALID;
  }
}



int AH_Msg_GetFirstPosBehindSignedData(const GWEN_LIST *sigtails)
{
  GWEN_DB_NODE *n;

  n=(GWEN_DB_NODE *)GWEN_List_GetFront(sigtails);
  if (n) {
    int segBegin;
    int segLen;

    segBegin=GWEN_DB_GetIntValue(n, "segment/pos", 0, 0);
    if (!segBegin) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No position specification in segment");
      return GWEN_ERROR_BAD_DATA;
    }

    segLen=GWEN_DB_GetIntValue(n, "segment/length", 0, 0);
    if (!segLen) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No length specification in segment");
      return GWEN_ERROR_BAD_DATA;
    }

    return segBegin;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No signature head");
    return GWEN_ERROR_INVALID;
  }
}





