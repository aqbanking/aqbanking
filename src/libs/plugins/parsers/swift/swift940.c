/***************************************************************************
 begin       : Fri Apr 02 2004
 copyright   : (C) 2022 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "swift940_p.h"
#include "swift940_25.h"
#include "swift940_60.h"
#include "swift940_61.h"
#include "swift940_86.h"
#include "aqbanking/i18n_l.h"

#include <aqbanking/error.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <ctype.h>





int AHB_SWIFT940_Parse_NS(const AHB_SWIFT_TAG *tg,
                          uint32_t flags,
                          GWEN_DB_NODE *data,
                          GWEN_DB_NODE *cfg)
{
  const char *p;
  const char *p2;

  /* TODO: Use AHB_SWIFT_ParseSubTags */
  p=AHB_SWIFT_Tag_GetData(tg);
  assert(p);

  while (*p) {
    int code;

    code=0;
    /* read code */
    if (strlen(p)>2) {
      if (isdigit(p[0]) && isdigit(p[1])) {
        /* starts with a two digit number */
        code=(((p[0]-'0')*10) + (p[1]-'0'));
        p+=2;
      }
    }

    /* search for end of line */
    p2=p;
    while (*p2 && *p2!=10 && *p2!=13)
      p2++;

    if (code==0) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "No code in line");
      p=p2;
    }
    else {
      int len;

      len=p2-p;
      if (len<1 || (len==1 && *p=='/')) {
        DBG_DEBUG(AQBANKING_LOGDOMAIN, "Empty field %02d", code);
      }
      else {
        char *s;

        s=(char *)GWEN_Memory_malloc(len+1);
        memmove(s, p, len);
        s[len]=0;
        DBG_DEBUG(AQBANKING_LOGDOMAIN, "Got his field: %02d: %s", code, s);

        switch (code) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
          AHB_SWIFT_SetCharValue(data, flags, "purpose", s);
          break;

        case 15: /* Auftraggeber1 */
        case 16: /* Auftraggeber2 */
          AHB_SWIFT_SetCharValue(data, flags, "localName", s);
          break;

        case 17: /* Buchungstext */
          AHB_SWIFT_SetCharValue(data, flags, "transactionText", s);
          break;

        case 18: /* Primanota */
          AHB_SWIFT_SetCharValue(data, flags, "primanota", s);
          break;

        case 19: /* Uhrzeit der Buchung */
        case 20: /* Anzahl der Sammlerposten */
        case 33: /* BLZ Auftraggeber */
        case 34: /* Konto Auftraggeber */
          break;

        default: /* ignore all other fields (if any) */
          DBG_WARN(AQBANKING_LOGDOMAIN,
                   "Unknown :NS: field \"%02d\" (%s) (%s)",
                   code, s,
                   AHB_SWIFT_Tag_GetData(tg));
          break;
        }
        GWEN_Memory_dealloc(s);
      }
      p=p2;
    }

    if (*p==10)
      p++;
    if (*p==13)
      p++;
    if (*p==10)
      p++;
  } /* while */

  return 0;
}


/* Import SWIFT MT940 data.
   @param tl input: list of tags. Tags are lines in a SWIFT data block (block 4). A tag has an
          id and content. See the AHB_SWIFT_Tag_new function for more information.
 */
