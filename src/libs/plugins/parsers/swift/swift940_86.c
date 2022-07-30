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

#include "swift940_86.h"


#include <aqbanking/error.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <ctype.h>




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void _extractAndHandleSepaTags(GWEN_DB_NODE *dbData, uint32_t flags);
static void _transformPurposeIntoOneString(GWEN_DB_NODE *dbData, uint32_t flags);
static void _readSubTagsIntoDb(AHB_SWIFT_SUBTAG_LIST *stlist, GWEN_DB_NODE *dbData, uint32_t flags);
static int _readSepaTags(const char *sPurpose, GWEN_DB_NODE *dbSepaTags);
static int _storeSepaTag(const char *sTagStart, int tagLen, GWEN_DB_NODE *dbSepaTags);
static void _transformSepaTags(GWEN_DB_NODE *dbData, GWEN_DB_NODE *dbSepaTags, uint32_t flags);
static void _parseTransactionData(const char *p, GWEN_DB_NODE *dbData, uint32_t flags);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AHB_SWIFT940_Parse_86(const AHB_SWIFT_TAG *tg, uint32_t flags, GWEN_DB_NODE *dbData, GWEN_DB_NODE *cfg)
{
  const char *p;
  int isStructured;
  int code;
  int keepMultipleBlanks;

  keepMultipleBlanks=GWEN_DB_GetIntValue(cfg, "keepMultipleBlanks", 0, 1);
  p=AHB_SWIFT_Tag_GetData(tg);
  assert(p);
  isStructured=0;
  code=999;
  if (strlen(p)>3) {
    if (isdigit(p[0]) && isdigit(p[1]) && isdigit(p[2])) {
      /* starts with a three digit number */
      code=(((p[0]-'0')*100) + ((p[1]-'0')*10) + (p[2]-'0'));
      if (p[3]=='?')
        /* it is structured, get the code */
        isStructured=1;
      p+=3;
    }
  }

  if (isStructured) {
    AHB_SWIFT_SUBTAG_LIST *stlist;
    int rv;

    /* store code */
    GWEN_DB_SetIntValue(dbData, flags, "transactioncode", code);

    stlist=AHB_SWIFT_SubTag_List_new();
    rv=AHB_SWIFT_ParseSubTags(p, stlist, keepMultipleBlanks);
    if (rv<0) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Handling tag :86: as unstructured (%d)", rv);
      isStructured=0;
    }
    else {
      if (code<900) {
        /* sepa */
        DBG_INFO(AQBANKING_LOGDOMAIN, "Reading as SEPA tag (%d)", code);
        _readSubTagsIntoDb(stlist, dbData, flags);
        _extractAndHandleSepaTags(dbData, flags);
        _transformPurposeIntoOneString(dbData, flags);
      }
      else {
        /* non-sepa */
        DBG_INFO(AQBANKING_LOGDOMAIN, "Reading as non-SEPA tag (%d)", code);
        _readSubTagsIntoDb(stlist, dbData, flags);
        _transformPurposeIntoOneString(dbData, flags);
      }
    } /* if really structured */
    AHB_SWIFT_SubTag_List_free(stlist);
  } /* if isStructured */
  else {
    /* unstructured :86:, simply store as mutliple purpose lines */
    _parseTransactionData(p, dbData, flags);
  }

  return 0;
}



