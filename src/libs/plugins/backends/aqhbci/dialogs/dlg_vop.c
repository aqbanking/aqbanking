/***************************************************************************
 begin       : Sat Nov 29 2025
 copyright   : (C) 2025 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "./dlg_vop_p.h"

#include "aqbanking/i18n_l.h"
#include "aqbanking/banking_l.h"

#include <ctype.h>



/* ------------------------------------------------------------------------------------------------
 * defines
 * ------------------------------------------------------------------------------------------------
 */

#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 200

#define DLG_DIALOGFILE   "aqbanking/backends/aqhbci/dialogs/dlg_vop.dlg"

#define VOPMSG_MAXWIDTH_IN_CHARS 64
#define VOPMSG_NEWLINE           "\n"

/* IDs for dialog widgets */
#define ID_VOPMSG           "vopMsg"
#define ID_VOPRESULTLISTBOX "vopResultListBox"



/* ------------------------------------------------------------------------------------------------
 * types
 * ------------------------------------------------------------------------------------------------
 */

typedef int (*_DIALOG_SIGNAL_HANDLER_FN)(GWEN_DIALOG *dlg);
typedef struct _DIALOG_SIGNAL_ENTRY _DIALOG_SIGNAL_ENTRY;
struct _DIALOG_SIGNAL_ENTRY {
  const char *sender;
  GWEN_DIALOG_EVENTTYPE eventType;
  _DIALOG_SIGNAL_HANDLER_FN handlerFn;
};



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static int GWENHYWFAR_CB _dlgApi_signalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender);

static int _handleInit(GWEN_DIALOG *dlg);
static int _handleFini(GWEN_DIALOG *dlg);

static int _handleActivatedAccept(GWEN_DIALOG *dlg);
static int _handleActivatedReject(GWEN_DIALOG *dlg);

static void _setIntroLabel(GWEN_DIALOG *dlg);
static void _buildResultList(GWEN_DIALOG *dlg, const char *widgetName, const AH_VOP_RESULT_LIST *resultList);
static void _vopMsgToGui(GWEN_DIALOG *dlg, const char *widgetName, const char *sVopMsg, int maxLen);
static void _stringListToBufferPlain(const GWEN_STRINGLIST *sl, int maxLen, GWEN_BUFFER *tbuf);
static void _stringListToBufferHtml(const GWEN_STRINGLIST *sl, GWEN_BUFFER *tbuf);
static GWEN_STRINGLIST *_htmlTextToStringList(const uint8_t *s);
static const uint8_t *_readNextWordIntoBuffer(const uint8_t *s, GWEN_BUFFER *tbuf);



/* ------------------------------------------------------------------------------------------------
 * static vars
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(GWEN_DIALOG, AH_VOP_DIALOG)


static _DIALOG_SIGNAL_ENTRY _signalMap[]={
  {NULL,                   GWEN_DialogEvent_TypeInit,         _handleInit},
  {NULL,                   GWEN_DialogEvent_TypeFini,         _handleFini},

  {"acceptButton",         GWEN_DialogEvent_TypeActivated,    _handleActivatedAccept},
  {"abortButton",          GWEN_DialogEvent_TypeActivated,    _handleActivatedReject},

  {NULL, 0, NULL}
};



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

GWEN_DIALOG *AH_VopDialog_new(const char *jobName, const char *bankName, const char *userName,
                              const char *vopMsg,
                              const AH_VOP_RESULT_LIST *resultList)
{
  GWEN_DIALOG *dlg;
  AH_VOP_DIALOG *xdlg;

  dlg=GWEN_Dialog_CreateAndLoadWithPath("ah_vopmsg", AB_PM_LIBNAME, AB_PM_DATADIR, DLG_DIALOGFILE);
  if (dlg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return NULL;
  }

  GWEN_NEW_OBJECT(AH_VOP_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AH_VOP_DIALOG, dlg, xdlg, _freeData);
  GWEN_Dialog_SetSignalHandler(dlg, _dlgApi_signalHandler);

  xdlg->vopMsg=vopMsg?strdup(vopMsg):NULL;
  xdlg->resultList=resultList;
  xdlg->jobName=jobName?strdup(jobName):NULL;
  xdlg->bankName=bankName?strdup(bankName):NULL;
  xdlg->userName=userName?strdup(userName):NULL;

  return dlg;
}



void GWENHYWFAR_CB _freeData(void *bp, void *p)
{
  AH_VOP_DIALOG *xdlg;

  xdlg=(AH_VOP_DIALOG*) p;
  free(xdlg->userName);
  free(xdlg->bankName);
  free(xdlg->jobName);
  free(xdlg->vopMsg);
  GWEN_FREE_OBJECT(xdlg);
}



int _handleInit(GWEN_DIALOG *dlg)
{
  AH_VOP_DIALOG *xdlg;
  GWEN_DB_NODE *dbPrefs;
  int i;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_VOP_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  GWEN_Dialog_SetCharProperty(dlg, "", GWEN_DialogProperty_Title, 0, I18N("Verification of Payee"), 0);

  _setIntroLabel(dlg);
  _vopMsgToGui(dlg, ID_VOPMSG, xdlg->vopMsg, VOPMSG_MAXWIDTH_IN_CHARS);

  /* setup result list */
  GWEN_Dialog_SetCharProperty(dlg, ID_VOPRESULTLISTBOX, GWEN_DialogProperty_Title, 0,
                              I18N("Result\tRemote IBAN\tRemote Name\tCorrected Remote Name\tLocal BIC"), 0);
  GWEN_Dialog_SetIntProperty(dlg, ID_VOPRESULTLISTBOX, GWEN_DialogProperty_SelectionMode, 0, GWEN_Dialog_SelectionMode_Single, 0);
  _buildResultList(dlg, ID_VOPRESULTLISTBOX, xdlg->resultList);

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  /* read settings for result list box */
  GWEN_Dialog_ListReadColumnSettings(dlg, ID_VOPRESULTLISTBOX, "vop_result_list_", 5, 64, dbPrefs);

  return GWEN_DialogEvent_ResultHandled;
}



