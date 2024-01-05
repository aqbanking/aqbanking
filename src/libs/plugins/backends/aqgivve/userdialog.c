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

#include "userdialog_p.h"
#include <aqbanking/banking_be.h>
#include <gwenhywfar/gui.h>
#include "cardsdialog.h"
#include "provider_request.h"
#include "voucher.h"
#include "voucherlist.h"



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static int GWENHYWFAR_CB _dlgSignalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);
static int _dlgHandleActivated(GWEN_DIALOG *dlg, const char *sender);
static int _addUser(GWEN_DIALOG *dlg);



/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(GWEN_DIALOG, AG_USER_DIALOG);



void _freeData(void *bp, void *p)
{
}



GWEN_DIALOG *AG_GetNewUserDialog(AB_PROVIDER *pro, int i)
{

  GWEN_DIALOG *dlg;
  AG_USER_DIALOG *xdlg;

  dlg=GWEN_Dialog_CreateAndLoadWithPath("ag_new_user", AB_PM_LIBNAME, AB_PM_DATADIR,
                                        "aqbanking/backends/aqgivve/dialogs/dlg_edituser.dlg");
  GWEN_Dialog_SetSignalHandler(dlg, _dlgSignalHandler);

  GWEN_NEW_OBJECT(AG_USER_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AG_USER_DIALOG, dlg, xdlg, _freeData);

  xdlg->provider = pro;
  xdlg->user = NULL;
  /* done */
  return dlg;
}



GWEN_DIALOG *AG_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u)
{
  GWEN_DIALOG *dlg = AG_GetNewUserDialog(pro, 0);
  AG_USER_DIALOG *xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AG_USER_DIALOG, dlg);
  if (xdlg) {
    xdlg->user = u;
  }

  return dlg;
}



int AG_Provider_EditUserDialog_init(GWEN_DIALOG *dlg)
{

  AG_USER_DIALOG *xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AG_USER_DIALOG, dlg);
  if (xdlg) {

    if (xdlg->user) {
      GWEN_Dialog_SetCharProperty(dlg, "username_edit", GWEN_DialogProperty_Value, 0, AB_User_GetUserName(xdlg->user), 0);
      GWEN_Dialog_SetCharProperty(dlg, "userid_edit", GWEN_DialogProperty_Value, 0, AB_User_GetUserId(xdlg->user), 0);
    }
  }
  return GWEN_DialogEvent_ResultHandled;
}



int _dlgSignalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  switch (t) {
  case  GWEN_DialogEvent_TypeInit :
    return AG_Provider_EditUserDialog_init(dlg);
  case GWEN_DialogEvent_TypeActivated:
    return _dlgHandleActivated(dlg, sender);
  default:
    break;
  }

  return GWEN_DialogEvent_ResultHandled;

}



int _addUser(GWEN_DIALOG *dlg)
{
  AG_USER_DIALOG *xdlg;
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AG_USER_DIALOG, dlg);
  if (xdlg) {
    AB_USER *user = xdlg->user;
    if (!user) {
      user = AB_Provider_CreateUserObject(xdlg->provider);
      AB_Provider_AddUser(xdlg->provider, user);
    }
    char *u_name = strdup(GWEN_Dialog_GetCharProperty(dlg, "username_edit", GWEN_DialogProperty_Value, 0, NULL));
    char *u_id = strdup(GWEN_Dialog_GetCharProperty(dlg, "userid_edit", GWEN_DialogProperty_Value, 0, NULL));
    DBG_INFO(AQGIVVE_LOGDOMAIN, "user: (%s)", u_name);
    AB_User_SetUserName(user, u_name);
    AB_User_SetUserId(user, u_id);
    AB_User_SetCustomerId(user, u_id);
    AB_Provider_WriteUser(xdlg->provider, AB_User_GetUniqueId(user), 1, 1, user);

    char *token = AG_Provider_Request_GetToken(user);

    if (!token) {
      return GWEN_DialogEvent_ResultNotHandled;
    };
    DBG_INFO(AQGIVVE_LOGDOMAIN, "token: %s ", token);

    AG_VOUCHERLIST *card_list;
    card_list = AG_Provider_Request_GetVoucherList(token);
    if (AG_VOUCHERLIST_Get_TotalEntries(card_list) < 1) {
      DBG_INFO(AQGIVVE_LOGDOMAIN, "no cards found");
      GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN, "Warning", "Could not find any card", "OK", NULL, NULL, 0);
      return GWEN_DialogEvent_ResultAccept;
    }

    AG_VOUCHER *c = AG_VOUCHERLIST_Get_Card_By_Index(card_list, 0);
    const AG_VOUCHEROWNER *o = AG_VOUCHER_GetOwner(c);

    DBG_INFO(AQGIVVE_LOGDOMAIN, "card: %s, owner: %s", AG_VOUCHER_GetID(c), AG_VOUCHEROWNER_GetName(o));
    GWEN_DIALOG *cards_dialog = AG_CardsDialog_new(xdlg->provider, user, card_list);
    GWEN_Gui_ExecDialog(cards_dialog, GWEN_Dialog_GetGuiId(cards_dialog));

    AG_VOUCHERLIST_free(card_list, 1);
    free(token);
    free(u_id);
    free(u_name);
  }
  return GWEN_DialogEvent_ResultAccept;
}



int _dlgHandleActivated(GWEN_DIALOG *dlg, const char *sender)
{
  if (strcasecmp(sender, "next_button") ==0) {
    return _addUser(dlg);
  }
  else if (strcasecmp(sender, "abort_button") == 0) {
    return GWEN_DialogEvent_ResultReject;

  }

  return GWEN_DialogEvent_ResultReject;
}


