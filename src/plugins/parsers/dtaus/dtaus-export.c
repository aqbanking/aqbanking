/***************************************************************************
 begin       : Thu Apr 29 2004
 copyright   : (C) 2004-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dtaus_p.h"
#include "dtaus-export_p.h"
#include <aqbanking/banking.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gwentime.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif


/* TODO: AHB_DTAUS__AddWord transforms a given word into the DTA charset.
 * This might change the size of the word in bytes, so this transformation
 * should really be left to the caller...
 */


int AHB_DTAUS__ToDTA(int c) {
  if (isdigit(c))
    return c;
  if (c>='A' && c<='Z')
    return c;
  if (c>='a' && c<='z')
    return toupper(c);
  if (strchr(" .,&-+*%/$", c))
    return c;
  switch(c) {
  case 0xc4 : return 0x5b; // A umlaut
  case 0xd6 : return 0x5c; // O umlaut
  case 0xdc : return 0x5d; // U umlaut
  case 0xdf : return 0x7e; // szlig
  default:
    break;
  } /* switch */
  return 0;
}



int AHB_DTAUS__AddWord(GWEN_BUFFER *dst,
                       unsigned int size,
                       const char *s) {
  unsigned int i;
  unsigned int ssize;
  GWEN_BUFFER *nbuf;

  assert(dst);
  assert(size);
  assert(s);

  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding word: %s", s);

  nbuf=GWEN_Buffer_new(0, size, 0, 1);
  AB_ImExporter_Utf8ToDta(s, -1, nbuf);
  s=GWEN_Buffer_GetStart(nbuf);

  ssize=strlen(s);
  if (ssize>size) {
    /* Error out here because e.g. truncated accountid will lead to failed jobs. */
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Word \"%s\" too long: Has length %d but must not be longer than %d characters",
	      s, ssize, size);
    return -1;
  }

  for (i=0; i<size; i++) {
    char c;

    if (i>=ssize)
      c=0;
    else
      c=s[i];

    if (c)
      GWEN_Buffer_AppendByte(dst, c);
    else
      GWEN_Buffer_AppendByte(dst, ' ');
  } /* for */
  GWEN_Buffer_free(nbuf);
  return 0;
}



int AHB_DTAUS__AddDtaWord(GWEN_BUFFER *dst,
			  unsigned int size,
			  const char *s) {
  unsigned int i;
  unsigned int ssize;

  assert(dst);
  assert(size);
  assert(s);

  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding DTA word: %s", s);

  ssize=strlen(s);
  if (ssize>size) {
    /* Error out here because e.g. truncated accountid will lead to failed jobs. */
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Word \"%s\" too long: Has length %d but must not be longer than %d characters",
	      s, ssize, size);
    return -1;
  }

  for (i=0; i<size; i++) {
    char c;

    if (i>=ssize)
      c=0;
    else
      c=s[i];

    if (c)
      GWEN_Buffer_AppendByte(dst, c);
    else
      GWEN_Buffer_AppendByte(dst, ' ');
  } /* for */
  return 0;
}



int AHB_DTAUS__AddNum(GWEN_BUFFER *dst,
                      unsigned int size,
                      const char *s) {
  unsigned int i, j;

  assert(dst);
  assert(s);

  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding num : %s", s);

  i=strlen(s);
  if (i>size) {
    /* Error out here because e.g. truncated BLZ will lead to failed jobs. */
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Number \"%s\" too long: Has length %d but must not be longer than %d characters",
	      s, i, size);
    return -1;
  }
  j=size-i;
  if (j) {
    unsigned int k;

    for (k=0; k<j; k++)
      GWEN_Buffer_AppendByte(dst, '0');
  }
  GWEN_Buffer_AppendString(dst, s);
  return 0;
}