int _handleFini(GWEN_DIALOG *dlg)
{
  AH_VOP_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_VOP_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  DBG_ERROR(AQHBCI_LOGDOMAIN, "fini");

  /* store dialog width */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "dialog_width", i);

  /* store dialog height */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, -1);
  GWEN_DB_SetIntValue(dbPrefs, GWEN_DB_FLAGS_OVERWRITE_VARS, "dialog_height", i);

  /* store settings for result list box */
  GWEN_Dialog_ListWriteColumnSettings(dlg, ID_VOPRESULTLISTBOX, "vop_result_list_", 5, 64, dbPrefs);

  return GWEN_DialogEvent_ResultHandled;
}



int _handleActivatedAccept(GWEN_DIALOG *dlg)
{
  return GWEN_DialogEvent_ResultAccept;
}



int _handleActivatedReject(GWEN_DIALOG *dlg)
{
  return GWEN_DialogEvent_ResultReject;
}



int GWENHYWFAR_CB _dlgApi_signalHandler(GWEN_DIALOG *dlg, GWEN_DIALOG_EVENTTYPE t, const char *sender)
{
  const _DIALOG_SIGNAL_ENTRY *entry;

  entry=_signalMap;
  while(entry->handlerFn) {
    if (entry->eventType==t && (entry->sender==NULL || (sender && strcasecmp(sender, entry->sender)==0))) {
      return entry->handlerFn(dlg);
    }
    entry++;
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



void _setIntroLabel(GWEN_DIALOG *dlg)
{
  AH_VOP_DIALOG *xdlg;
  GWEN_BUFFER *guiBuf;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AH_VOP_DIALOG, dlg);
  assert(xdlg);

  guiBuf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendArgs(guiBuf,
    I18N("These are the results of the Verification of Payee process at the bank (user %s at %s).\n"
         "FinTS specifications and your bank require us to show this dialog.\n"
         "If your bank provided a message it will be included below (the formatting probably had to be\n"
         "changed but otherwise the message was not altered).\n"
         "Sometimes there are no results reported by the bank, in that case the result list below is empty.\n"
         "\n"
         "If you still want to execute the job \"%s\" click \"Accept\".\n"

         "<html>"
         "<p>These are the results of the <i>Verification of Payee</i> process at the bank (user <i>%s</i> at <i>%s</i>).</p>"
         "<p>FinTS specifications and your bank require us to show this dialog.</p>"
         "<p>If your bank provided a message it will be included below (the formatting probably had to be "
         "changed but otherwise the message was not altered).</p>"
         "<p>Sometimes there are no results reported by the bank, in that case the result list below is empty.</p>"
         "<p>If you still want to execute the job <b>%s</b> click <b>Accept</b>.</p>"
         "</html>"
        ),
    xdlg->userName?xdlg->userName:I18N("<no user id>"),
    xdlg->bankName?xdlg->bankName:I18N("<no bank name>"),
    xdlg->jobName?xdlg->jobName:I18N("<no job name>"),
    xdlg->userName?xdlg->userName:I18N("<no user id>"),
    xdlg->bankName?xdlg->bankName:I18N("<no bank name>"),
    xdlg->jobName?xdlg->jobName:I18N("<no job name>"));

  GWEN_Dialog_SetCharProperty(dlg, "introLabel", GWEN_DialogProperty_Title, 0, GWEN_Buffer_GetStart(guiBuf), 0);
  GWEN_Buffer_free(guiBuf);
}



void _buildResultList(GWEN_DIALOG *dlg, const char *widgetName, const AH_VOP_RESULT_LIST *resultList)
{
  if (resultList) {
    const AH_VOP_RESULT *vopResult;
    GWEN_BUFFER *tbuf;

    GWEN_Dialog_SetIntProperty(dlg, widgetName, GWEN_DialogProperty_ClearValues, 0, 0, 0);

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    vopResult=AH_VopResult_List_First(resultList);
    while(vopResult) {
      const char *sRemoteIban=AH_VopResult_GetRemoteIban(vopResult);
      const char *sRemoteName=AH_VopResult_GetRemoteName(vopResult);
      const char *sAltRemoteName=AH_VopResult_GetAltRemoteName(vopResult);
      const char *sResult=AH_VopResultCode_toGuiString(AH_VopResult_GetResult(vopResult));
      const char *sLocalBic=AH_VopResult_GetLocalBic(vopResult);

      GWEN_Buffer_AppendArgs(tbuf, "%s\t%s\t", sResult?sResult:"", sRemoteIban?sRemoteIban:"");
      if (sRemoteName && *sRemoteName)
	AB_Banking_Iso8859_1ToUtf8(sRemoteName, strlen(sRemoteName), tbuf);
      GWEN_Buffer_AppendByte(tbuf, '\t');
      if (sAltRemoteName && *sAltRemoteName)
	AB_Banking_Iso8859_1ToUtf8(sAltRemoteName, strlen(sAltRemoteName), tbuf);
      GWEN_Buffer_AppendByte(tbuf, '\t');
      if (sLocalBic && *sLocalBic)
	GWEN_Buffer_AppendString(tbuf, sLocalBic);

      GWEN_Dialog_SetCharProperty(dlg, widgetName, GWEN_DialogProperty_AddValue, 0, GWEN_Buffer_GetStart(tbuf), 0);
      GWEN_Buffer_Reset(tbuf);
      vopResult=AH_VopResult_List_Next(vopResult);
    }
    GWEN_Buffer_free(tbuf);
  }
}


/* this might be interesting for GWEN_TEXT, too. */
void _vopMsgToGui(GWEN_DIALOG *dlg, const char *widgetName, const char *sVopMsg, int maxLen)
{
  if (sVopMsg) {
    GWEN_STRINGLIST *sl;
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 256, 0, 1);
    AB_Banking_Iso8859_1ToUtf8(sVopMsg, strlen(sVopMsg), ubuf);

    sl=_htmlTextToStringList((const uint8_t*) GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    if (sl) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      _stringListToBufferPlain(sl, maxLen, tbuf);
      _stringListToBufferHtml(sl, tbuf);
      if (GWEN_Buffer_GetUsedBytes(tbuf))
        GWEN_Dialog_SetCharProperty(dlg, widgetName, GWEN_DialogProperty_Value, 0, GWEN_Buffer_GetStart(tbuf), 0);
      GWEN_Buffer_free(tbuf);
      GWEN_StringList_free(sl);
    }
  }
}



