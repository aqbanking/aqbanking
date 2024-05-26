/***************************************************************************
 begin       : Wed May 01 2024
 copyright   : (C) 2024 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "w_usercombo.h"

#include "aqbanking/i18n_l.h"




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void _userComboAddUser(GWEN_DIALOG *dlg, const char *widgetName, const AB_USER *u);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

void AH_Widget_UserComboRebuild(GWEN_DIALOG *dlg, const char *widgetName, AB_PROVIDER *provider)
{
  AB_USER_LIST *users;
  int rv;

  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_ClearValues, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg,
                              widgetName,
                              GWEN_DialogProperty_AddValue,
                              0,
                              I18N("-- select --"),
                              0);

  users=AB_User_List_new();
  rv=AB_Provider_ReadUsers(provider, users);
  if (rv>=0) {
    const AB_USER *u;

    u=AB_User_List_First(users);
    while (u) {
      _userComboAddUser(dlg, widgetName, u);
      u=AB_User_List_Next(u);
    }
    AB_User_List_free(users);

    GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Sort, 0, 0, 0);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error reading users (%d)", rv);
  }
}



int AH_Widget_UserComboFindUserByUid(GWEN_DIALOG *dlg, const char *widgetName, uint32_t uid)
{
  int idx;

  for (idx=0; ; idx++) {
    const char *s;

    s=GWEN_Dialog_GetCharProperty(dlg, widgetName, GWEN_DialogProperty_Value, idx, NULL);
    if (s && *s) {
      unsigned long int currentUid;

      if (1==sscanf(s, "%lu", &currentUid) && currentUid==(unsigned long int) uid)
        return idx;
    }
    else
      break;
  } /* for */

  return -1;
}



void AH_Widget_UserComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, uint32_t uid)
{
  if (uid) {
    int idx;

    idx=AH_Widget_UserComboFindUserByUid(dlg, widgetName, uid);
    if (idx>=0)
      GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, idx, 0);
  }
}



uint32_t AH_Widget_UserComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName)
{
  int idx;

  idx=GWEN_Dialog_GetIntProperty(dlg, widgetName,  GWEN_DialogProperty_Value, 0, -1);
  if (idx>=0) {
    const char *s;

    s=GWEN_Dialog_GetCharProperty(dlg, widgetName, GWEN_DialogProperty_Value, idx, NULL);
    if (s && *s) {
      unsigned long int currentUid;

      if (1==sscanf(s, "%lu", &currentUid))
        return (uint32_t) currentUid;
    }
  }

  return 0;
}



void _userComboAddUser(GWEN_DIALOG *dlg, const char *widgetName, const AB_USER *u)
{
  GWEN_BUFFER *buf;
  uint32_t uid;
  const char *s;

  buf=GWEN_Buffer_new(0, 256, 0, 1);

  /* column 1 */
  uid=AB_User_GetUniqueId(u);
  GWEN_Buffer_AppendArgs(buf, "%lu ", (unsigned long int) uid);

  /* column 2 */
  s=AB_User_GetBankCode(u);
  GWEN_Buffer_AppendArgs(buf, "%s ", s?s:"");

  /* column 3 */
  s=AB_User_GetBankCode(u);
  GWEN_Buffer_AppendArgs(buf, "%s ", s?s:"");

  /* column 4 */
  s=AB_User_GetCustomerId(u);
  if (!(s && *s))
    s=AB_User_GetUserId(u);
  GWEN_Buffer_AppendArgs(buf, "%s ", s?s:"");

  /* column 5 */
  s=AB_User_GetUserName(u);
  GWEN_Buffer_AppendArgs(buf, "%s", s?s:"");

  GWEN_Dialog_SetCharProperty(dlg, widgetName, GWEN_DialogProperty_AddValue, 0, GWEN_Buffer_GetStart(buf), 0);
  GWEN_Buffer_free(buf);
}



