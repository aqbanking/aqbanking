/***************************************************************************
    begin       : Tue Jun 03 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/*
 * This file is included by provider.c
 */




GWEN_DIALOG *AH_Provider_GetNewCardUserDialog(AB_PROVIDER *pro) {
  int rv;
  GWEN_BUFFER *mtypeName;
  GWEN_BUFFER *mediumName;
  GWEN_CRYPT_TOKEN *ct;

  mtypeName=GWEN_Buffer_new(0, 64, 0, 1);
  mediumName=GWEN_Buffer_new(0, 64, 0, 1);

  rv=AB_Banking_CheckCryptToken(AB_Provider_GetBanking(pro),
				GWEN_Crypt_Token_Device_Card,
				mtypeName,
				mediumName);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Gui_ShowError(I18N("Chipcard Error"),
		       I18N("Error checking chip card (%d).\n"
			    "Maybe libchipcard or its plugins are not installed?"));
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(mtypeName);
    return NULL;
  }

  rv=AB_Banking_GetCryptToken(AB_Provider_GetBanking(pro),
			      GWEN_Buffer_GetStart(mtypeName),
			      GWEN_Buffer_GetStart(mediumName),
			      &ct);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(mtypeName);
    return NULL;
  }

  if (strcasecmp(GWEN_Buffer_GetStart(mtypeName), "ddvcard")==0) {
    GWEN_DIALOG *dlg2;

    DBG_WARN(0, "DDV card");
    dlg2=AH_DdvCardDialog_new(AB_Provider_GetBanking(pro), ct);
    if (dlg2==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
      GWEN_Buffer_free(mediumName);
      GWEN_Buffer_free(mtypeName);
      return NULL;
    }

    GWEN_Dialog_SetWidgetText(dlg2, "", I18N("Create HBCI/FinTS DDV User"));
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(mtypeName);
    return dlg2;
  }
  else if (strcasecmp(GWEN_Buffer_GetStart(mtypeName), "starcoscard")==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "STARCOS RSA card currently not supported by this dialog");
    // TODO
  }
  else if (strcasecmp(GWEN_Buffer_GetStart(mtypeName), "zkacard")==0) {
    GWEN_DIALOG *dlg2;

    DBG_WARN(0, "ZKA RSA card");
    dlg2=AH_ZkaCardDialog_new(AB_Provider_GetBanking(pro), ct);
    if (dlg2==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
      GWEN_Buffer_free(mediumName);
      GWEN_Buffer_free(mtypeName);
      return NULL;
    }

    GWEN_Dialog_SetWidgetText(dlg2, "", I18N("Create HBCI/FinTS ZKA RSA User"));
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(mtypeName);
    return dlg2;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Card type \"%s\" not yet supported",
	      GWEN_Buffer_GetStart(mtypeName));
  }
  GWEN_Buffer_free(mediumName);
  GWEN_Buffer_free(mtypeName);
  AB_Banking_ClearCryptTokenList(AB_Provider_GetBanking(pro));

  return NULL;
}



GWEN_DIALOG *AH_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u) {
  AH_PROVIDER *hp;
  GWEN_DIALOG *dlg;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  switch(AH_User_GetCryptMode(u)) {
  case AH_CryptMode_Pintan:
    dlg=AH_EditUserPinTanDialog_new(AB_Provider_GetBanking(pro), u, 1);
    break;
  case AH_CryptMode_Ddv:
    dlg=AH_EditUserDdvDialog_new(AB_Provider_GetBanking(pro), u, 1);
    break;
  case AH_CryptMode_Rdh:
    dlg=AH_EditUserRdhDialog_new(AB_Provider_GetBanking(pro), u, 1);
    break;
  default:
    dlg=NULL;
    break;
  }

  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



GWEN_DIALOG *AH_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i) {
  AH_PROVIDER *hp;
  GWEN_DIALOG *dlg;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Get user dialog %d", i);

  switch(i) {
  case AqHBCI_NewUserDialog_CodeExistingPinTan:
    dlg=AH_PinTanDialog_new(AB_Provider_GetBanking(pro));
    break;

  case AqHBCI_NewUserDialog_CodeExistingChipcard:
    dlg=AH_Provider_GetNewCardUserDialog(pro);
    break;

  case AqHBCI_NewUserDialog_CodeCreateKeyFile:
    dlg=AH_NewKeyFileDialog_new(AB_Provider_GetBanking(pro));
    break;

  case AqHBCI_NewUserDialog_CodeExistingKeyFile:
    dlg=AH_ImportKeyFileDialog_new(AB_Provider_GetBanking(pro));
    break;
  case AqHBCI_NewUserDialog_CodeCreateChipcard:

  case AqHBCI_NewUserDialog_CodeGeneric:
  default:
    dlg=AH_NewUserDialog_new(AB_Provider_GetBanking(pro));
    break;
  }

  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



GWEN_DIALOG *AH_Provider_GetEditAccountDialog(AB_PROVIDER *pro, AB_ACCOUNT *a) {
  AH_PROVIDER *hp;
  GWEN_DIALOG *dlg;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  dlg=AH_EditAccountDialog_new(AB_Provider_GetBanking(pro), a, 1);
  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



GWEN_DIALOG *AH_Provider_GetUserTypeDialog(AB_PROVIDER *pro) {
  AH_PROVIDER *hp;
  GWEN_DIALOG *dlg;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  dlg=AH_ChooseUserTypeDialog_new(AB_Provider_GetBanking(pro));
  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