int AHB_DTAUS__CreateSetA(GWEN_BUFFER *dst,
                          GWEN_DB_NODE *cfg) {
  unsigned int i;
  GWEN_TIME *gt;
  char buffer[16];
  int day, month, year;
  const char *p;
  GWEN_DB_NODE *dbT;

  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Creating A set");
  /* field 1, 2: record header */
  GWEN_Buffer_AppendString(dst, "0128A");

  /* field 3: type */
  p=GWEN_DB_GetCharValue(cfg, "type", 0, "transfer");
  if (strcasecmp(p, "transfer")==0 ||
      strcasecmp(p, "transaction")==0)
    GWEN_Buffer_AppendString(dst, "GK");
  else if (strcasecmp(p, "debitnote")==0)
    GWEN_Buffer_AppendString(dst, "LK");
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown group \"%s\"", GWEN_DB_GroupName(cfg));
    return -1;
  }

  /* field 4: bank code */
  if (AHB_DTAUS__AddNum(dst, 8,
                          GWEN_DB_GetCharValue(cfg,
                                               "bankCode",
                                               0, "0"))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing bankCode to buffer");
    return -1;
  }

  /* field 5: 0s */
  for (i=0; i<8; i++) GWEN_Buffer_AppendByte(dst, '0');

  /* field 6: sender name */
  if (AHB_DTAUS__AddWord(dst, 27,
                         GWEN_DB_GetCharValue(cfg,
                                              "name",
                                              0, "0"))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing name to buffer");
    return -1;
  }

  /* field 7: date */
  gt=GWEN_CurrentTime();
  if (GWEN_Time_GetBrokenDownDate(gt, &day, &month, &year)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to break down date");
    GWEN_Time_free(gt);
    return -1;
  }
  GWEN_Time_free(gt);
  snprintf(buffer, sizeof(buffer), "%02d%02d%02d", day, month+1, year%100);
  if (AHB_DTAUS__AddWord(dst, 6, buffer)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    return -1;
  }

  /* field 8: blanks */
  for (i=0; i<4; i++) GWEN_Buffer_AppendByte(dst, ' ');

  /* field 9: account id */
  if (AHB_DTAUS__AddNum(dst, 10,
                        GWEN_DB_GetCharValue(cfg,
                                             "accountid",
                                             0, ""))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing accountid to buffer");
    return -1;
  }

  /* field 10: customer reference */
  if (AHB_DTAUS__AddNum(dst, 10,
                        GWEN_DB_GetCharValue(cfg,
                                             "custref",
                                             0, "0"))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing custref to buffer");
    return -1;
  }

  /* field 11a: blanks */
  for (i=0; i<15; i++) GWEN_Buffer_AppendByte(dst, ' ');

  /* field 11b: date of execution */
  dbT=GWEN_DB_GetGroup(cfg, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "execdate");
  if (dbT) {
    GWEN_TIME *ti;

    ti=GWEN_Time_fromDb(dbT);
    if (!ti) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad execution date");
      return -1;
    }

    if (GWEN_Time_GetBrokenDownDate(ti, &day, &month, &year)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad execution date");
      GWEN_Time_free(ti);
      return -1;
    }
    snprintf(buffer, sizeof(buffer), "%02d%02d%04d", day, month+1, year);

    if (AHB_DTAUS__AddWord(dst, 8, buffer)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing execdate to buffer");
      GWEN_Time_free(ti);
      return -1;
    }
    GWEN_Time_free(ti);
  }
  else {
    if (AHB_DTAUS__AddWord(dst, 8, "")) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
      return -1;
    }
  }

  /* field 11c: blanks */
  for (i=0; i<24; i++) GWEN_Buffer_AppendByte(dst, ' ');

  /* field 12: currency */
  p=GWEN_DB_GetCharValue(cfg, "currency", 0, "EUR");
  if (strcasecmp(p, "EUR")==0)
    GWEN_Buffer_AppendByte(dst, '1');
  else if (strcasecmp(p, "DEM")==0)
    GWEN_Buffer_AppendByte(dst, ' ');
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown currency \"%s\"", p);
    return -1;
  }

  return 0;
}



