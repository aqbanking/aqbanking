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

#include "w_tanmethodcombo.h"

#include "aqbanking/i18n_l.h"


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _createTanMethodString(const AH_TAN_METHOD *tm, GWEN_BUFFER *tbuf);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

void AH_Widget_TanMethodComboRebuild(GWEN_DIALOG *dlg, const char *widgetName, const AH_TAN_METHOD_LIST *ctl)
{
  GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_ClearValues, 0, 0, 0);
  GWEN_Dialog_SetCharProperty(dlg, widgetName, GWEN_DialogProperty_AddValue, 0, I18N("-- select --"), 0);
  if (ctl) {
    AH_TAN_METHOD *tm;
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    tm=AH_TanMethod_List_First(ctl);
    while (tm) {
      if (_createTanMethodString(tm, tbuf)==0)
        GWEN_Dialog_SetCharProperty(dlg, widgetName, GWEN_DialogProperty_AddValue, 0, GWEN_Buffer_GetStart(tbuf), 0);
      GWEN_Buffer_Reset(tbuf);

      tm=AH_TanMethod_List_Next(tm);
    }
    GWEN_Buffer_free(tbuf);
  }
}



int AH_Widget_TanMethodComboFindMethodById(GWEN_DIALOG *dlg, const char *widgetName, int id)
{
  int idx;

  for (idx=0; ; idx++) {
    const char *s;

    s=GWEN_Dialog_GetCharProperty(dlg, widgetName, GWEN_DialogProperty_Value, idx, NULL);
    if (s && *s) {
      int currentId;

      if (1==sscanf(s, "%u", &currentId) && currentId==id)
        return idx;
    }
    else
      break;
  } /* for */

  return -1;
}



void AH_Widget_TanMethodComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, int id)
{
  if (id) {
    int idx;

    idx=AH_Widget_TanMethodComboFindMethodById(dlg, widgetName, id);
    if (idx>=0)
      GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, idx, 0);
  }
}



int AH_Widget_TanMethodComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName)
{
  int idx;

  idx=GWEN_Dialog_GetIntProperty(dlg, widgetName,  GWEN_DialogProperty_Value, 0, -1);
  if (idx>=0) {
    const char *s;

    s=GWEN_Dialog_GetCharProperty(dlg, widgetName, GWEN_DialogProperty_Value, idx, NULL);
    if (s && *s) {
      int currentId;

      if (1==sscanf(s, "%u", &currentId))
        return currentId;
    }
  }

  return 0;
}



int _createTanMethodString(const AH_TAN_METHOD *tm, GWEN_BUFFER *tbuf)
{
  const char *s;

  GWEN_Buffer_AppendArgs(tbuf, "%d - ", (AH_TanMethod_GetGvVersion(tm)*1000)+AH_TanMethod_GetFunction(tm));
  GWEN_Buffer_AppendArgs(tbuf, "%d", AH_TanMethod_GetFunction(tm));

  s=AH_TanMethod_GetMethodName(tm);
  if (!(s && *s))
    s=AH_TanMethod_GetMethodId(tm);
  if (s && *s)
    GWEN_Buffer_AppendArgs(tbuf, " - %s", s);

  /* add HKTAN version */
  GWEN_Buffer_AppendArgs(tbuf, " (Version %d)", AH_TanMethod_GetGvVersion(tm));

  return 0;
}



