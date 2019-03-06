/***************************************************************************
 begin       : Fri Apr 02 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "swift535_p.h"
#include "aqbanking/i18n_l.h"

/* #include <aqhbci/aqhbci.h> */
#include <aqbanking/error.h>
#include <aqbanking/backendsupport/imexporter_be.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/gui.h>

#include <ctype.h>



/* #define ENABLE_FULL_SEPA_LOG */


#define CENTURY_CUTOFF_YEAR 79



static void _iso8859_1ToUtf8(const char *p, int size, GWEN_BUFFER *buf)
{
  while (*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    if (c<32 || c==127)
      c=32;
    if (c & 0x80) {
      GWEN_Buffer_AppendByte(buf, 0xc0 | c>>6);
      c &= ~0x40;
    }
    GWEN_Buffer_AppendByte(buf, c);
    if (size!=-1)
      size--;
  } /* while */
}



int AHB_SWIFT__SetCharValue535(GWEN_DB_NODE *db,
                               uint32_t flags,
                               const char *name,
                               const char *s)
{
  GWEN_BUFFER *vbuf;
  int rv;

  vbuf=GWEN_Buffer_new(0, strlen(s)+32, 0, 1);
  _iso8859_1ToUtf8(s, -1, vbuf);
  rv=GWEN_DB_SetCharValue(db, flags, name, GWEN_Buffer_GetStart(vbuf));
  GWEN_Buffer_free(vbuf);
  return rv;
}



int AHB_SWIFT535_Parse_97A(const AHB_SWIFT_TAG *tg,
                           uint32_t flags,
                           GWEN_DB_NODE *data,
                           GWEN_DB_NODE *cfg)
{
  const char *p;
  const char *p2;

  p=AHB_SWIFT_Tag_GetData(tg);
  assert(p);

  while (*p && *p==32)
    p++;
  if (*p==0) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Tag 97A is empty");
    return 0;
  }

  p2=strchr(p, '/');
  if (p2) {
    char *s;

    /* "BLZ/Konto" */
    s=(char *)GWEN_Memory_malloc(p2-p+1);
    memmove(s, p, p2-p+1);
    s[p2-p]=0;
    AHB_SWIFT__SetCharValue535(data,
                               GWEN_DB_FLAGS_OVERWRITE_VARS,
                               "localBankCode", s);
    GWEN_Memory_dealloc(s);
    p=p2+1;
  }

  while (*p && *p==32)
    p++;

  if (*p) {
    p2=p;
    while (*p2 && isdigit(*p2))
      p2++;
    if (p2==p) {
      DBG_INFO(AQBANKING_LOGDOMAIN,
               "LocalAccountNumber starts with nondigits (%s)", p);
      AHB_SWIFT__SetCharValue535(data,
                                 GWEN_DB_FLAGS_OVERWRITE_VARS,
                                 "localAccountNumber", p);
    }
    else {
      char *s;

      s=(char *)GWEN_Memory_malloc(p2-p+1);
      memmove(s, p, p2-p+1);
      s[p2-p]=0;
      AHB_SWIFT__SetCharValue535(data,
                                 GWEN_DB_FLAGS_OVERWRITE_VARS,
                                 "localAccountNumber", s);
      GWEN_Memory_dealloc(s);
    }
  }
  return 0;
}


// get names / IDs of security
int AHB_SWIFT535_Parse_35B(const AHB_SWIFT_TAG *tg,
                           uint32_t flags,
                           GWEN_DB_NODE *data,
                           GWEN_DB_NODE *cfg)
{
  char *p, *s, *ss;
  int gotIsin=0;

  p=(char *)AHB_SWIFT_Tag_GetData(tg);
  assert(p);

  while (*p && *p==32)
    p++;
  if (*p==0) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Tag 35B is empty");
    return 0;
  }

  // get ISIN
  if (strncasecmp(p, "ISIN ", 5)==0) {
    p+=5;
    s=(char *)GWEN_Memory_malloc(1024);
    if (sscanf(p, " %s ", s)!=1) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Tag 35B: Cannot read ISIN");
      GWEN_Memory_dealloc(s);
      return 0;
    }
    p+=strlen(s);
    AHB_SWIFT__SetCharValue535(data, flags, "nameSpace", "ISIN");
    AHB_SWIFT__SetCharValue535(data, flags, "uniqueId", s);
    GWEN_Memory_dealloc(s);
    gotIsin=1;
  }

  // get WKN
  while (*p && *p<=32)
    p++;

  if (strncasecmp(p, "/DE/", 4)==0) {
    p+=4;
    s=(char *)GWEN_Memory_malloc(1024);
    if (sscanf(p, "%s", s)!=1) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Tag 35B: Cannot read WKN");
      GWEN_Memory_dealloc(s);
      return 0;
    }
    p+=strlen(s);
    if (!gotIsin) {
      AHB_SWIFT__SetCharValue535(data, flags, "nameSpace", "WKN");
      AHB_SWIFT__SetCharValue535(data, flags, "uniqueId", s);
    }
    GWEN_Memory_dealloc(s);
  }

  // get security name
  while (*p && *p<=32)
    p++;

  s=(char *)GWEN_Memory_malloc(1024);
  ss=s;
  while (*p) {
    if (*p>=32)
      *ss++ = *p;
    p++;
  }
  *ss=0;

  AHB_SWIFT__SetCharValue535(data, flags, "name", s);
  GWEN_Memory_dealloc(s);

  return 0;
}