double AHB_DTAUS__string2double(const char *s){
  assert(s);

  while(*s && isspace(*s)) s++;
  if (!*s) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Empty value");
    return 0.0;
  }
  else {
    double d;

    if (GWEN_Text_StringToDouble(s, &d)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad value \"%s\"", s);
      return 0.0;
    }
    return d;
  }
}



int AHB_DTAUS__CreateSetC(GWEN_BUFFER *dst,
                          GWEN_DB_NODE *cfg,
                          GWEN_DB_NODE *xa,
			  AB_VALUE *sumEUR,
                          AB_VALUE *sumDEM,
                          AB_VALUE *sumBankCodes,
                          AB_VALUE *sumAccountIds){
  unsigned int i;
  const char *p;
  char buffer[32];
  int isDebitNote;
  int isEuro;
  unsigned int extSets;
  //unsigned int startPos;
  AB_VALUE *val;
  GWEN_STRINGLIST *purposeList;

  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Creating C set");

  /* ______________________________________________________________________
   * preparations
   */
  purposeList=GWEN_StringList_new();
  /* cut purpose lines into manageable portions (max 27 chars) */
  for (i=0; ; i++) {
    int slen;
    GWEN_BUFFER *nbuf;

    p=GWEN_DB_GetCharValue(xa, "purpose", i, 0);
    if (p==NULL)
      break;
    if (i>14) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Too many purpose lines (maxmimum is 14)");
      GWEN_StringList_free(purposeList);
      return -1;
    }

    slen=strlen(p);
    nbuf=GWEN_Buffer_new(0, slen+1, 0, 1);
    AB_ImExporter_Utf8ToDta(p, -1, nbuf);
    p=GWEN_Buffer_GetStart(nbuf);

    while(*p) {
      while(*p>0 && *p<33)
	p++;
      slen=strlen(p);
      if (slen==0)
        break;
      else if (slen>27) {
	char *ns;

	ns=(char*) malloc(28);
	assert(ns);
	memmove(ns, p, 27);
	ns[27]=0;
	/* let stringlist take over ownership of the the string */
	GWEN_StringList_AppendString(purposeList, ns, 1, 0);
	p+=27;
      }
      else {
	GWEN_StringList_AppendString(purposeList, p, 0, 0);
	break;
      }
    }
    GWEN_Buffer_free(nbuf);
  } /* for */

  //startPos=GWEN_Buffer_GetPos(dst);
  GWEN_Buffer_AllocRoom(dst, 256);

  isDebitNote=(strcasecmp(GWEN_DB_GetCharValue(cfg, "type", 0, "transfer"),
                          "debitnote")==0);
  isEuro=(strcasecmp(GWEN_DB_GetCharValue(cfg, "currency", 0, "EUR"),
                     "EUR")==0);

  /* compute number of extension sets */
  extSets=0;

  /* add purpose */
  if (GWEN_StringList_Count(purposeList))
    extSets+=GWEN_StringList_Count(purposeList)-1;

  /* add name */
  for (i=1; i<2; i++) { /* max 1 extset for local name */
    if (GWEN_DB_GetCharValue(xa, "localName", i, 0)==0)
      break;
    if (i>1) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Too many name lines (maxmimum is 2)");
      GWEN_StringList_free(purposeList);
      return -1;
    }
    extSets++;
  } /* for */

  /* add other name */
  for (i=1; i<2; i++) { /* max 1 extset for remote name */
    if (GWEN_DB_GetCharValue(xa, "remoteName", i, 0)==0)
      break;
    if (i>1) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Too many peer name lines (maxmimum is 2)");
      GWEN_StringList_free(purposeList);
      return -1;
    }
    extSets++;
  } /* for */

  /* check number of extension sets */
  if (extSets>15) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Too many extension sets (%d)", extSets);
    GWEN_StringList_free(purposeList);
    return -1;
  }

  /* ______________________________________________________________________
   * actually write C set
   */

  /* field 1, 2: record header */
  snprintf(buffer, sizeof(buffer), "%04d", 187+(extSets*29));
  GWEN_Buffer_AppendString(dst, buffer);
  GWEN_Buffer_AppendByte(dst, 'C');

  /* field 3: acting bank code */
  if (AHB_DTAUS__AddNum(dst, 8, GWEN_DB_GetCharValue(cfg, "bankCode", 0, ""))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    GWEN_StringList_free(purposeList);
    return -1;
  }

  /* field 4: destination bank code */
  p=GWEN_DB_GetCharValue(xa, "remoteBankCode", 0, 0);
  if (p) {
    val=AB_Value_fromString(p);
    if (val==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad bank code");
      GWEN_StringList_free(purposeList);
      return -1;
    }
    AB_Value_AddValue(sumBankCodes, val);
    AB_Value_free(val);
    if (AHB_DTAUS__AddNum(dst, 8, p)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
      GWEN_StringList_free(purposeList);
      return -1;
    }
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Peer bank code missing");
    GWEN_StringList_free(purposeList);
    return -1;
  }

  /* field 5: destination account id */
  p=GWEN_DB_GetCharValue(xa, "remoteAccountNumber", 0, 0);
  if (p) {
    val=AB_Value_fromString(p);
    if (val==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad account id");
      GWEN_StringList_free(purposeList);
      return -1;
    }
    AB_Value_AddValue(sumAccountIds, val);
    AB_Value_free(val);
    if (AHB_DTAUS__AddNum(dst, 10, p)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
      GWEN_StringList_free(purposeList);
      return -1;
    }
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Peer account id missing");
    GWEN_StringList_free(purposeList);
    return -1;
  }

  /* field 6: internal customer number (0s for now) */
  for (i=0; i<13; i++) GWEN_Buffer_AppendByte(dst, '0');

  /* field 7a: text key */
  snprintf(buffer, sizeof(buffer), "%02d", GWEN_DB_GetIntValue(xa, "textkey", 0, isDebitNote?5:51));
  if (AHB_DTAUS__AddNum(dst, 2, buffer)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    GWEN_StringList_free(purposeList);
    return -1;
  }

  /* field 7b: text key extension */
  snprintf(buffer, sizeof(buffer), "%03d", GWEN_DB_GetIntValue(xa, "textkeyext", 0, 0));
  if (AHB_DTAUS__AddNum(dst, 3, buffer)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    GWEN_StringList_free(purposeList);
    return -1;
  }

  /* field 8: bank internal field */
  GWEN_Buffer_AppendByte(dst, ' ');

  /* field 9: value in DEM */
  if (!isEuro) {
    val=AB_Value_fromString(GWEN_DB_GetCharValue(xa, "value/value", 0, "0,0"));
    if (val==NULL || AB_Value_IsZero(val)) {
      AB_Value_free(val);
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad DEM value:");
      GWEN_StringList_free(purposeList);
      return -1;
    }
    AB_Value_AddValue(sumDEM, val);
    snprintf(buffer, sizeof(buffer), "%011.0f", AB_Value_GetValueAsDouble(val)*100.0);
    AB_Value_free(val);
    if (AHB_DTAUS__AddNum(dst, 11, buffer)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
      GWEN_StringList_free(purposeList);
      return -1;
    }
  }
  else {
    if (AHB_DTAUS__AddNum(dst, 11, "0")) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
      GWEN_StringList_free(purposeList);
      return -1;
    }
  }

  /* field 10: local bank code */
  p=GWEN_DB_GetCharValue(xa, "localbankCode", 0, 0);
  if (!p)
    p=GWEN_DB_GetCharValue(cfg, "bankCode", 0, 0);
  if (!p) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No local bank code");
    GWEN_StringList_free(purposeList);
    return -1;
  }
  if (AHB_DTAUS__AddNum(dst, 8, p)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    GWEN_StringList_free(purposeList);
    return -1;
  }

  /* field 11: local account id */
  p=GWEN_DB_GetCharValue(xa, "localAccountNumber", 0, 0);
  if (!p)
    GWEN_DB_GetCharValue(cfg, "accountId", 0, 0);
  if (!p) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No local account number");
    GWEN_StringList_free(purposeList);
    return -1;
  }
  if (AHB_DTAUS__AddNum(dst, 10, p)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    GWEN_StringList_free(purposeList);
    return -1;
  }

  /* field 12: value in EUR */
  if (isEuro) {
    val=AB_Value_fromString(GWEN_DB_GetCharValue(xa, "value/value", 0, "0,0"));
    if (val==NULL || AB_Value_IsZero(val)) {
      AB_Value_free(val);
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad EUR value:");
      GWEN_StringList_free(purposeList);
      return -1;
    }
    AB_Value_AddValue(sumEUR, val);
    snprintf(buffer, sizeof(buffer), "%011.0f", AB_Value_GetValueAsDouble(val)*100.0);
    AB_Value_free(val);
    if (AHB_DTAUS__AddNum(dst, 11, buffer)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
      GWEN_StringList_free(purposeList);
      return -1;
    }
  }
  else {
    if (AHB_DTAUS__AddNum(dst, 11, "0")) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
      GWEN_StringList_free(purposeList);
      return -1;
    }
  }

  /* field 13: blanks */
  for (i=0; i<3; i++) GWEN_Buffer_AppendByte(dst, ' ');

  /* field 14a: peer name */
  if (AHB_DTAUS__AddWord(dst, 27, GWEN_DB_GetCharValue(xa, "remoteName", 0, ""))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    GWEN_StringList_free(purposeList);
    return -1;
  }

  /* field 14b: blanks */
  for (i=0; i<8; i++) GWEN_Buffer_AppendByte(dst, ' ');

  /* field 15: name */
  if (AHB_DTAUS__AddWord(dst, 27, GWEN_DB_GetCharValue(xa, "localname", 0, ""))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    GWEN_StringList_free(purposeList);
    return -1;
  }

  /* field 16: purpose */
  p=GWEN_StringList_FirstString(purposeList);
  if (p==NULL)
    p="";
  if (AHB_DTAUS__AddWord(dst, 27, p)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    GWEN_StringList_free(purposeList);
    return -1;
  }

  /* field 17a: currency */
  if (isEuro)
    GWEN_Buffer_AppendByte(dst, '1');
  else
    GWEN_Buffer_AppendByte(dst, ' ');

  /* field 17b: blanks */
  for (i=0; i<2; i++) GWEN_Buffer_AppendByte(dst, ' ');

  /* field 18: number of extension sets */
  snprintf(buffer, sizeof(buffer), "%02d", extSets);
  if (AHB_DTAUS__AddNum(dst, 2, buffer)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    GWEN_StringList_free(purposeList);
    return -1;
  }

  if (extSets) {
    unsigned int writtenExtSets=0;

    /* now append extension sets */

    /* add peer name lines */
    for (i=1; i<2; i++) { /* max: 1 extset */
      unsigned int j;

      p=GWEN_DB_GetCharValue(xa, "remoteName", i, 0);
      if (!p)
	break;

      /* append extension set */
      GWEN_Buffer_AppendString(dst, "01");
      if (AHB_DTAUS__AddWord(dst, 27, p)) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
	GWEN_StringList_free(purposeList);
	return -1;
      }
      writtenExtSets++;

      if (writtenExtSets==2)
	/* 2 ext sets written, so we need to align "C 2.Satzabschnitt" to 128 now */
	for (j=0; j<11; j++) GWEN_Buffer_AppendByte(dst, ' ');
      else if (writtenExtSets>2 && ((writtenExtSets-2) % 4)==0)
	/* "C 3-5.Satzabschnitt" complete, align to 128 bytes */
	for (j=0; j<12; j++) GWEN_Buffer_AppendByte(dst, ' ');
    } /* for */

    /* add purpose lines */
    for (i=1; i<GWEN_StringList_Count(purposeList); i++) {
      unsigned int j;

      p=GWEN_StringList_StringAt(purposeList, i);
      if (!p)
	break;

      /* append extension set */
      GWEN_Buffer_AppendString(dst, "02");
      /* strings in the list are already in DTA charset */
      if (AHB_DTAUS__AddDtaWord(dst, 27, p)) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
	GWEN_StringList_free(purposeList);
	return -1;
      }
      writtenExtSets++;

      if (writtenExtSets==2)
	/* 2 ext sets written, so we need to align "C 2.Satzabschnitt" to 128 now */
	for (j=0; j<11; j++) GWEN_Buffer_AppendByte(dst, ' ');
      else if (writtenExtSets>2 && ((writtenExtSets-2) % 4)==0)
	/* "C 3-5.Satzabschnitt" complete, align to 128 bytes */
	for (j=0; j<12; j++) GWEN_Buffer_AppendByte(dst, ' ');
    } /* for */

    /* add name lines */
    for (i=1; i<2; i++) { /* max: 1 extset */
      unsigned int j;

      p=GWEN_DB_GetCharValue(xa, "localname", i, 0);
      if (!p)
	break;

      /* append extension set */
      GWEN_Buffer_AppendString(dst, "03");
      if (AHB_DTAUS__AddWord(dst, 27, p)) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
	GWEN_StringList_free(purposeList);
	return -1;
      }
      writtenExtSets++;

      if (writtenExtSets==2)
	/* 2 ext sets written, so we need to align "C 2.Satzabschnitt" to 128 now */
	for (j=0; j<11; j++) GWEN_Buffer_AppendByte(dst, ' ');
      else if (writtenExtSets>2 && ((writtenExtSets-2) % 4)==0)
	/* "C 3-5.Satzabschnitt" complete, align to 128 bytes */
	for (j=0; j<12; j++) GWEN_Buffer_AppendByte(dst, ' ');
    } /* for */
  }

  i=((GWEN_Buffer_GetUsedBytes(dst)+127) & ~127)-GWEN_Buffer_GetUsedBytes(dst);
  while(i--)
    GWEN_Buffer_AppendByte(dst, ' ');

  GWEN_StringList_free(purposeList);
  return 0;
}



