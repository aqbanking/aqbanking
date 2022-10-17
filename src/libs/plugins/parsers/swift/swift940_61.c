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

#include "swift940_61.h"


#include <aqbanking/error.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <ctype.h>




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _readValutaAndBookingDate(const char **pCurrentChar, unsigned int *pBytesLeft, GWEN_DB_NODE *data);
static int _readAmount(const char **pCurrentChar, unsigned int *pBytesLeft, GWEN_DB_NODE *data, GWEN_DB_NODE *cfg);
static int _readBookingKey(const char **pCurrentChar, unsigned int *pBytesLeft, GWEN_DB_NODE *data);
static int _tryReadExtraAmountTag(const char **pCurrentChar, unsigned int *pBytesLeft,
                                  const char *tagName,
                                  GWEN_DB_NODE *data, const char *groupName);
static int _readAmountFromExtraLine(const char **pCurrentChar, unsigned int *pBytesLeft, GWEN_DB_NODE *data, const char *groupName);

static int _readTextAfterDoubleSlashesReturnLength(const char **pCurrentChar, unsigned int *pBytesLeft,
                                                   GWEN_DB_NODE *data, const char *varName, uint32_t flags);
static int _readTextUntilSlashOrEndReturnLength(const char **pCurrentChar, unsigned int *pBytesLeft,
                                                GWEN_DB_NODE *data, const char *varName, uint32_t flags);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


int AHB_SWIFT940_Parse_61(const AHB_SWIFT_TAG *tg, uint32_t flags, GWEN_DB_NODE *data, GWEN_DB_NODE *cfg)
{
  const char *p;
  unsigned int bleft;
  int rv;
  const char *s;
  int readExtraData61=0;

  s=GWEN_DB_GetCharValue(cfg, "readExtraData61", 0, "no");
  if (s && *s)
    readExtraData61=(strcasecmp(s, "yes")==0)?1:0;

  p=AHB_SWIFT_Tag_GetData(tg);
  assert(p);
  bleft=strlen(p);

  rv=_readValutaAndBookingDate(&p, &bleft, data);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_readAmount(&p, &bleft, data, cfg);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (bleft>0) {
    /* skip 'N' */
    p++;
    bleft--;
  }

  rv=_readBookingKey(&p, &bleft, data);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* customer reference (M) */
  rv=_readTextUntilSlashOrEndReturnLength(&p, &bleft, data, "customerReference", flags);
  if (rv>0) {
    const char *s;

    s=GWEN_DB_GetCharValue(data, "customerReference", 0, NULL);
    if (s && strcasecmp(s, "NONREF")==0)
      GWEN_DB_DeleteVar(data, "customerReference");
  }
  else if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d) [%s]", rv, p);
    return rv;
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Missing customer reference (%s), ignoring", p);
  }

  /* bank reference (K) */
  rv=_readTextAfterDoubleSlashesReturnLength(&p, &bleft, data, "bankReference", flags);
  if (rv<0) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Error reading bank reference (%s)", p);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, "SWIFT: Error reading bank reference");
  }

  /* more information ? */
  if (*p==10) {
    /* yes... */
    p++;
    bleft--;

    while (*p) {
      /* read extra information */
      rv=_tryReadExtraAmountTag(&p, &bleft, "/OCMT/", data, "origvalue");
      if (rv<0) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
      if (rv<1) { /* no OCMT, try next */
        rv=_tryReadExtraAmountTag(&p, &bleft, "/CHGS/", data, "fees");
        if (rv<0) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
          return rv;
        }
      }
      if (rv<1) { /* no CHGS, try next */
	if (readExtraData61) {
	  /* add extra data to purpose lines */
	  AHB_SWIFT_SetCharValue(data, GWEN_DB_FLAGS_DEFAULT, "purpose", p);
	  return 0;
	}
	else {
	  /* we should read the remainder of the line because that might contain important data
	   * for non-German users (see example file in bug #262), but where to store? */
	  DBG_WARN(AQBANKING_LOGDOMAIN, "Unknown/unstructured extra data, ignoring for now (%s)", p);
	  /* probably skip "/" if any */
	  return 0;
	}
      }
    } /* while */
  } /* if there is extra data */

  return 0;
}