void _readSubTagsIntoDb(AHB_SWIFT_SUBTAG_LIST *stlist, GWEN_DB_NODE *dbData, uint32_t flags)
{
  AHB_SWIFT_SUBTAG *stg;

  stg=AHB_SWIFT_SubTag_List_First(stlist);
  while (stg) {
    const char *s;
    int id;
    int intVal;

    id=AHB_SWIFT_SubTag_GetId(stg);
    s=AHB_SWIFT_SubTag_GetData(stg);
    switch (id) {
    case 0: /* Buchungstext */
      AHB_SWIFT_SetCharValue(dbData, flags, "transactionText", s);
      break;
    case 10: /* Primanota */
      AHB_SWIFT_SetCharValue(dbData, flags, "primanota", s);
      break;

    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 60:
    case 61:
    case 62:
    case 63: /* Verwendungszweck */
      AHB_SWIFT_SetCharValue(dbData, flags, "purpose", s);
      break;

    case 30: /* BLZ Gegenseite */
      AHB_SWIFT_SetCharValue(dbData, flags, "remoteBankCode", s);
      break;

    case 31: /* Kontonummer Gegenseite */
      AHB_SWIFT_SetCharValue(dbData, flags, "remoteAccountNumber", s);
      break;

    case 32:
    case 33: /* Name Auftraggeber */
      //DBG_ERROR(AQBANKING_LOGDOMAIN, "Setting remote name: [%s]", s);
      AHB_SWIFT_SetCharValue(dbData, flags, "remoteName", s);
      break;

    case 34: /* Textschluesselergaenzung */
      if (1==sscanf(s, "%d", &intVal)) {
        GWEN_DB_SetIntValue(dbData, flags, "textkeyExt", intVal);
      }
      else {
        DBG_WARN(AQBANKING_LOGDOMAIN, "Value [%s] is not a number (textkeyext)", s);
      }
      break;

    case 38: /* IBAN */
      AHB_SWIFT_SetCharValue(dbData, flags, "remoteIban", s);
      break;

    default: /* ignore all other fields (if any) */
      DBG_WARN(AQBANKING_LOGDOMAIN, "Unknown :86: field \"%02d\" (%s)", id, s);
      break;
    } /* switch */
    stg=AHB_SWIFT_SubTag_List_Next(stg);
  } /* while */
}



void _extractAndHandleSepaTags(GWEN_DB_NODE *dbData, uint32_t flags)
{
  GWEN_BUFFER *tbuf;
  int i;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  for (i=0; i<99; i++) {
    const char *s;

    s=GWEN_DB_GetCharValue(dbData, "purpose", i, 0);
    if (s && *s)
      GWEN_Buffer_AppendString(tbuf, s);
  }

  if (GWEN_Buffer_GetUsedBytes(tbuf)) {
    GWEN_DB_NODE *dbSepaTags;
    int realSepaTagCount;

    dbSepaTags=GWEN_DB_Group_new("sepa-tags");
    realSepaTagCount=_readSepaTags(GWEN_Buffer_GetStart(tbuf), dbSepaTags);

    if (realSepaTagCount>0 && GWEN_DB_Variables_Count(dbSepaTags))
      _transformSepaTags(dbData, dbSepaTags, flags);

    GWEN_DB_Group_free(dbSepaTags);
  }

  /* buffer no longer needed */
  GWEN_Buffer_free(tbuf);
}



void _transformPurposeIntoOneString(GWEN_DB_NODE *dbData, uint32_t flags)
{
  GWEN_BUFFER *tbuf;
  int i;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  for (i=0; i<99; i++) {
    const char *s;

    s=GWEN_DB_GetCharValue(dbData, "purpose", i, 0);
    if (s && *s) {
      if (GWEN_Buffer_GetUsedBytes(tbuf))
        GWEN_Buffer_AppendString(tbuf, "\n");
      GWEN_Buffer_AppendString(tbuf, s);
    }
  }

  if (GWEN_Buffer_GetUsedBytes(tbuf)) {
    GWEN_DB_DeleteVar(dbData, "purpose");
    GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_DEFAULT, "purpose", GWEN_Buffer_GetStart(tbuf));
  }
  GWEN_Buffer_free(tbuf);
}