int AHB_DTAUS__CreateSetE(GWEN_BUFFER *dst,
                          GWEN_DB_NODE *cfg,
                          int csets,
			  AB_VALUE *sumEUR,
                          AB_VALUE *sumDEM,
                          AB_VALUE *sumBankCodes,
                          AB_VALUE *sumAccountIds){
  unsigned int i;
  char buffer[32];

  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Creating E set");

  /* field 1, 2: record header */
  GWEN_Buffer_AppendString(dst, "0128E");

  /* field 3: reserved */
  for (i=0; i<5; i++) GWEN_Buffer_AppendByte(dst, ' ');

  /* field 4: number of C sets */
  snprintf(buffer, sizeof(buffer), "%07d", csets);
  if (AHB_DTAUS__AddNum(dst, 7, buffer)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    return -1;
  }

  /* field 5: sum of DEM values */
  snprintf(buffer, sizeof(buffer), "%013.0f",
	   AB_Value_GetValueAsDouble(sumDEM)*100.0);
  if (AHB_DTAUS__AddNum(dst, 13, buffer)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    return -1;
  }

  /* field 6: sum of peer account ids */
  snprintf(buffer, sizeof(buffer), "%017.0f",
	   AB_Value_GetValueAsDouble(sumAccountIds));
  if (AHB_DTAUS__AddNum(dst, 17, buffer)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    return -1;
  }

  /* field 7: sum of peer bank codes */
  snprintf(buffer, sizeof(buffer), "%017.0f",
	   AB_Value_GetValueAsDouble(sumBankCodes));
  if (AHB_DTAUS__AddNum(dst, 17, buffer)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    return -1;
  }

  /* field 8: sum of EUR values */
  snprintf(buffer, sizeof(buffer), "%013.0f",
	   AB_Value_GetValueAsDouble(sumEUR)*100.0);
  if (AHB_DTAUS__AddNum(dst, 13, buffer)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing to buffer");
    return -1;
  }

  /* field 9: reserved */
  for (i=0; i<51; i++) GWEN_Buffer_AppendByte(dst, ' ');

  return 0;
}