void _stringListToBufferPlain(const GWEN_STRINGLIST *sl, int maxLen, GWEN_BUFFER *tbuf)
{
  if (sl) {
    int currentWidth=0;
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        DBG_ERROR(NULL, "Adding word [%s]", s);
        if (*s==10) {
          DBG_ERROR(NULL, "Adding a forced new line");
          /* forced newline */
          GWEN_Buffer_AppendString(tbuf, VOPMSG_NEWLINE);
          currentWidth=0;
        }
        else if (*s=='<') {
          /* html element */
          if (strcasecmp(s, "<br>")==0) {
            DBG_ERROR(NULL, "Adding a new line (because of <br>");
            GWEN_Buffer_AppendString(tbuf, VOPMSG_NEWLINE);
            currentWidth=0;
          }
          else if (strcasecmp(s, "<p>")==0 || strcasecmp(s, "</p>")==0) {
            DBG_ERROR(NULL, "Adding a new line (because of <p> or </p>");
            GWEN_Buffer_AppendString(tbuf, VOPMSG_NEWLINE);
            currentWidth=0;
          }
          else {
            DBG_INFO(AQHBCI_LOGDOMAIN, "Ignoring format element \"%s\"", s);
          }
        }
        else {
          int wlen;
  
          /* word */
          wlen=strlen(s);
          if (wlen>VOPMSG_MAXWIDTH_IN_CHARS) {
            /* just add the word as it is, we don't cut it for now */
            GWEN_Buffer_AppendString(tbuf, VOPMSG_NEWLINE);
            GWEN_Buffer_AppendString(tbuf, s);
            GWEN_Buffer_AppendString(tbuf, VOPMSG_NEWLINE);
            currentWidth=0;
          }
          else {
            if ((currentWidth+wlen+1)>maxLen) {
              GWEN_Buffer_AppendString(tbuf, VOPMSG_NEWLINE);
              GWEN_Buffer_AppendString(tbuf, s);
              currentWidth=wlen;
            }
            else {
              if (currentWidth)
                GWEN_Buffer_AppendByte(tbuf, ' ');
              GWEN_Buffer_AppendString(tbuf, s);
              currentWidth+=wlen+1;
            }
          }
        }
      }
      se=GWEN_StringListEntry_Next(se);
    }
  }
}



