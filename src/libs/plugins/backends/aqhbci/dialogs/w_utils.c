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

#include "w_utils.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gui.h>


static void _removeAllSpaces(uint8_t *s);




void AH_Widget_UserToGuiText(GWEN_DIALOG *dlg, const char *widgetName, const AB_USER *user, AH_WIDGET_USER_GETCHARVALUE_FN fn)
{
  const char *s;

  s=fn(user);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, s?s:"", 0);
}



int AH_Widget_GuiTextToUserDeleSpaces(GWEN_DIALOG *dlg, const char *widgetName,
                                      AB_USER *user, AH_WIDGET_USER_SETCHARVALUE_FN fn,
                                      const char *errMsgIfMissing)
{
  const char *s;

  s=GWEN_Dialog_GetCharProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    _removeAllSpaces((uint8_t *)GWEN_Buffer_GetStart(tbuf));
    if (user)
      fn(user, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(NULL, "Missing input from widget %s", widgetName);
    if (errMsgIfMissing) {
      GWEN_Gui_ShowError(I18N("Error on Input"), "%s", errMsgIfMissing);
      GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Focus, 0, 1, 0);
      return GWEN_ERROR_INVALID;
    }
    if (user)
      fn(user, NULL);
  }
  return 0;
}



int AH_Widget_GuiTextToUserKeepSpaces(GWEN_DIALOG *dlg, const char *widgetName,
                                      AB_USER *user, AH_WIDGET_USER_SETCHARVALUE_FN fn,
                                      const char *errMsgIfMissing)
{
  const char *s;

  s=GWEN_Dialog_GetCharProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (user)
      fn(user, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(NULL, "Missing input from widget %s", widgetName);
    if (errMsgIfMissing) {
      GWEN_Gui_ShowError(I18N("Error on Input"), "%s", errMsgIfMissing);
      GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Focus, 0, 1, 0);
      return GWEN_ERROR_INVALID;
    }
    if (user)
      fn(user, NULL);
  }
  return 0;
}






void AH_Widget_AccountToGuiText(GWEN_DIALOG *dlg, const char *widgetName, const AB_ACCOUNT *acc, AH_WIDGET_ACCOUNT_GETCHARVALUE_FN fn)
{
  const char *s;

  s=fn(acc);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, s?s:"", 0);
}



int AH_Widget_GuiTextToAccountDeleSpaces(GWEN_DIALOG *dlg, const char *widgetName,
                                         AB_ACCOUNT *acc, AH_WIDGET_ACCOUNT_SETCHARVALUE_FN fn,
                                         const char *errMsgIfMissing)
{
  const char *s;

  s=GWEN_Dialog_GetCharProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    _removeAllSpaces((uint8_t *)GWEN_Buffer_GetStart(tbuf));
    if (acc)
      fn(acc, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(NULL, "Missing input from widget %s", widgetName);
    if (errMsgIfMissing) {
      GWEN_Gui_ShowError(I18N("Error on Input"), "%s", errMsgIfMissing);
      GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Focus, 0, 1, 0);
      return GWEN_ERROR_INVALID;
    }
    if (acc)
      fn(acc, NULL);
  }
  return 0;
}



int AH_Widget_GuiTextToAccountKeepSpaces(GWEN_DIALOG *dlg, const char *widgetName,
                                         AB_ACCOUNT *acc, AH_WIDGET_ACCOUNT_SETCHARVALUE_FN fn,
                                         const char *errMsgIfMissing)
{
  const char *s;

  s=GWEN_Dialog_GetCharProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    if (acc)
      fn(acc, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(NULL, "Missing input from widget %s", widgetName);
    if (errMsgIfMissing) {
      GWEN_Gui_ShowError(I18N("Error on Input"), "%s", errMsgIfMissing);
      GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Focus, 0, 1, 0);
      return GWEN_ERROR_INVALID;
    }
    if (acc)
      fn(acc, NULL);
  }
  return 0;
}



void _removeAllSpaces(uint8_t *s)
{
  uint8_t *d;

  d=s;
  while (*s) {
    if (*s>33)
      *(d++)=*s;
    s++;
  }
  *d=0;
}