int AHB_DTAUS__Export(GWEN_DBIO *dbio,
		      GWEN_SYNCIO *sio,
                      GWEN_DB_NODE *data,
		      GWEN_DB_NODE *cfg,
		      uint32_t flags){
  AB_VALUE *sumEUR;
  AB_VALUE *sumDEM;
  AB_VALUE *sumBankCodes;
  AB_VALUE *sumAccountIds;
  unsigned int cSets;
  GWEN_BUFFER *dst;
  GWEN_DB_NODE *gr;
  int isDebitNote;
  //int isEuro;
  const uint8_t *p;
  uint32_t size;
  int rv;

  isDebitNote=(strcasecmp(GWEN_DB_GetCharValue(cfg, "type", 0, "transfer"),
                          "debitnote")==0);
  //isEuro=(strcasecmp(GWEN_DB_GetCharValue(cfg, "currency", 0, "EUR"),
  //                   "EUR")==0);
  cSets=0;
  sumEUR=AB_Value_new();
  sumDEM=AB_Value_new();
  sumBankCodes=AB_Value_new();
  sumAccountIds=AB_Value_new();

  dst=GWEN_Buffer_new(0, 1024, 0, 1);
  GWEN_Buffer_SetHardLimit(dst, AHB_DTAUS_HARDLIMIT);

  /* create A set */
  if (AHB_DTAUS__CreateSetA(dst, cfg)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error creating A set");
    GWEN_Buffer_free(dst);
    AB_Value_free(sumAccountIds);
    AB_Value_free(sumBankCodes);
    AB_Value_free(sumDEM);
    AB_Value_free(sumEUR);
    return -1;
  }

  /* create C sets */
  gr=GWEN_DB_GetFirstGroup(data);
  while(gr) {
    const char *gn;

    gn=GWEN_DB_GroupName(gr);
    if ((isDebitNote && strcasecmp(gn, "debitnote")==0) ||
	(!isDebitNote &&
	 (strcasecmp(gn, "transfer")==0 ||
	  strcasecmp(gn, "transaction")==0))){
      if (AHB_DTAUS__CreateSetC(dst, cfg, gr,
				sumEUR, sumDEM,
				sumBankCodes, sumAccountIds)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Error creating C set from this data:");
        GWEN_DB_Dump(gr, 2);
        GWEN_Buffer_free(dst);
	AB_Value_free(sumAccountIds);
	AB_Value_free(sumBankCodes);
	AB_Value_free(sumDEM);
	AB_Value_free(sumEUR);
	return -1;
      }
      cSets++;
    } /* if group matches */
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Ignoring group [%s]",
		GWEN_DB_GroupName(gr));
    }
    gr=GWEN_DB_GetNextGroup(gr);
  } /* while */

  /* create E set */
  if (AHB_DTAUS__CreateSetE(dst, cfg, cSets,
                            sumEUR, sumDEM,
                            sumBankCodes, sumAccountIds)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error creating E set");
    GWEN_Buffer_free(dst);
    AB_Value_free(sumAccountIds);
    AB_Value_free(sumBankCodes);
    AB_Value_free(sumDEM);
    AB_Value_free(sumEUR);
    return -1;
  }

  AB_Value_free(sumAccountIds);
  AB_Value_free(sumBankCodes);
  AB_Value_free(sumDEM);
  AB_Value_free(sumEUR);

  /* DTAUS finished, write it */
  p=(const uint8_t*)GWEN_Buffer_GetStart(dst);
  size=GWEN_Buffer_GetUsedBytes(dst);
  rv=GWEN_SyncIo_WriteForced(sio, p, size);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Broken pipe");
    GWEN_Buffer_free(dst);
    return GWEN_ERROR_IO;
  }
  GWEN_Buffer_free(dst);

  return 0;
}