void _stringListToBufferHtml(const GWEN_STRINGLIST *sl, GWEN_BUFFER *tbuf)
{
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    GWEN_Buffer_AppendString(tbuf, "<html>");
    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        if (*s==10) {
          /* forced newline */
          GWEN_Buffer_AppendString(tbuf, "<br>");
        }
        else if (*s=='<') {
          /* html element */
          if (strcasecmp(s, "<br>")==0 ||
              strcasecmp(s, "<p>")==0 ||
              strcasecmp(s, "</p>")==0 ||
              strcasecmp(s, "<b>")==0 ||
              strcasecmp(s, "</b>")==0) {
            GWEN_Buffer_AppendString(tbuf, s);
          }
          else {
            DBG_INFO(AQHBCI_LOGDOMAIN, "Ignoring format element \"%s\"", s);
          }
        }
        else {
          GWEN_Buffer_AppendString(tbuf, s);
          GWEN_Buffer_AppendByte(tbuf, ' ');
        }
      }
      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_Buffer_AppendString(tbuf, "</html>");
  }
}



GWEN_STRINGLIST *_htmlTextToStringList(const uint8_t *s)
{
  if (s && *s) {
    GWEN_STRINGLIST *sl;
    GWEN_BUFFER *tbuf;

    sl=GWEN_StringList_new();
    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    while(*s) {
      const uint8_t *t;

      t=_readNextWordIntoBuffer(s, tbuf);
      if (t!=s) {
        GWEN_StringList_AppendString(sl, GWEN_Buffer_GetStart(tbuf), 0, 0);
        s=t;
      }
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No word read at %s, aborting.", s);
        break;
      }
      GWEN_Buffer_Reset(tbuf);
    }
    GWEN_Buffer_free(tbuf);

    if (GWEN_StringList_Count(sl)>0)
      return sl;
    GWEN_StringList_free(sl);
  }

  return NULL;
}



const uint8_t *_readNextWordIntoBuffer(const uint8_t *s, GWEN_BUFFER *tbuf)
{
  int wordFinished=0;
  int charsAdded=0;

  while(*s && !wordFinished) {
    uint8_t c;

    c=*s;
    if (c==10) {
      wordFinished=1;
      if (!charsAdded) {
        GWEN_Buffer_AppendByte(tbuf, 10);
        charsAdded++;
        wordFinished=1;
        s++;
        /* skip blanks */
        while(s && isspace(*s))
          s++;
      }
    }
    else if (c<33) {
      wordFinished=1;
      s++;
      /* skip blanks */
      while(s && isspace(*s))
        s++;
    }
    else if (c>127) {
      while(*s && (*s & 0x80)) {
	GWEN_Buffer_AppendByte(tbuf, *s);
	s++;
      }
      charsAdded++;
    }
    else if (c=='<') {
      if (charsAdded) {
        /* read html element as one word. If this is not the beginning of a word return the current word */
        wordFinished=1;
      }
      else {
        const uint8_t *t=s;

        while(*t && *t!='>') {
          GWEN_Buffer_AppendByte(tbuf, *(t++));
        }
        if (*t=='>') {
          GWEN_Buffer_AppendByte(tbuf, '>');
          wordFinished=1;
          s=++t;
        }
      }
    }
    else {
      GWEN_Buffer_AppendByte(tbuf, c);
      charsAdded++;
      s++;
    }
  }

  return s;
}