int _readValutaAndBookingDate(const char **pCurrentChar, unsigned int *pBytesLeft, GWEN_DB_NODE *data)
{
  const char *p;
  unsigned int bleft;
  GWEN_DATE *valutaDate;
  GWEN_DATE *bookingDate;

  p=*pCurrentChar;
  bleft=*pBytesLeft;

  valutaDate=AHB_SWIFT_ReadDateYYMMDD(&p, &bleft);
  if (valutaDate==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or invalid valuta date (%s)", p);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, "SWIFT: Missing or invalid valuta date");
    return GWEN_ERROR_GENERIC;
  }

  bookingDate=AHB_SWIFT_ReadDateMMDDWithReference(&p, &bleft, valutaDate);
  if (bookingDate==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No or bad booking date in transaction (%s), ignoring", p);
  }

  GWEN_DB_SetCharValue(data, GWEN_DB_FLAGS_OVERWRITE_VARS, "valutaDate", GWEN_Date_GetString(valutaDate));
  if (bookingDate)
    GWEN_DB_SetCharValue(data, GWEN_DB_FLAGS_OVERWRITE_VARS, "date", GWEN_Date_GetString(bookingDate));

  GWEN_Date_free(bookingDate);
  GWEN_Date_free(valutaDate);

  *pCurrentChar=p;
  *pBytesLeft=bleft;
  return 0;
}



int _readAmount(const char **pCurrentChar, unsigned int *pBytesLeft, GWEN_DB_NODE *data, GWEN_DB_NODE *cfg)
{
  const char *p;
  unsigned int bleft;
  int neg;
  const char *p2;
  char *s;

  p=*pCurrentChar;
  bleft=*pBytesLeft;

  /* credit/debit mark (M) */
  if (bleft<2) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad value string (%s)", p);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, "SWIFT: Bad value string");
    return GWEN_ERROR_GENERIC;
  }
  neg=0;
  if (*p=='R') {
    if (p[1]=='C' || p[1]=='c')
      neg=1;
    p+=2;
    bleft-=2;
  }
  else {
    if (*p=='D' || *p=='d')
      neg=1;
    p++;
    bleft--;
  }

  /* third character of currency (K) */
  if (bleft<1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad data (%s)", p);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, "SWIFT: Bad currency");
    return GWEN_ERROR_GENERIC;
  }
  if (!isdigit(*p)) {
    /* found third character, skip it */
    p++;
    bleft--;
  }

  /* value (M) */
  p2=p;
  while (*p2 && (isdigit(*p2) || *p2==','))
    p2++;
  if (p2==p) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No value (%s)", p);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, "SWIFT: Bad value");
    return GWEN_ERROR_GENERIC;
  }
  s=(char *)GWEN_Memory_malloc(p2-p+1+(neg?1:0));
  if (neg) {
    s[0]='-';
    memmove(s+1, p, p2-p+1);
    s[p2-p+1]=0;
  }
  else {
    memmove(s, p, p2-p+1);
    s[p2-p]=0;
  }

  if (1) {
    const char *cu;

    AHB_SWIFT_SetCharValue(data, GWEN_DB_FLAGS_OVERWRITE_VARS, "value/value", s);

    cu=GWEN_DB_GetCharValue(cfg, "currency", 0, 0);
    if (cu && *cu)
      AHB_SWIFT_SetCharValue(data, GWEN_DB_FLAGS_OVERWRITE_VARS, "value/currency", cu);
  }
  GWEN_Memory_dealloc(s);
  bleft-=p2-p;
  p=p2;

  *pCurrentChar=p;
  *pBytesLeft=bleft;
  return 0;
}



int _readBookingKey(const char **pCurrentChar, unsigned int *pBytesLeft, GWEN_DB_NODE *data)
{
  const char *p;
  unsigned int bleft;
  char buffer[32];

  p=*pCurrentChar;
  bleft=*pBytesLeft;

  /* key (M) */
  if (bleft<3) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing booking key (%s)", p);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, "SWIFT: Missing booking key");
    return GWEN_ERROR_GENERIC;
  }
  memmove(buffer, p, 3);
  buffer[3]=0;
  AHB_SWIFT_SetCharValue(data, GWEN_DB_FLAGS_OVERWRITE_VARS, "transactionKey", buffer);
  p+=3;
  bleft-=3;

  *pCurrentChar=p;
  *pBytesLeft=bleft;
  return 0;
}



