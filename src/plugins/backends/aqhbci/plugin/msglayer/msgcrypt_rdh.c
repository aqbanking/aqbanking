/***************************************************************************
    begin       : Tue Nov 25 2008
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



int AH_Msg_SignRdh(AH_MSG *hmsg,
		   GWEN_BUFFER *rawBuf,
		   const char *signer) {
  AB_USER *su;
  int rv;

  assert(hmsg);
  su=AB_Banking_FindUser(AH_HBCI_GetBankingApi(AH_Dialog_GetHbci(hmsg->dialog)),
			 AH_PROVIDER_NAME,
			 "de", "*",
			 signer, "*");
  if (!su) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "Unknown user \"%s\"",
	      signer);
    return GWEN_ERROR_NOT_FOUND;
  }

  switch(AH_User_GetRdhType(su)) {
  case 0:
  case 1:
    rv=AH_Msg_SignRdh1(hmsg, su, rawBuf, signer);
    break;
  case 2:
    rv=AH_Msg_SignRdh2(hmsg, su, rawBuf, signer);
    break;
  case 3:
    rv=AH_Msg_SignRdh3(hmsg, su, rawBuf, signer);
    break;
  case 5:
    rv=AH_Msg_SignRdh5(hmsg, su, rawBuf, signer);
    break;
  case 10:
    rv=AH_Msg_SignRdh10(hmsg, su, rawBuf, signer);
    break;
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "RDH %d not supported", AH_User_GetRdhType(su));
    rv=GWEN_ERROR_INVALID;
  }

  return rv;
}



int AH_Msg_EncryptRdh(AH_MSG *hmsg) {
  AB_USER *u;
  int rv;

  assert(hmsg);
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);

  switch(AH_User_GetRdhType(u)) {
  case 0:
  case 1:
    rv=AH_Msg_EncryptRdh1(hmsg);
    break;
  case 2:
    rv=AH_Msg_EncryptRdh2(hmsg);
    break;
  case 3:
    rv=AH_Msg_EncryptRdh3(hmsg);
    break;
  case 5:
    rv=AH_Msg_EncryptRdh5(hmsg);
    break;
  case 10:
    rv=AH_Msg_EncryptRdh10(hmsg);
    break;
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "RDH %d not supported", AH_User_GetRdhType(u));
    rv=GWEN_ERROR_INVALID;
  }

  return rv;
}




int AH_Msg_DecryptRdh(AH_MSG *hmsg, GWEN_DB_NODE *gr){
  AB_USER *u;
  int rv;

  assert(hmsg);
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);

  switch(AH_User_GetRdhType(u)) {
  case 0:
  case 1:
    rv=AH_Msg_DecryptRdh1(hmsg, gr);
    break;
  case 2:
    rv=AH_Msg_DecryptRdh2(hmsg, gr);
    break;
  case 3:
    rv=AH_Msg_DecryptRdh3(hmsg, gr);
    break;
  case 5:
    rv=AH_Msg_DecryptRdh5(hmsg, gr);
    break;
  case 10:
    rv=AH_Msg_DecryptRdh10(hmsg, gr);
    break;
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "RDH %d not supported", AH_User_GetRdhType(u));
    rv=GWEN_ERROR_INVALID;
  }

  return rv;
}



int AH_Msg_VerifyRdh(AH_MSG *hmsg, GWEN_DB_NODE *gr) {
  AB_USER *u;
  int rv;

  assert(hmsg);
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);

  switch(AH_User_GetRdhType(u)) {
  case 0:
  case 1:
    rv=AH_Msg_VerifyRdh1(hmsg, gr);
    break;
  case 2:
    rv=AH_Msg_VerifyRdh2(hmsg, gr);
    break;
  case 3:
    rv=AH_Msg_VerifyRdh3(hmsg, gr);
    break;
  case 5:
    rv=AH_Msg_VerifyRdh5(hmsg, gr);
    break;
  case 10:
    rv=AH_Msg_VerifyRdh10(hmsg, gr);
    break;
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "RDH %d not supported", AH_User_GetRdhType(u));
    rv=GWEN_ERROR_INVALID;
  }

  return rv;
}
















