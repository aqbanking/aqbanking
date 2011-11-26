/***************************************************************************
 begin       : Tue Apr 13 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "dlg_selectbankinfo_p.h"
#include "i18n_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>


#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 200
#define LIST_MINCOLWIDTH 50




GWEN_INHERIT(GWEN_DIALOG, AB_SELECTBANKINFO_DIALOG)





GWEN_DIALOG *AB_SelectBankInfoDialog_new(AB_BANKING *ab,
					 const char *country,
					 const char *bankCode) {
  GWEN_DIALOG *dlg;
  AB_SELECTBANKINFO_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("ab_selectbankinfo");
  GWEN_NEW_OBJECT(AB_SELECTBANKINFO_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AB_SELECTBANKINFO_DIALOG, dlg, xdlg,
		       AB_SelectBankInfoDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AB_SelectBankInfoDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/dialogs/dlg_selectbankinfo.dlg",
			       fbuf);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(fbuf));
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  GWEN_Buffer_free(fbuf);

  xdlg->banking=ab;

  if (country) xdlg->country=strdup(country);
  else xdlg->country=strdup("de");

  if (bankCode) xdlg->bankCode=strdup(bankCode);
  else xdlg->bankCode=NULL;

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AB_SelectBankInfoDialog_FreeData(void *bp, void *p) {
  AB_SELECTBANKINFO_DIALOG *xdlg;

  xdlg=(AB_SELECTBANKINFO_DIALOG*) p;
  AB_BankInfo_free(xdlg->selectedBankInfo);
  AB_BankInfo_List2_freeAll(xdlg->matchingBankInfos);
  free(xdlg->country);
  free(xdlg->bankCode);
  GWEN_FREE_OBJECT(xdlg);
}



static void createListBoxString(const AB_BANKINFO *bi, GWEN_BUFFER *tbuf) {
  const char *s;
  AB_BANKINFO_SERVICE *sv;
  uint32_t pos;
  int svsAdded=0;

  s=AB_BankInfo_GetBankId(bi);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");
  
  s=AB_BankInfo_GetBic(bi);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");
  
  s=AB_BankInfo_GetBankName(bi);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");
  
  s=AB_BankInfo_GetLocation(bi);
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "\t");

  pos=GWEN_Buffer_GetPos(tbuf);
  sv=AB_BankInfoService_List_First(AB_BankInfo_GetServices(bi));
  while(sv) {
    const char *s;

    s=AB_BankInfoService_GetType(sv);
    if (s && *s) {
      const char *p;

      p=GWEN_Buffer_GetStart(tbuf)+pos;
      if (strstr(p, s)==NULL) {
	if (svsAdded)
	  GWEN_Buffer_AppendString(tbuf, ", ");
	GWEN_Buffer_AppendString(tbuf, s);
        svsAdded++;
      }
    }
    sv=AB_BankInfoService_List_Next(sv);
  }
}



const AB_BANKINFO *AB_SelectBankInfoDialog_GetSelectedBankInfo(GWEN_DIALOG *dlg) {
  AB_SELECTBANKINFO_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBANKINFO_DIALOG, dlg);
  assert(xdlg);

  return xdlg->selectedBankInfo;
}



AB_BANKINFO *AB_SelectBankInfoDialog_DetermineSelectedBankInfo(GWEN_DIALOG *dlg) {
  AB_SELECTBANKINFO_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBANKINFO_DIALOG, dlg);
  assert(xdlg);

  if (xdlg->matchingBankInfos) {
    AB_BANKINFO_LIST2_ITERATOR *it;
    int idx;

    idx=GWEN_Dialog_GetIntProperty(dlg, "listBox", GWEN_DialogProperty_Value, 0, -1);
    if (idx>=0) {
      const char *currentText;

      currentText=GWEN_Dialog_GetCharProperty(dlg, "listBox", GWEN_DialogProperty_Value, idx, NULL);
      if (currentText && *currentText) {
	it=AB_BankInfo_List2_First(xdlg->matchingBankInfos);
	if (it) {
	  AB_BANKINFO *bi;
	  GWEN_BUFFER *tbuf;

	  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  bi=AB_BankInfo_List2Iterator_Data(it);
	  while(bi) {
	    createListBoxString(bi, tbuf);

	    if (strcasecmp(currentText, GWEN_Buffer_GetStart(tbuf))==0) {
	      GWEN_Buffer_free(tbuf);
	      AB_BankInfo_List2Iterator_free(it);
	      return bi;
	    }

	    GWEN_Buffer_Reset(tbuf);
	    bi=AB_BankInfo_List2Iterator_Next(it);
	  }

	  GWEN_Buffer_free(tbuf);
	  AB_BankInfo_List2Iterator_free(it);
	}
      }
    }
  }
  return NULL;
}



static void removeAllSpaces(uint8_t *s) {
  uint8_t *d;

  d=s;
  while(*s) {
    if (*s>33)
      *(d++)=*s;
    s++;
  }
  *d=0;
}



void AB_SelectBankInfoDialog_UpdateList(GWEN_DIALOG *dlg) {
  AB_SELECTBANKINFO_DIALOG *xdlg;
  AB_BANKINFO *tbi;
  const char *s;
  AB_BANKINFO_LIST2 *bl;
  int rv;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBANKINFO_DIALOG, dlg);
  assert(xdlg);

  /* clear bank info list */
  GWEN_Dialog_SetIntProperty(dlg, "listBox", GWEN_DialogProperty_ClearValues, 0, 0, 0);
  if (xdlg->matchingBankInfos)
    AB_BankInfo_List2_freeAll(xdlg->matchingBankInfos);
  xdlg->matchingBankInfos=NULL;

  /* setup template */
  tbi=AB_BankInfo_new();

  /* set country */
  AB_BankInfo_SetCountry(tbi, xdlg->country);

  /* set bank code */
  s=GWEN_Dialog_GetCharProperty(dlg, "blzEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    int len;
    char *cpy;

    len=strlen(s);
    cpy=(char*) malloc(len+2);
    assert(cpy);
    memmove(cpy, s, len+1); /* copy including terminating zero char */
    removeAllSpaces((uint8_t*)cpy);
    len=strlen(cpy);
    if (len) {
      /* append joker */
      cpy[len]='*';
      cpy[len+1]=0;
    }
    AB_BankInfo_SetBankId(tbi, cpy);
    free(cpy);
  }

  /* set bank code */
  s=GWEN_Dialog_GetCharProperty(dlg, "bicEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    int len;
    char *cpy;

    len=strlen(s);
    cpy=(char*) malloc(len+2);
    assert(cpy);
    memmove(cpy, s, len+1); /* copy including terminating zero char */
    removeAllSpaces((uint8_t*)cpy);
    len=strlen(cpy);
    if (len) {
      /* append joker */
      cpy[len]='*';
      cpy[len+1]=0;
    }
    AB_BankInfo_SetBic(tbi, cpy);
    free(cpy);
  }

  /* set bank name */
  s=GWEN_Dialog_GetCharProperty(dlg, "nameEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    GWEN_Buffer_AppendString(tbuf, "*");
    AB_BankInfo_SetBankName(tbi, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  /* set bank name */
  s=GWEN_Dialog_GetCharProperty(dlg, "locationEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Text_CondenseBuffer(tbuf);
    GWEN_Buffer_AppendString(tbuf, "*");
    AB_BankInfo_SetLocation(tbi, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  bl=AB_BankInfo_List2_new();
  rv=AB_Banking_GetBankInfoByTemplate(xdlg->banking, xdlg->country, tbi, bl);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    AB_BankInfo_List2_freeAll(bl);
  }
  else {
    AB_BANKINFO_LIST2_ITERATOR *it;

    it=AB_BankInfo_List2_First(bl);
    if (it) {
      AB_BANKINFO *bi;
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      bi=AB_BankInfo_List2Iterator_Data(it);
      while(bi) {
	createListBoxString(bi, tbuf);

	GWEN_Dialog_SetCharProperty(dlg,
				    "listBox",
				    GWEN_DialogProperty_AddValue,
				    0,
				    GWEN_Buffer_GetStart(tbuf),
				    0);
	GWEN_Buffer_Reset(tbuf);
	bi=AB_BankInfo_List2Iterator_Next(it);
      }

      GWEN_Buffer_free(tbuf);
      AB_BankInfo_List2Iterator_free(it);
    }
    xdlg->matchingBankInfos=bl;
  }

  AB_BankInfo_free(tbi);

  if (GWEN_Dialog_GetIntProperty(dlg, "listBox", GWEN_DialogProperty_Value, 0, -1)>=0)
    GWEN_Dialog_SetIntProperty(dlg, "okButton", GWEN_DialogProperty_Enabled, 0, 1, 0);
  else
    GWEN_Dialog_SetIntProperty(dlg, "okButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
}



void AB_SelectBankInfoDialog_Init(GWEN_DIALOG *dlg) {
  AB_SELECTBANKINFO_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;
  int j;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBANKINFO_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Select a Bank"),
			      0);

  GWEN_Dialog_SetCharProperty(dlg,
			      "listBox",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Bank Code\tBIC\tName\tLocation\tProtocols"),
			      0);
  GWEN_Dialog_SetIntProperty(dlg,
			     "listBox",
			     GWEN_DialogProperty_SelectionMode,
			     0,
			     GWEN_Dialog_SelectionMode_Single,
			     0);


  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  /* read bank column widths */
  for (i=0; i<5; i++) {
    j=GWEN_DB_GetIntValue(dbPrefs, "bank_list_columns", i, -1);
    if (j<LIST_MINCOLWIDTH)
      j=LIST_MINCOLWIDTH;
    GWEN_Dialog_SetIntProperty(dlg, "listBox", GWEN_DialogProperty_ColumnWidth, i, j, 0);
  }
  /* get sort column */
  i=GWEN_DB_GetIntValue(dbPrefs, "bank_list_sortbycolumn", 0, -1);
  j=GWEN_DB_GetIntValue(dbPrefs, "bank_list_sortdir", 0, -1);
  if (i>=0 && j>=0)
    GWEN_Dialog_SetIntProperty(dlg, "listBox", GWEN_DialogProperty_SortDirection, i, j, 0);

  /* disable ok button */
  GWEN_Dialog_SetIntProperty(dlg, "okButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
}



void AB_SelectBankInfoDialog_Fini(GWEN_DIALOG *dlg) {
  AB_SELECTBANKINFO_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBANKINFO_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* store dialog width */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "dialog_width",
		      i);

  /* store dialog height */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "dialog_height",
		      i);


  /* store column widths of importer list */
  GWEN_DB_DeleteVar(dbPrefs, "bank_list_columns");
  for (i=0; i<5; i++) {
    int j;

    j=GWEN_Dialog_GetIntProperty(dlg, "listBox", GWEN_DialogProperty_ColumnWidth, i, -1);
    if (j<LIST_MINCOLWIDTH)
      j=LIST_MINCOLWIDTH;
    GWEN_DB_SetIntValue(dbPrefs,
			GWEN_DB_FLAGS_DEFAULT,
			"bank_list_columns",
			j);
  }
  /* store column sorting */
  GWEN_DB_SetIntValue(dbPrefs,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "bank_list_sortbycolumn",
		      -1);
  for (i=0; i<5; i++) {
    int j;

    j=GWEN_Dialog_GetIntProperty(dlg, "listBox", GWEN_DialogProperty_SortDirection, i,
				 GWEN_DialogSortDirection_None);
    if (j!=GWEN_DialogSortDirection_None) {
      GWEN_DB_SetIntValue(dbPrefs,
			  GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "bank_list_sortbycolumn",
			  i);
      GWEN_DB_SetIntValue(dbPrefs,
			  GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "bank_list_sortdir",
			  (j==GWEN_DialogSortDirection_Up)?1:0);
      break;
    }
  }
}



int AB_SelectBankInfoDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  AB_SELECTBANKINFO_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBANKINFO_DIALOG, dlg);
  assert(xdlg);

  DBG_NOTICE(0, "Activated: %s", sender);
  if (strcasecmp(sender, "blzEdit")==0 ||
      strcasecmp(sender, "bicEdit")==0 ||
      strcasecmp(sender, "nameEdit")==0 ||
      strcasecmp(sender, "locationEdit")==0) {
    AB_SelectBankInfoDialog_UpdateList(dlg);
    return GWEN_DialogEvent_ResultHandled;
  }
  else if (strcasecmp(sender, "listBox")==0) {
    AB_BANKINFO *bi;

    bi=AB_SelectBankInfoDialog_DetermineSelectedBankInfo(dlg);
    GWEN_Dialog_SetIntProperty(dlg, "okButton", GWEN_DialogProperty_Enabled, 0, bi?1:0, 0);
    return GWEN_DialogEvent_ResultHandled;
  }
  else if (strcasecmp(sender, "okButton")==0) {
    AB_BANKINFO *bi;

    bi=AB_SelectBankInfoDialog_DetermineSelectedBankInfo(dlg);
    if (bi)
      xdlg->selectedBankInfo=AB_BankInfo_dup(bi);
    return GWEN_DialogEvent_ResultAccept;
  }
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "wiz_help_button")==0) {
    /* TODO: open a help dialog */
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int AB_SelectBankInfoDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender) {
  const char *s;

  DBG_NOTICE(0, "Changed %s", sender);
  if (strcasecmp(sender, "blzEdit")==0 ||
      strcasecmp(sender, "bicEdit")==0) {
    s=GWEN_Dialog_GetCharProperty(dlg, sender, GWEN_DialogProperty_Value, 0, NULL);
    if (s && strlen(s)>2) {
      AB_SelectBankInfoDialog_UpdateList(dlg);
      return GWEN_DialogEvent_ResultHandled;
    }
  }
  else if (strcasecmp(sender, "nameEdit")==0 ||
	   strcasecmp(sender, "locationEdit")==0) {
    s=GWEN_Dialog_GetCharProperty(dlg, sender, GWEN_DialogProperty_Value, 0, NULL);
    if (s && strlen(s)>3) {
      AB_SelectBankInfoDialog_UpdateList(dlg);
      return GWEN_DialogEvent_ResultHandled;
    }
  }
  else if (strcasecmp(sender, "listBox")==0) {
    return GWEN_DialogEvent_ResultHandled;
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB AB_SelectBankInfoDialog_SignalHandler(GWEN_DIALOG *dlg,
							GWEN_DIALOG_EVENTTYPE t,
							const char *sender) {
  AB_SELECTBANKINFO_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_SELECTBANKINFO_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AB_SelectBankInfoDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AB_SelectBankInfoDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return AB_SelectBankInfoDialog_HandleValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return AB_SelectBankInfoDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}