int _tryReadExtraAmountTag(const char **pCurrentChar, unsigned int *pBytesLeft,
                           const char *tagName,
                           GWEN_DB_NODE *data, const char *groupName)
{
  const char *p;
  unsigned int bleft;
  int tlength;

  tlength=strlen(tagName);

  p=*pCurrentChar;
  bleft=*pBytesLeft;

  if (bleft>=tlength && strncasecmp(p, tagName, tlength)==0) {
    int rv;

    /* original value */
    p+=tlength;
    bleft-=tlength;

    rv=_readAmountFromExtraLine(&p, &bleft, data, groupName);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    *pCurrentChar=p;
    *pBytesLeft=bleft;
    return 1;
  }
  return 0;
}



int _readAmountFromExtraLine(const char **pCurrentChar, unsigned int *pBytesLeft, GWEN_DB_NODE *data, const char *groupName)
{
  const char *p;
  unsigned int bleft;

  p=*pCurrentChar;
  bleft=*pBytesLeft;

  if (bleft>0) {
    const char *p2;
    char *s;
    char buffer[32];
    GWEN_DB_NODE *dbGroup;

    dbGroup=GWEN_DB_GetGroup(data, GWEN_DB_FLAGS_OVERWRITE_GROUPS, groupName);
    assert(dbGroup);

    /* get currency */
    memmove(buffer, p, 3);
    buffer[3]=0;
    AHB_SWIFT_SetCharValue(dbGroup, GWEN_DB_FLAGS_OVERWRITE_VARS, "currency", buffer);
    p+=3;
    bleft-=3;
    if (*p=='/') { /* Deutsche Bank seems to be sending */
      p++;         /* a "/" between currency and amount */
      bleft--;
    }
    /* get value */
    p2=p;
    while (*p2 && *p2!='/')
      p2++;
    if (p2==p) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad charges value (%s)", p);
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, "SWIFT: Bad charges value");
      return GWEN_ERROR_GENERIC;
    }
    s=(char *)GWEN_Memory_malloc(p2-p+1);
    memmove(s, p, p2-p+1);
    s[p2-p]=0;
    AHB_SWIFT_SetCharValue(dbGroup, GWEN_DB_FLAGS_OVERWRITE_VARS, "value", s);
    GWEN_Memory_dealloc(s);
    bleft-=p2-p;
    p=p2;

    *pCurrentChar=p;
    *pBytesLeft=bleft;
    return 0;
  }
  return GWEN_ERROR_BAD_DATA;
}



int _readTextAfterDoubleSlashesReturnLength(const char **pCurrentChar, unsigned int *pBytesLeft,
                                            GWEN_DB_NODE *data, const char *varName, uint32_t flags)
{
  const char *p;
  unsigned int bleft;

  p=*pCurrentChar;
  bleft=*pBytesLeft;

  if (bleft>1) {
    if (*p=='/' && p[1]=='/') {
      int rv;

      p+=2;
      bleft-=2;

      rv=_readTextUntilSlashOrEndReturnLength(&p, &bleft, data, varName, flags);
      *pCurrentChar=p;
      *pBytesLeft=bleft;
      if (rv<0) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
      return rv;
    }
  }

  /* no data */
  return 0;
}



int _readTextUntilSlashOrEndReturnLength(const char **pCurrentChar, unsigned int *pBytesLeft,
                                         GWEN_DB_NODE *data, const char *varName, uint32_t flags)
{
  const char *p;
  unsigned int bleft;
  int length=0;

  p=*pCurrentChar;
  bleft=*pBytesLeft;

  if (bleft>0) {
    const char *p2;

    p2=p;
    while (*p2 && *p2!='/' && *p2!=10)
      p2++;

    if (p2==p) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Empty text for %s (%s)", varName, p);
      return 0;
    }
    else {
      char *s;

      length=p2-p;
      s=(char *)GWEN_Memory_malloc(length+1); /* take trailing zero into account */
      memmove(s, p, length);
      s[length]=0;
      AHB_SWIFT_SetCharValue(data, flags, varName, s);
      GWEN_Memory_dealloc(s);
    }
    bleft-=length;
    p=p2;
  }

  *pCurrentChar=p;
  *pBytesLeft=bleft;
  return length;
}



