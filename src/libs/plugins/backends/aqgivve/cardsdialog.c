/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "cardsdialog_p.h"
#include <gwenhywfar/gui.h>
#include <aqbanking/banking_be.h>


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static int _dlgInit ( GWEN_DIALOG *dlg );
static int GWENHYWFAR_CB _dlgSignalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _dlgHandleActivated(GWEN_DIALOG *dlg, const char *sender);
static int _addAccount(GWEN_DIALOG *dlg);



/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(GWEN_DIALOG, AG_CARDS_DIALOG);



GWEN_DIALOG *AG_CardsDialog_new(AB_PROVIDER *pro, AB_USER *user, AG_VOUCHERLIST *card_list)
{
  AG_CARDS_DIALOG *xdlg;
  GWEN_DIALOG *dlg;
  dlg=GWEN_Dialog_CreateAndLoadWithPath("carddialog", AB_PM_LIBNAME, AB_PM_DATADIR,
                                        "aqbanking/backends/aqgivve/dialogs/dlg_cardselect.dlg");
  GWEN_Dialog_SetSignalHandler(dlg, _dlgSignalHandler);

  GWEN_NEW_OBJECT(AG_CARDS_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AG_CARDS_DIALOG, dlg, xdlg, _freeData);

  xdlg->cardlist = card_list;
  xdlg->provider = pro;
  xdlg->user = user;
  return dlg;
}



void _freeData(void *bp, void *p)
{
  AG_CARDS_DIALOG *xdlg;

  xdlg=(AG_CARDS_DIALOG*) p;
  GWEN_FREE_OBJECT(xdlg);
}



int _dlgInit(GWEN_DIALOG *dlg)
{
  GWEN_Dialog_SetCharProperty(dlg, "cardlistbox", GWEN_DialogProperty_Title, 0, "Cards", 0);

  AG_CARDS_DIALOG *xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AG_CARDS_DIALOG, dlg);


  for (int n = 0; n < AG_VOUCHERLIST_Get_TotalEntries(xdlg->cardlist); n++) {

    AG_VOUCHER *c = AG_VOUCHERLIST_Get_Card_By_Index(xdlg->cardlist, n);
    if (c) {
      const char *id = AG_VOUCHER_GetID(c);
      GWEN_Dialog_SetCharProperty(dlg, "cardlistbox", GWEN_DialogProperty_AddValue, 0, id, 0);
    }
  }

  return GWEN_DialogEvent_ResultHandled;
}



int _dlgSignalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  switch (t) {
  case  GWEN_DialogEvent_TypeInit :
    return _dlgInit(dlg);
  case GWEN_DialogEvent_TypeActivated:
    return _dlgHandleActivated(dlg, sender);
  default:
    break;
  }

  return GWEN_DialogEvent_ResultHandled;

}



int _dlgHandleActivated(GWEN_DIALOG *dlg, const char *sender)
{
  if (strcasecmp(sender, "add_button") == 0) {
    return _addAccount(dlg);
  }
  else if (strcasecmp(sender, "close_button") == 0) {
    return GWEN_DialogEvent_ResultAccept;

  }
  return GWEN_DialogEvent_ResultHandled;
}



int _addAccount(GWEN_DIALOG *dlg)
{
  int index=GWEN_Dialog_GetIntProperty(dlg, "cardlistbox", GWEN_DialogProperty_Value, 0, -1);
  if (index >= 0) {
    const char *id = GWEN_Dialog_GetCharProperty(dlg, "cardlistbox", GWEN_DialogProperty_Value, index, NULL);
    DBG_INFO(AQGIVVE_LOGDOMAIN, "card selected: %s", id);

    AG_CARDS_DIALOG *xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AG_CARDS_DIALOG, dlg);
    if (!xdlg) {
      return GWEN_DialogEvent_ResultNotHandled;
    }
    else {
      const AG_VOUCHER *selected_card = AG_VOUCHERLIST_Get_Card_By_ID(xdlg->cardlist, id);
      if (!selected_card) {
        return GWEN_DialogEvent_ResultNotHandled;
      }
      else {
        const AG_VOUCHEROWNER *owner = AG_VOUCHER_GetOwner(selected_card);
        char *owner_name;

        if (owner) {
          owner_name = strdup(AG_VOUCHEROWNER_GetName(owner));
        }
        else {
          owner_name = strdup(AB_User_GetUserName(xdlg->user));
        }

        DBG_INFO(AQGIVVE_LOGDOMAIN, "creating account with card id %s and owner %s", id, owner_name);

        AB_ACCOUNT *account;

        int account_name_len = strlen(owner_name) + 12;

        char account_name[account_name_len];

        snprintf(account_name, account_name_len, "GivveCard %s", owner_name);

        account = AB_Provider_CreateAccountObject(xdlg->provider);
        AB_Provider_AddAccount(xdlg->provider, account, 1);
        AB_Account_SetUserId(account, AB_User_GetUniqueId(xdlg->user));
        AB_Account_SetOwnerName(account, owner_name);
        AB_Account_SetAccountNumber(account, id);
        AB_Account_SetAccountName(account, account_name);

        AB_Provider_WriteAccount(xdlg->provider, AB_Account_GetUniqueId(account), 1, 1, account);
      }

    }

  }
  return GWEN_DialogEvent_ResultHandled;
}