// get price of security
int AHB_SWIFT535_Parse_90B(const AHB_SWIFT_TAG *tg,
                           uint32_t flags,
                           GWEN_DB_NODE *data,
                           GWEN_DB_NODE *cfg)
{
  char *p, *s;

  p=(char *)AHB_SWIFT_Tag_GetData(tg);
  assert(p);

  while (*p && *p==32)
    p++;
  if (*p==0) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Tag 90B is empty");
    return 0;
  }

  // get price
  if (strncasecmp(p, ":MRKT//ACTU/", 12)==0) {
    p+=12;
    s=(char *)GWEN_Memory_malloc(1024);
    if (sscanf(p, " %3s ", s)!=1) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Tag 90B: Cannot read currency");
      GWEN_Memory_dealloc(s);
      return 0;
    }
    p+=strlen(s);
    AHB_SWIFT__SetCharValue535(data, flags, "unitCurrency", s);

    if (sscanf(p, " %s ", s)!=1) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Tag 90B: Cannot read price");
      GWEN_Memory_dealloc(s);
      return 0;
    }
    /*p+=strlen(s); */
    AHB_SWIFT__SetCharValue535(data, flags, "unitPrice", s);

    GWEN_Memory_dealloc(s);
  }

  return 0;
}


// get date of security price
int AHB_SWIFT535_Parse_98A(const AHB_SWIFT_TAG *tg,
                           uint32_t flags,
                           GWEN_DB_NODE *data,
                           GWEN_DB_NODE *cfg)
{
  char *p;
  int year, month, day;
  GWEN_DATE *dt;

  p=(char *)AHB_SWIFT_Tag_GetData(tg);
  assert(p);

  while (*p && *p==32)
    p++;
  if (*p==0) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Tag 98A is empty");
    return 0;
  }

  // get date
  if (strncasecmp(p, ":PRIC//", 7)==0) {
    p+=7;
    if (sscanf(p, "%4d%2d%2d", &year, &month, &day)!=3) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Tag 98A: Cannot read date");
      return 0;
    }
    dt=GWEN_Date_fromGregorian(year, month, day);
    assert(dt);

    GWEN_DB_SetCharValue(data, GWEN_DB_FLAGS_DEFAULT, "unitPriceDate", GWEN_Date_GetString(dt));
    GWEN_Date_free(dt);
  }

  return 0;
}


// get units of security
int AHB_SWIFT535_Parse_93B(const AHB_SWIFT_TAG *tg,
                           uint32_t flags,
                           GWEN_DB_NODE *data,
                           GWEN_DB_NODE *cfg)
{
  char *p;

  p=(char *)AHB_SWIFT_Tag_GetData(tg);
  assert(p);

  while (*p && *p==32)
    p++;
  if (*p==0) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Tag 93B is empty");
    return 0;
  }

  // get units
  if (strncasecmp(p, ":AGGR//UNIT/", 12)==0) {
    p+=12;
    AHB_SWIFT__SetCharValue535(data, flags, "units", p);
  }

  return 0;
}



/* Import SWIFT MT535 data.
   @param tl input: list of tags. Tags are lines in a SWIFT data block (block 4). A tag has an
          id and content. See the AHB_SWIFT_Tag_new function for more information.
 */