int _readSepaTags(const char *sPurpose, GWEN_DB_NODE *dbSepaTags)
{
  const char *s;
  const char *sLastTagStart;
  int realSepaTagCount=0;

#ifdef ENABLE_FULL_SEPA_LOG
  DBG_ERROR(AQHBCI_LOGDOMAIN, "FullPurposeBuffer");
  GWEN_Buffer_Dump(bufFullPurpose, 2);
#endif

  s=sPurpose;
  sLastTagStart=s;

  /* sample all SEPA fields from concatenated string of purpose lines */
  while (*s) {
    /* look for begin of next tag */
    while (*s) {
      if ((*s && isalpha(*s)) &&
          (s[1] && isalpha(s[1])) &&
          (s[2] && isalpha(s[2])) &&
          (s[3] && isalpha(s[3])) &&
          s[4]=='+') {
        if (strncasecmp(s, "EREF+", 5)==0 ||
            strncasecmp(s, "KREF+", 5)==0 ||
            strncasecmp(s, "MREF+", 5)==0 ||
            strncasecmp(s, "CRED+", 5)==0 ||
            strncasecmp(s, "DEBT+", 5)==0 ||
            strncasecmp(s, "SVWZ+", 5)==0 ||
            strncasecmp(s, "ABWA+", 5)==0 ||
            strncasecmp(s, "ABWE+", 5)==0)
          break;
      }
      /* not the beginning of a SEPA field, just skip */
      s++;
    }

    /* found begin of the next SEPA field or end of buffer */
    if (s > sLastTagStart) {
      int tagLen;

      /* we currently have a field, close that first */
      tagLen=s-sLastTagStart;

      if (_storeSepaTag(sLastTagStart, tagLen, dbSepaTags)>0)
        realSepaTagCount++;
    }

    if (*s) {
      /* save start of next tag */
      sLastTagStart=s;
      /* skip XXX+ at the beginning, otherwise we would immediately stop in next loop
       * we know that the next 5 bytes are valid, so it is safe to skip them */
      s+=5;
    }
  } /* while */

  return realSepaTagCount;
}



int _storeSepaTag(const char *sTagStart, int tagLen, GWEN_DB_NODE *dbSepaTags)
{
  int isRealSepaTag=0;

#ifdef ENABLE_FULL_SEPA_LOG
  DBG_ERROR(0, "Current tag:");
  GWEN_Text_LogString(sTagStart, tagLen, 0, GWEN_LoggerLevel_Error);
#endif

  /* check tag length (must be long enough for 'XXX+', i.e. at least 5 bytes) */
  if (tagLen>5 && sTagStart[4]=='+') {
    char sIdentifier[6];
    const char *sPayload;

    /* ok, 5 bytes or more, 4 alphas and a plus sign, should be the begin of a SEPA tag */
    strncpy(sIdentifier, sTagStart, 5);
    sIdentifier[5]=0;

    /* remove leading blanks */
    sPayload=sTagStart+5;
    tagLen-=5;
    while (tagLen>0 && *sPayload && isblank(*sPayload)) {
      sPayload++;
      tagLen--;
    }

    /* remove trailing blanks */
    if (tagLen>0) {
      while (tagLen>0) {
        if (!isblank(sPayload[tagLen-1]))
          break;
        tagLen--;
      }
    }

    /* store tag, if still data left */
    if (tagLen>0) {
      char *sCopyPayload;

      sCopyPayload=GWEN_Text_strndup(sPayload, tagLen);
      GWEN_DB_SetCharValue(dbSepaTags, GWEN_DB_FLAGS_DEFAULT, sIdentifier, sCopyPayload);
      free(sCopyPayload);
      isRealSepaTag=1;
    }
    else {
      DBG_WARN(GWEN_LOGDOMAIN, "Ignoring empty SEPA field \"%s\"", sIdentifier);
    }
  }
  else {
    /* tag is shorter than 5 bytes or pos 4 doesn't contain a plus, treat as normal purpose */
    if (tagLen>0) {
      char *sCopyPayload;

      sCopyPayload=GWEN_Text_strndup(sTagStart, tagLen);
      GWEN_DB_SetCharValue(dbSepaTags, GWEN_DB_FLAGS_DEFAULT, "_purpose", sCopyPayload);
      free(sCopyPayload);
    }
  }

  return isRealSepaTag?1:0;
}



