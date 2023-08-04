/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "msgcrypt_rxh_decrypt.h"
#include "msgcrypt_rxh_encrypt.h"



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