int AHB_SWIFT535_Import(AHB_SWIFT_TAG_LIST *tl,
                        GWEN_DB_NODE *data,
                        GWEN_DB_NODE *cfg,
                        uint32_t flags)
{
  AHB_SWIFT_TAG *tg;
  GWEN_DB_NODE *dbTemplate=NULL;
  GWEN_DB_NODE *dbSecurity=NULL;
  uint32_t progressId;
  int docLvl;

  dbTemplate=GWEN_DB_Group_new("template");

  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                                    GWEN_GUI_PROGRESS_ALLOW_EMBED |
                                    GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                                    GWEN_GUI_PROGRESS_SHOW_ABORT,
                                    I18N("Importing SWIFT tags..."),
                                    NULL,
                                    AHB_SWIFT_Tag_List_GetCount(tl),
                                    0);


  docLvl=AHB_SWIFT535_LEVEL_TOP;

  tg=AHB_SWIFT_Tag_List_First(tl);
  while (tg) {
    const char *id;
    const char *da;

    id=AHB_SWIFT_Tag_GetId(tg);
    da=AHB_SWIFT_Tag_GetData(tg);
    assert(id);
    assert(da);

    switch (docLvl) {
    case AHB_SWIFT535_LEVEL_TOP:
      if (strcasecmp(id, "16R")==0) {
        if (strcasecmp(da, "GENL")==0)
          docLvl=AHB_SWIFT535_LEVEL_GENL;
        //  else if (strcasecmp(da, "SUBSAFE")==0)
        //    docLvl=AHB_SWIFT535_LEVEL_SUBSAFE;
        else if (strcasecmp(da, "FIN")==0) {
          docLvl=AHB_SWIFT535_LEVEL_FIN;
          dbSecurity=GWEN_DB_GetGroup(data, GWEN_PATH_FLAGS_CREATE_GROUP, "security");
        }
        else if (strcasecmp(da, "ADDINFO")==0)
          docLvl=AHB_SWIFT535_LEVEL_ADDINFO;
        else
          DBG_INFO(AQBANKING_LOGDOMAIN, "TOP: Ignoring tag :%s:%s", id, da);
      }
      break;

    case AHB_SWIFT535_LEVEL_GENL:
      if (strcasecmp(id, "16S")==0 && strcasecmp(da, "GENL")==0)
        docLvl=AHB_SWIFT535_LEVEL_TOP;

      else if (strcasecmp(id, "97A")==0) { /* LocalAccount */
        if (AHB_SWIFT535_Parse_97A(tg, flags, data, cfg)) {
          DBG_WARN(AQBANKING_LOGDOMAIN, "Error in tag %s", id);
          GWEN_Gui_ProgressEnd(progressId);
          return -1;
        }
      }

      else
        DBG_INFO(AQBANKING_LOGDOMAIN, "GENL: Ignoring tag :%s:%s", id, da);
      break;

    case AHB_SWIFT535_LEVEL_SUBSAFE:
      if (strcasecmp(id, "16S")==0 && strcasecmp(da, "SUBSAFE")==0)
        docLvl=AHB_SWIFT535_LEVEL_TOP;
      else
        DBG_INFO(AQBANKING_LOGDOMAIN, "SUBSAFE: Ignoring tag :%s:%s", id, da);
      break;

    case AHB_SWIFT535_LEVEL_FIN:
      if (strcasecmp(id, "16S")==0 && strcasecmp(da, "FIN")==0)
        //  docLvl=AHB_SWIFT535_LEVEL_SUBSAFE;
        docLvl=AHB_SWIFT535_LEVEL_TOP;

      else if (strcasecmp(id, "16R")==0 && strcasecmp(da, "SUBBAL")==0)
        docLvl=AHB_SWIFT535_LEVEL_SUBBAL;

      else if (strcasecmp(id, "35B")==0) { /* name of security */
        if (AHB_SWIFT535_Parse_35B(tg, flags, dbSecurity, cfg)) {
          DBG_WARN(AQBANKING_LOGDOMAIN, "Error in tag %s", id);
          GWEN_Gui_ProgressEnd(progressId);
          return -1;
        }
      }

      else if (strcasecmp(id, "90B")==0) { /* price of security */
        if (AHB_SWIFT535_Parse_90B(tg, flags, dbSecurity, cfg)) {
          DBG_WARN(AQBANKING_LOGDOMAIN, "Error in tag %s", id);
          GWEN_Gui_ProgressEnd(progressId);
          return -1;
        }
      }

      else if (strcasecmp(id, "98A")==0) { /* date of security price*/
        if (AHB_SWIFT535_Parse_98A(tg, flags, dbSecurity, cfg)) {
          DBG_WARN(AQBANKING_LOGDOMAIN, "Error in tag %s", id);
          GWEN_Gui_ProgressEnd(progressId);
          return -1;
        }
      }

      else if (strcasecmp(id, "93B")==0) { /* units of security */
        if (AHB_SWIFT535_Parse_93B(tg, flags, dbSecurity, cfg)) {
          DBG_WARN(AQBANKING_LOGDOMAIN, "Error in tag %s", id);
          GWEN_Gui_ProgressEnd(progressId);
          return -1;
        }
      }

      else
        DBG_INFO(AQBANKING_LOGDOMAIN, "FIN: Ignoring tag :%s:%s", id, da);
      break;

    case AHB_SWIFT535_LEVEL_SUBBAL:
      if (strcasecmp(id, "16S")==0 && strcasecmp(da, "SUBBAL")==0)
        docLvl=AHB_SWIFT535_LEVEL_FIN;
      else
        DBG_INFO(AQBANKING_LOGDOMAIN, "SUBBAL: Ignoring tag :%s:%s", id, da);
      break;

    case AHB_SWIFT535_LEVEL_ADDINFO:
      if (strcasecmp(id, "16S")==0 && strcasecmp(da, "ADDINFO")==0)
        docLvl=AHB_SWIFT535_LEVEL_TOP;
      else
        DBG_INFO(AQBANKING_LOGDOMAIN, "ADDINFO: Ignoring tag :%s:%s", id, da);
      break;

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

  if (docLvl!=AHB_SWIFT535_LEVEL_TOP)
    DBG_WARN(AQBANKING_LOGDOMAIN, "Illegal document structure");
  GWEN_DB_Group_free(dbTemplate);
  GWEN_Gui_ProgressEnd(progressId);

  return 0;
}