void _transformSepaTags(GWEN_DB_NODE *dbData, GWEN_DB_NODE *dbSepaTags, uint32_t flags)
{
  GWEN_DB_NODE *dbVar;

#ifdef ENABLE_FULL_SEPA_LOG
  DBG_ERROR(0, "Got these SEPA tags:");
  GWEN_DB_Dump(dbSepaTags, 2);
#endif

  /* clear purpose variable, since we are about to add it back from SEPA tags */
  GWEN_DB_DeleteVar(dbData, "purpose");

  dbVar=GWEN_DB_GetFirstVar(dbSepaTags);
  while (dbVar) {
    const char *sVarName;

    sVarName=GWEN_DB_VariableName(dbVar);
    if (sVarName && *sVarName) {
      GWEN_BUFFER *tbuf;
      GWEN_DB_NODE *dbValue;

      /* sample all values into a buffer and concatenate */
      tbuf=GWEN_Buffer_new(0, 128, 0, 1);
      dbValue=GWEN_DB_GetFirstValue(dbVar);
      while (dbValue) {
        const char *s;

        s=GWEN_DB_GetCharValueFromNode(dbValue);
        if (s && *s)
          GWEN_Buffer_AppendString(tbuf, s);

        dbValue=GWEN_DB_GetNextValue(dbValue);
      }

      if (strcasecmp(sVarName, "EREF+")==0) {
        AHB_SWIFT_SetCharValue(dbData, flags, "endToEndReference", GWEN_Buffer_GetStart(tbuf));
      }
      else if (strcasecmp(sVarName, "KREF+")==0) {
        AHB_SWIFT_SetCharValue(dbData, flags, "customerReference", GWEN_Buffer_GetStart(tbuf));
      }
      else if (strcasecmp(sVarName, "MREF+")==0) {
        AHB_SWIFT_SetCharValue(dbData, flags, "mandateId", GWEN_Buffer_GetStart(tbuf));
      }
      else if (strcasecmp(sVarName, "CRED+")==0) {
        AHB_SWIFT_SetCharValue(dbData, flags, "creditorSchemeId", GWEN_Buffer_GetStart(tbuf));
      }
      else if (strcasecmp(sVarName, "DEBT+")==0) {
        AHB_SWIFT_SetCharValue(dbData, flags, "originatorId", GWEN_Buffer_GetStart(tbuf));
      }
      else if (strcasecmp(sVarName, "SVWZ+")==0) {
        AHB_SWIFT_SetCharValue(dbData, flags | GWEN_DB_FLAGS_OVERWRITE_VARS, "purpose", GWEN_Buffer_GetStart(tbuf));
      }
      else if (strcasecmp(sVarName, "ABWA+")==0) {
        /* "abweichender Auftraggeber" */
        AHB_SWIFT_SetCharValue(dbData, flags, "ultimateDebtor", GWEN_Buffer_GetStart(tbuf));
      }
      else if (strcasecmp(sVarName, "ABWE+")==0) {
        /* "abweichender Empfaenger" */
        AHB_SWIFT_SetCharValue(dbData, flags, "ultimateCreditor", GWEN_Buffer_GetStart(tbuf));
      }
      else if (strcasecmp(sVarName, "_purpose")==0) {
        /* manually added tag (i.e. data outside a tag)
        * will be replaced if there was a real purpose field (i.e. "SVWZ+") */
        AHB_SWIFT_SetCharValue(dbData, flags, "purpose", GWEN_Buffer_GetStart(tbuf));
      }
      GWEN_Buffer_free(tbuf);
    }

    dbVar=GWEN_DB_GetNextVar(dbVar);
  }
}



void _parseTransactionData(const char *p, GWEN_DB_NODE *dbData, uint32_t flags)
{
  char *pcopy=strdup(p);
  char *p1;

  /* unstructured :86:, simply store as mutliple purpose lines */
  p1=pcopy;
  while (p1 && *p1) {
    char *p2;

    p2=strchr(p1, 10);
    if (p2) {
      *p2=0;
      p2++;
    }

    /* look for pattern "KTO/BLZ", if found try to extract remote account info
     * from unstructured purpose string */
    if (-1!=GWEN_Text_ComparePattern(p1, "*KTO/BLZ */*", 0)) {
      char *p3;
      char *kto;

      p3=p1;
      while (*p3) {
        *p3=toupper(*p3);
        p3++;
      }
      kto=strstr(p1, "KTO/BLZ ");
      if (kto) {
        char *blz;

        kto+=8;
        blz=strchr(kto, '/');
        if (blz) {
          *blz=0;
          blz++;

          p3=blz;
          while (*p3 && isdigit(*p3))
            p3++;
          *p3=0;

          AHB_SWIFT_SetCharValue(dbData, flags, "remoteBankCode", blz);
          AHB_SWIFT_SetCharValue(dbData, flags, "remoteAccountNumber", kto);
        }
      }
      else {
        AHB_SWIFT_SetCharValue(dbData, flags, "purpose", p1);
      }
    }
    else
      AHB_SWIFT_SetCharValue(dbData, flags, "purpose", p1);
    p1=p2;
  }
  free(pcopy);
}



