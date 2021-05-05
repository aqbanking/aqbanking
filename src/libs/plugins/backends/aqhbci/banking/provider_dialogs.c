/***************************************************************************
    begin       : Tue Jun 03 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_dialogs.h"
#include "aqhbci/banking/provider.h"
#include "aqhbci/banking/user.h"


#include "aqhbci/dialogs/dlg_edituserpintan_l.h"
#include "aqhbci/dialogs/dlg_edituserddv_l.h"
#include "aqhbci/dialogs/dlg_edituserrdh_l.h"
#include "aqhbci/dialogs/dlg_editaccount_l.h"
#include "aqhbci/dialogs/dlg_newuser_l.h"
#include "aqhbci/dialogs/dlg_ddvcard_l.h"
#include "aqhbci/dialogs/dlg_zkacard_l.h"
#include "aqhbci/dialogs/dlg_pintan_l.h"
#include "aqhbci/dialogs/dlg_newkeyfile_l.h"
#include "aqhbci/dialogs/dlg_importkeyfile_l.h"
#include "aqhbci/dialogs/dlg_choose_usertype_l.h"

#include <aqbanking/banking_be.h>
#include <aqbanking/i18n_l.h>

#include <gwenhywfar/gui.h>




GWEN_DIALOG *AH_Provider_GetNewCardUserDialog(AB_PROVIDER *pro)
{
  int rv;
  GWEN_BUFFER *mtypeName;
  GWEN_BUFFER *mediumName;
  GWEN_CRYPT_TOKEN *ct;
  AB_BANKING *ab;

  assert(pro);
  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  mtypeName=GWEN_Buffer_new(0, 64, 0, 1);
  mediumName=GWEN_Buffer_new(0, 64, 0, 1);

  rv=AB_Banking_CheckCryptToken(ab,
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
    dlg2=AH_DdvCardDialog_new(pro, ct);
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
    dlg2=AH_ZkaCardDialog_new(pro, ct);
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



GWEN_DIALOG *AH_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u)
{
  GWEN_DIALOG *dlg;

  DBG_ERROR(AQBANKING_LOGDOMAIN, "GetEditUserDialog");

  assert(pro);

  switch (AH_User_GetCryptMode(u)) {
  case AH_CryptMode_Pintan:
    dlg=AH_EditUserPinTanDialog_new(pro, u, 1);
    break;
  case AH_CryptMode_Ddv:
    dlg=AH_EditUserDdvDialog_new(pro, u, 1);
    break;
  case AH_CryptMode_Rdh:
    dlg=AH_EditUserRdhDialog_new(pro, u, 1);
    break;
  case AH_CryptMode_Rah:
    dlg=AH_EditUserRdhDialog_new(pro, u, 1);
    break;
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown crypt mode %d)", AH_User_GetCryptMode(u));
    dlg=NULL;
    break;
  }

  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



GWEN_DIALOG *AH_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i)
{
  GWEN_DIALOG *dlg;

  assert(pro);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Get user dialog %d", i);

  switch (i) {
  case AqHBCI_NewUserDialog_CodeExistingPinTan:
    dlg=AH_PinTanDialog_new(pro);
    break;

  case AqHBCI_NewUserDialog_CodeExistingChipcard:
    dlg=AH_Provider_GetNewCardUserDialog(pro);
    break;

  case AqHBCI_NewUserDialog_CodeCreateKeyFile:
    dlg=AH_NewKeyFileDialog_new(pro);
    break;

  case AqHBCI_NewUserDialog_CodeExistingKeyFile:
    dlg=AH_ImportKeyFileDialog_new(pro);
    break;
  case AqHBCI_NewUserDialog_CodeCreateChipcard:

  case AqHBCI_NewUserDialog_CodeGeneric:
  default:
    dlg=AH_NewUserDialog_new(pro);
    break;
  }

  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



GWEN_DIALOG *AH_Provider_GetEditAccountDialog(AB_PROVIDER *pro, AB_ACCOUNT *a)
{
  GWEN_DIALOG *dlg;

  assert(pro);

  dlg=AH_EditAccountDialog_new(pro, a, 1);
  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



GWEN_DIALOG *AH_Provider_GetUserTypeDialog(AB_PROVIDER *pro)
{
  GWEN_DIALOG *dlg;

  DBG_INFO(AQHBCI_LOGDOMAIN, "AH_Provider_GetUserTypeDialog called");

  assert(pro);

  dlg=AH_ChooseUserTypeDialog_new(pro);
  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