int AHB_SWIFT940_Import(AHB_SWIFT_TAG_LIST *tl,
                        GWEN_DB_NODE *data,
                        GWEN_DB_NODE *cfg,
                        uint32_t flags)
{
  AHB_SWIFT_TAG *tg;
  GWEN_DB_NODE *dbDay=NULL;
  GWEN_DB_NODE *dbTemplate=NULL;
  GWEN_DB_NODE *dbTransaction=NULL;
  const char *sDate=NULL;
  uint32_t progressId;
  const char *acceptTag20="*";
  const char *rejectTag20=NULL;
  int ignoreCurrentReport=0;

  acceptTag20=GWEN_DB_GetCharValue(cfg, "acceptTag20", 0, NULL);
  if (acceptTag20 && *acceptTag20==0)
    acceptTag20=NULL;
  rejectTag20=GWEN_DB_GetCharValue(cfg, "rejectTag20", 0, NULL);
  if (rejectTag20 && *rejectTag20==0)
    rejectTag20=NULL;

  dbTemplate=GWEN_DB_Group_new("template");

  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                                    GWEN_GUI_PROGRESS_ALLOW_EMBED |
                                    GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                                    GWEN_GUI_PROGRESS_SHOW_ABORT,
                                    I18N("Importing SWIFT tags..."),
                                    NULL,
                                    AHB_SWIFT_Tag_List_GetCount(tl),
                                    0);

  tg=AHB_SWIFT_Tag_List_First(tl);
  while (tg) {
    const char *id;

    id=AHB_SWIFT_Tag_GetId(tg);
    assert(id);

    if (strcasecmp(id, "20")==0) {
      if (acceptTag20 || rejectTag20) {
        const char *p;

        p=AHB_SWIFT_Tag_GetData(tg);
        assert(p);
        if (rejectTag20) {
          if (-1!=GWEN_Text_ComparePattern(p, rejectTag20, 0)) {
            DBG_INFO(AQBANKING_LOGDOMAIN, "Ignoring report [%s]", p);
            ignoreCurrentReport=1;
          }
          else {
            ignoreCurrentReport=0;
          }
        }
        else if (acceptTag20) {
          if (-1==GWEN_Text_ComparePattern(p, acceptTag20, 0)) {
            DBG_INFO(AQBANKING_LOGDOMAIN,
                     "Ignoring report [%s] (not matching [%s])",
                     p, acceptTag20);
            ignoreCurrentReport=1;
          }
          else {
            ignoreCurrentReport=0;
          }
        }

      }
    }
    else {
      if (!ignoreCurrentReport) {
        if (strcasecmp(id, "25")==0) { /* LocalAccount */
          if (AHB_SWIFT940_Parse_25(tg, flags, dbTemplate, cfg)) {
            DBG_INFO(AQBANKING_LOGDOMAIN, "Error in tag");
            GWEN_DB_Group_free(dbTemplate);
            GWEN_Gui_ProgressEnd(progressId);
            return -1;
          }
        }
        else if (strcasecmp(id, "28C")==0) {
          /* Sequence/Statement Number - currently ignored */
          /* PostFinance statements don't have a correctly incrementing count... */
        }
        else if (strcasecmp(id, "60M")==0 || /* Interim StartSaldo */
                 strcasecmp(id, "60F")==0) { /* StartSaldo */
          GWEN_DB_NODE *dbSaldo;
          const char *curr;

          /* start a new day */
          dbDay=GWEN_DB_GetGroup(data, GWEN_PATH_FLAGS_CREATE_GROUP, "day");

          dbTransaction=0;
          DBG_INFO(AQBANKING_LOGDOMAIN, "Starting new day");
          if (strcasecmp(id, "60F")==0)
            dbSaldo=GWEN_DB_GetGroup(dbDay, GWEN_PATH_FLAGS_CREATE_GROUP, "StartSaldo");
          else
            dbSaldo=GWEN_DB_GetGroup(dbDay, GWEN_PATH_FLAGS_CREATE_GROUP, "InterimStartSaldo");
          GWEN_DB_AddGroupChildren(dbSaldo, dbTemplate);
          if (AHB_SWIFT940_Parse_60_62(tg, flags, dbSaldo, cfg)) {
            DBG_INFO(AQBANKING_LOGDOMAIN, "Error in tag");
            GWEN_DB_Group_free(dbTemplate);
            GWEN_Gui_ProgressEnd(progressId);
            return -1;
          }
          else {
            sDate=GWEN_DB_GetCharValue(dbSaldo, "date", 0, NULL);
            DBG_INFO(AQBANKING_LOGDOMAIN, "Storing date \"%s\" as default for maybe later", sDate?sDate:"(empty)");
          }

          curr=GWEN_DB_GetCharValue(dbSaldo, "value/currency", 0, 0);
          if (curr) {
            AHB_SWIFT_SetCharValue(dbTemplate, flags,
                                    "value/currency", curr);
          }
          if (strcasecmp(id, "60F")==0)
            GWEN_DB_SetCharValue(dbSaldo, GWEN_DB_FLAGS_OVERWRITE_VARS, "type", "final");
          else
            GWEN_DB_SetCharValue(dbSaldo, GWEN_DB_FLAGS_OVERWRITE_VARS, "type", "interim");

        }
        else if (strcasecmp(id, "62M")==0 || /* Interim EndSaldo */
                 strcasecmp(id, "62F")==0) { /* EndSaldo */
          GWEN_DB_NODE *dbSaldo;

          /* end current day */
          dbTransaction=0;
          if (!dbDay) {
            DBG_WARN(AQBANKING_LOGDOMAIN, "Your bank does not send an opening saldo");
            dbDay=GWEN_DB_GetGroup(data, GWEN_PATH_FLAGS_CREATE_GROUP, "day");
          }
          dbSaldo=GWEN_DB_GetGroup(dbDay, GWEN_PATH_FLAGS_CREATE_GROUP, "EndSaldo");
          GWEN_DB_AddGroupChildren(dbSaldo, dbTemplate);
          if (AHB_SWIFT940_Parse_60_62(tg, flags, dbSaldo, cfg)) {
            DBG_INFO(AQBANKING_LOGDOMAIN, "Error in tag");
            GWEN_DB_Group_free(dbTemplate);
            GWEN_Gui_ProgressEnd(progressId);
            return -1;
          }
          if (strcasecmp(id, "62F")==0)
            GWEN_DB_SetCharValue(dbSaldo, GWEN_DB_FLAGS_OVERWRITE_VARS, "type", "final");
          else
            GWEN_DB_SetCharValue(dbSaldo, GWEN_DB_FLAGS_OVERWRITE_VARS, "type", "interim");
          dbDay=0;

        }
        else if (strcasecmp(id, "61")==0) {
          if (!dbDay) {
            DBG_WARN(AQBANKING_LOGDOMAIN,
                     "Your bank does not send an opening saldo");
            dbDay=GWEN_DB_GetGroup(data, GWEN_PATH_FLAGS_CREATE_GROUP, "day");
          }

          DBG_INFO(AQBANKING_LOGDOMAIN, "Creating new transaction");
          dbTransaction=GWEN_DB_GetGroup(dbDay, GWEN_PATH_FLAGS_CREATE_GROUP,
                                         "transaction");
          GWEN_DB_AddGroupChildren(dbTransaction, dbTemplate);
          if (sDate && *sDate) {
            /* dbDate is set upon parsing of tag 60F, use it as a default
             * if possible */
            GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS, "date", sDate);
          }
          if (AHB_SWIFT940_Parse_61(tg, flags, dbTransaction, cfg)) {
            DBG_INFO(AQBANKING_LOGDOMAIN, "Error in tag");
            GWEN_DB_Group_free(dbTemplate);
            GWEN_Gui_ProgressEnd(progressId);
            return -1;
          }
        }
        else if (strcasecmp(id, "86")==0) {
          if (!dbTransaction) {
            DBG_WARN(AQBANKING_LOGDOMAIN,
                     "Bad sequence of tags (86 before 61), ignoring");
          }
          else {
            if (AHB_SWIFT940_Parse_86(tg, flags, dbTransaction, cfg)) {
              DBG_INFO(AQBANKING_LOGDOMAIN, "Error in tag");
              GWEN_DB_Group_free(dbTemplate);
              GWEN_Gui_ProgressEnd(progressId);
              return -1;
            }
          }
        }
        else if (strcasecmp(id, "NS")==0) {
          if (!dbTransaction) {
            DBG_DEBUG(AQBANKING_LOGDOMAIN,
                      "Ignoring NS tags outside transactions");
          }
          else {
            if (AHB_SWIFT940_Parse_NS(tg, flags, dbTransaction, cfg)) {
              DBG_INFO(AQBANKING_LOGDOMAIN, "Error in tag");
              GWEN_DB_Group_free(dbTemplate);
              GWEN_Gui_ProgressEnd(progressId);
              return -1;
            }
          }
        }
        else if (strcmp(id, "21")==0) {
          const char *p;

          p=AHB_SWIFT_Tag_GetData(tg);
          assert(p);
          if (0==strcmp(p, "NONREF")) {
            DBG_INFO(AQBANKING_LOGDOMAIN, "Ignoring related reference '%s' in document tag 21.", p);
          }
          else {
            DBG_WARN(AQBANKING_LOGDOMAIN, "Unexpected related reference '%s' in document tag 21 encountered.", p);
          }
        }
        else if (strcmp(id, "13")==0 ||  /* "Erstellungszeitpunkt */
                 strcmp(id, "34F")==0 || /* "Mindestbetrag" (sometimes contains some strange values) */
                 strcmp(id, "90D")==0 || /* "Anzahl und Summe Soll-Buchungen" (examples I've seen are invalid anyway) */
                 strcmp(id, "90C")==0) { /* "Anzahl und Summe Haben-Buchungen" (examples I've seen are invalid anyway) */
          /* ignore some well known tags */
          DBG_INFO(AQBANKING_LOGDOMAIN, "Ignoring well known tag \"%s\"", id);
        }
        else {
          DBG_WARN(AQBANKING_LOGDOMAIN,
                   "Unhandled tag '%s' found. "
                   "This only means the file contains info we currently don't read, "
                   "in most cases this is unimportant data.",
                   id);
          DBG_WARN(AQBANKING_LOGDOMAIN,
                   "To debug set environment variable AQBANKING_LOGLEVEL=info and rerun,"
                   "otherwise just ignore this message.");
        }

      }
    }

    if (GWEN_Gui_ProgressAdvance(progressId, GWEN_GUI_PROGRESS_ONE)==
        GWEN_ERROR_USER_ABORTED) {
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                           I18N("Aborted by user"));
      GWEN_Gui_ProgressEnd(progressId);
      GWEN_DB_Group_free(dbTemplate);
      return GWEN_ERROR_USER_ABORTED;
    }

    tg=AHB_SWIFT_Tag_List_Next(tg);
  } /* while */

  GWEN_DB_Group_free(dbTemplate);
  GWEN_Gui_ProgressEnd(progressId);

  return 0;
}



