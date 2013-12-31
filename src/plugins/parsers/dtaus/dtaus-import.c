/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Apr 29 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dtaus_p.h"
#include "dtaus-import_p.h"
#include <aqbanking/banking.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/syncio_file.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


int AHB_DTAUS__SetCharValue(GWEN_DB_NODE *db,
                            uint32_t flags,
                            const char *name,
                            const char *s) {
  GWEN_BUFFER *vbuf;
  int rv;

  vbuf=GWEN_Buffer_new(0, strlen(s)+32, 0, 1);
  AB_ImExporter_DtaToUtf8(s, -1, vbuf);
  rv=GWEN_DB_SetCharValue(db, flags, name, GWEN_Buffer_GetStart(vbuf));
  GWEN_Buffer_free(vbuf);
  return rv;
}



int AHB_DTAUS__ReadWord(GWEN_BUFFER *src,
			GWEN_BUFFER *dst,
			unsigned int pos,
			unsigned int size) {
  unsigned int i;
  char *p;

  if (GWEN_Buffer_SetPos(src, pos)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Position %d out of range (size is %d)",
	      pos, GWEN_Buffer_GetUsedBytes(src));
    return -1;
  }

  /* skip blanks */
  for (i=0; i<size; i++) {
    int c;

    c=GWEN_Buffer_PeekByte(src);
    if (c==-1)
      break;
    if (!isspace(c))
      break;
    GWEN_Buffer_ReadByte(src);
  } /* for */

  /* read data */
  size-=i;
  for (i=0; i<size; i++) {
    int c;

    c=GWEN_Buffer_ReadByte(src);
    if (c==-1)
      break;
    GWEN_Buffer_AppendByte(dst, (unsigned char)c);
  }

  /* remove trailing spaces */
  p=GWEN_Buffer_GetStart(dst);
  i=size;
  while(i) {
    if (!isspace(p[i-1])) {
      break;
    }
    i--;
  } /* while */
  GWEN_Buffer_Crop(dst, 0, i);
  GWEN_Buffer_SetPos(dst, i);

  return 0;
}



int AHB_DTAUS__ParseExtSet(GWEN_BUFFER *src,
			   unsigned int pos,
			   GWEN_DB_NODE *xa){
  int typ;
  GWEN_BUFFER *tmp;

  tmp=GWEN_Buffer_new(0, 128, 0, 1);

  /* read type */
  if (AHB_DTAUS__ReadWord(src, tmp, pos, 2)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    GWEN_Buffer_free(tmp);
    return -1;
  }

  if (1!=sscanf(GWEN_Buffer_GetStart(tmp), "%d", &typ)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "not an integer at %d", pos);
    GWEN_Buffer_free(tmp);
    return -1;
  }

  /* read argument */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+2, 27)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading argument from ext set at %d", pos);
    GWEN_Buffer_free(tmp);
    return -1;
  }

  /* check for empty argument */
  if (GWEN_Buffer_GetUsedBytes(tmp)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Empty argument in ext set at %d", pos);
    GWEN_Buffer_free(tmp);
    return 0;
  }

  /* store argument according to type */
  switch(typ) {
  case 1: /* Kundenname */
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Customer name: %s", GWEN_Buffer_GetStart(tmp));
    AHB_DTAUS__SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT,
                            "remoteName",
                         GWEN_Buffer_GetStart(tmp));
    break;
  case 2: /* Verwendungszweck */
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Purpose: %s", GWEN_Buffer_GetStart(tmp));
    AHB_DTAUS__SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT,
                            "purpose",
			 GWEN_Buffer_GetStart(tmp));
    break;
  case 3: /* Name des Auftraggebers */
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Peer name: %s", GWEN_Buffer_GetStart(tmp));
    AHB_DTAUS__SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT,
                            "localName",
                            GWEN_Buffer_GetStart(tmp));
    break;
  default: /* unbekannt */
    break;
  } /* switch */
  GWEN_Buffer_free(tmp);

  return 0;
}



int AHB_DTAUS__ParseSetA(GWEN_BUFFER *src,
			 unsigned int pos,
			 GWEN_DB_NODE *xa){
  GWEN_BUFFER *tmp;

  tmp=GWEN_Buffer_new(0, 128, 0, 1);

  /* read transaction type */
  DBG_DEBUG(AQBANKING_LOGDOMAIN,
	    "Reading transaction type at %d", pos+5);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+5, 2)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error reading transaction type at %d", pos+5);
    GWEN_Buffer_free(tmp);
    return -1;
  }

  /* check transaction type */
  if (strcasecmp(GWEN_Buffer_GetStart(tmp), "GK")==0 ||
      strcasecmp(GWEN_Buffer_GetStart(tmp), "GB")==0) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "This DTAUS record contains transactions");
    GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "type", "transfer");
    GWEN_DB_GroupRename(xa, "transaction");
  }
  else if (strcasecmp(GWEN_Buffer_GetStart(tmp), "LK")==0) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "This DTAUS record contains debit notes");
    GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "type", "debitnote");
    GWEN_DB_GroupRename(xa, "debitnote");
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Transaction type \"%s\" not supported, assuming GK",
	      GWEN_Buffer_GetStart(tmp));
    GWEN_DB_GroupRename(xa, "transaction");
  }

  /* bank code */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+7, 8)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading bank code at %d", pos+7);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Our bank code: %s", GWEN_Buffer_GetStart(tmp));
  GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT |
                       GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "localbankCode",
		       GWEN_Buffer_GetStart(tmp));

  /* date */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+50, 6)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error reading date at %d", pos+50);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    int day, month, year;
    const char *p;
    GWEN_TIME *ti;

    if (GWEN_Buffer_GetUsedBytes(tmp)!=6) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid date at %d", pos+50);
      GWEN_Buffer_free(tmp);
      return -1;
    }
    p=GWEN_Buffer_GetStart(tmp);
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Date: %s", p);
    day=((p[0]-'0')*10) + (p[1]-'0');
    month=((p[2]-'0')*10) + (p[3]-'0');
    year=((p[4]-'0')*10) +
      (p[5]-'0');
    if (year>92)
      year+=1900;
    else
      year+=2000;
    ti=GWEN_Time_new(year, month-1, day, 12, 0, 0, 1);
    if (GWEN_Time_toDb(ti, GWEN_DB_GetGroup(xa,
                                            GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                                            "date"))) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error saving date");
      GWEN_Buffer_free(tmp);
      return -1;
    }
  }


  /* account id */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+60, 10)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading account id at %d", pos+60);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Our account id: %s", GWEN_Buffer_GetStart(tmp));
  GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT |
                       GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "localAccountNumber",
		       GWEN_Buffer_GetStart(tmp));

  /* customer reference */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+70, 10)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading customer reference at %d", pos+70);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Customer reference: %s", GWEN_Buffer_GetStart(tmp));
    AHB_DTAUS__SetCharValue(xa,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "custref",
			 GWEN_Buffer_GetStart(tmp));
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No customer reference");
  }

  /* execution date */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+95, 8)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading execution date at %d", pos+95);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    int day, month, year;
    const char *p;
    GWEN_TIME *ti;

    if (GWEN_Buffer_GetUsedBytes(tmp)!=8) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid execution date at %d", pos+95);
      GWEN_Buffer_free(tmp);
      return -1;
    }
    p=GWEN_Buffer_GetStart(tmp);
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Exec date: %s", p);
    day=((p[0]-'0')*10) + (p[1]-'0');
    month=((p[2]-'0')*10) + (p[3]-'0');
    year=((p[4]-'0')*1000)+
      ((p[5]-'0')*100)+
      ((p[6]-'0')*10) +
      (p[7]-'0');
    ti=GWEN_Time_new(year, month-1, day, 12, 0, 0, 1);
    if (GWEN_Time_toDb(ti, GWEN_DB_GetGroup(xa,
                                            GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                                            "execDate"))) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error saving date");
      GWEN_Buffer_free(tmp);
      return -1;
    }
  }

  /* currency mark */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+127, 1)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading currency mark at %d", pos+127);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (*(GWEN_Buffer_GetStart(tmp))=='1') {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Currency: EUR");
    GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT,
			 "value/currency",
			 "EUR");
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Currency: DEM");
    GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT,
			 "value/currency",
			 "DEM");
  }

  GWEN_Buffer_free(tmp);
  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Set size: %d", 128);
  return 128;
}



int AHB_DTAUS__ParseSetC(GWEN_BUFFER *src,
                         unsigned int pos,
                         GWEN_DB_NODE *xa,
			 AB_VALUE *sumEUR,
			 AB_VALUE *sumDEM,
			 AB_VALUE *sumBankCodes,
                         AB_VALUE *sumAccountIds){
  GWEN_BUFFER *tmp;
  AB_VALUE *val;
  unsigned int extSets;
  unsigned int i;
  unsigned int lpos;

  tmp=GWEN_Buffer_new(0, 256, 0, 1);

  /* read peer's bank code */
  if (AHB_DTAUS__ReadWord(src, tmp, pos+13, 8)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading bank code at %d", pos+13);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Bank code: %s", GWEN_Buffer_GetStart(tmp));
    val=AB_Value_fromString(GWEN_Buffer_GetStart(tmp));
    if (val==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad bank code at %d", pos+13);
      GWEN_Buffer_free(tmp);
      return -1;
    }
    AB_Value_AddValue(sumBankCodes, val);
    AB_Value_free(val);
    GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT |
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "remoteBankCode",
                         GWEN_Buffer_GetStart(tmp));
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No bank code");
  }

  /* read peer's account id */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+21, 10)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading account id at %d", pos+21);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Account id: %s", GWEN_Buffer_GetStart(tmp));
    val=AB_Value_fromString(GWEN_Buffer_GetStart(tmp));
    if (val==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad account id at %d", pos+21);
      GWEN_Buffer_free(tmp);
      return -1;
    }
    AB_Value_AddValue(sumAccountIds, val);
    AB_Value_free(val);
    GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT |
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "remoteAccountNumber",
                         GWEN_Buffer_GetStart(tmp));
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No account id");
  }

  /* read text key */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+44, 2)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error text key at %d", pos+44);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    const char *x;

    x=GWEN_Buffer_GetStart(tmp);
    if (*x=='0')
      x++;
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Text key: %s", x);
    AHB_DTAUS__SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT |
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "textkey",
                            x);
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No text key");
  }

  /* read own bank code */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+61, 8)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading bank code at %d", pos+61);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Bank code: %s", GWEN_Buffer_GetStart(tmp));
    GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT |
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "localBankCode",
                         GWEN_Buffer_GetStart(tmp));
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No bank code");
  }

  /* read own account id */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+69, 10)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading account id at %d", pos+69);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Account id: %s", GWEN_Buffer_GetStart(tmp));
    GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT |
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "localAccountNumber",
                         GWEN_Buffer_GetStart(tmp));
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No account id");
  }

  /* read value */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+182, 1)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading currency mark at %d", pos+182);
    GWEN_Buffer_free(tmp);
    return -1;
  }

  if (GWEN_Buffer_GetStart(tmp)[0]=='1') {
    /* EUR */
    GWEN_Buffer_Reset(tmp);
    if (AHB_DTAUS__ReadWord(src, tmp, pos+79, 11)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading value at %d", pos+79);
      GWEN_Buffer_free(tmp);
      return -1;
    }
    if (GWEN_Buffer_GetUsedBytes(tmp)) {
      if (GWEN_Buffer_GetUsedBytes(tmp)==11) {
	DBG_DEBUG(AQBANKING_LOGDOMAIN, "Value: %s",
		  GWEN_Buffer_GetStart(tmp));

	GWEN_Buffer_AppendString(tmp, "/100");
	GWEN_DB_SetCharValue(xa,
			     GWEN_DB_FLAGS_DEFAULT |
			     GWEN_DB_FLAGS_OVERWRITE_VARS,
			     "value/value",
			     GWEN_Buffer_GetStart(tmp));

        GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT |
                             GWEN_DB_FLAGS_OVERWRITE_VARS,
			     "value/currency", "EUR");
	val=AB_Value_fromString(GWEN_Buffer_GetStart(tmp));
	if (val==NULL) {
	  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Bad EUR value");
	  GWEN_Buffer_free(tmp);
	  return -1;
	}
	AB_Value_AddValue(sumEUR, val);
	AB_Value_free(val);
      }
      else {
        DBG_DEBUG(AQBANKING_LOGDOMAIN, "Bad EUR value");
        GWEN_Buffer_free(tmp);
        return -1;
      }
    }
    else {
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "No EUR value");
      GWEN_Buffer_free(tmp);
      return -1;
    }
  }
  else {
    /* DEM */
    GWEN_Buffer_Reset(tmp);
    if (AHB_DTAUS__ReadWord(src, tmp, pos+50, 11)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading value at %d", pos+50);
      GWEN_Buffer_free(tmp);
      return -1;
    }
    if (GWEN_Buffer_GetUsedBytes(tmp)) {
      if (GWEN_Buffer_GetUsedBytes(tmp)==11) {
        DBG_DEBUG(AQBANKING_LOGDOMAIN, "Value: %s", GWEN_Buffer_GetStart(tmp));
	GWEN_Buffer_AppendString(tmp, "/100");
	GWEN_DB_SetCharValue(xa,
			     GWEN_DB_FLAGS_DEFAULT |
			     GWEN_DB_FLAGS_OVERWRITE_VARS,
			     "value/value",
			     GWEN_Buffer_GetStart(tmp));

	GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT |
                             GWEN_DB_FLAGS_OVERWRITE_VARS,
			     "value/currency", "DEM");
	val=AB_Value_fromString(GWEN_Buffer_GetStart(tmp));
	if (val==NULL) {
	  DBG_DEBUG(AQBANKING_LOGDOMAIN, "Bad DEM value");
	  GWEN_Buffer_free(tmp);
	  return -1;
	}
	AB_Value_AddValue(sumDEM, val);
        AB_Value_free(val);
      }
      else {
        DBG_DEBUG(AQBANKING_LOGDOMAIN, "Bad DEM value");
        GWEN_Buffer_free(tmp);
        return -1;
      }
    }
    else {
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "No DEM value");
      GWEN_Buffer_free(tmp);
      return -1;
    }
  }

  /* read peer name */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+93, 27)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading peer name at %d", pos+93);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Peer name: %s", GWEN_Buffer_GetStart(tmp));
    AHB_DTAUS__SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT |
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "remoteName",
                         GWEN_Buffer_GetStart(tmp));
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No peer name");
  }

  /* read own name */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+128, 27)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading peer name at %d", pos+128);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Name: %s", GWEN_Buffer_GetStart(tmp));
    AHB_DTAUS__SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT |
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "localName",
                         GWEN_Buffer_GetStart(tmp));
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No name");
  }

  /* read purpose */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+155, 27)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading purpose at %d", pos+155);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Purpose: %s", GWEN_Buffer_GetStart(tmp));
    AHB_DTAUS__SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT |
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "purpose",
                            GWEN_Buffer_GetStart(tmp));
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No purpose");
  }

  /* read number of extension sets */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+185, 2)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Error reading number of ext sets at %d", pos+185);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (1!=sscanf(GWEN_Buffer_GetStart(tmp), "%ud", &extSets)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad number of ext sets at %d (%04x)",
              pos+185,
              pos+185);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  DBG_INFO(AQBANKING_LOGDOMAIN, "%d extension sets", extSets);

  /* read extension sets */
  if (extSets>0) {
    if (AHB_DTAUS__ParseExtSet(src, pos+187, xa)==-1) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Error in first extension set at %d", pos+187);
      GWEN_Buffer_free(tmp);
      return -1;
    }
  }

  if (extSets>1) {
    if (AHB_DTAUS__ParseExtSet(src, pos+216, xa)==-1) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Error in second extension set at %d", pos+216);
      GWEN_Buffer_free(tmp);
      return -1;
    }
  }

  /* read next extension sets */
  i=2;
  lpos=256;
  while(i<extSets) {
    int j;

    for (j=0; j<4; j++) {
      if (i+j>=extSets)
        break;
      DBG_DEBUG(AQBANKING_LOGDOMAIN,
		"Reading extension set %d at %d", i+j, pos+lpos);
      if (AHB_DTAUS__ParseExtSet(src, pos+lpos, xa)==-1) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Error in extension set %d at %d", i+j, pos+lpos);
        GWEN_Buffer_free(tmp);
        return -1;
      }
      lpos+=29;
    } /* for j */
    i+=j;
    lpos+=12;
  } /* while */

  lpos=(lpos+127) & ~127;

  GWEN_Buffer_free(tmp);
  return lpos;
}



int AHB_DTAUS__ParseSetE(GWEN_BUFFER *src,
                         unsigned int pos,
                         unsigned int csets,
			 AB_VALUE *sumEUR,
                         AB_VALUE *sumDEM,
                         AB_VALUE *sumBankCodes,
                         AB_VALUE *sumAccountIds){
  GWEN_BUFFER *tmp;
  unsigned int i;
  AB_VALUE *val;

  tmp=GWEN_Buffer_new(0, 128, 0, 1);

  if (AHB_DTAUS__ReadWord(src, tmp, pos+10, 7)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Error reading number of C sets at %d", pos+10);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (1!=sscanf(GWEN_Buffer_GetStart(tmp), "%ud", &i)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Bad number of C sets at %d", pos+10);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (i!=csets) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Bad number of C sets (is %d, should be %d)", csets, i);
    GWEN_Buffer_free(tmp);
    return -1;
  }

  /* checksum of DEM values */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+17, 13)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading value at %d", pos+17);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "DEM checksum: %s",
	      GWEN_Buffer_GetStart(tmp));
    GWEN_Buffer_AppendString(tmp, "/100");
    val=AB_Value_fromString(GWEN_Buffer_GetStart(tmp));
    if (val==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad value at %d", pos+17);
      GWEN_Buffer_free(tmp);
      return -1;
    }
    if (AB_Value_Compare(sumDEM, val)!=0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Bad DEM checksum (is %.2f, should be %.2f)",
		AB_Value_GetValueAsDouble(sumDEM),
		AB_Value_GetValueAsDouble(val));
      AB_Value_free(val);
      GWEN_Buffer_free(tmp);
      return -1;
    }
    else {
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "DEM checksum ok");
      AB_Value_free(val);
    }
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No DEM checksum");
  }

  /* checksum of account ids */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+30, 17)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading value at %d", pos+30);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN,
	      "Account id checksum: %s", GWEN_Buffer_GetStart(tmp));
    val=AB_Value_fromString(GWEN_Buffer_GetStart(tmp));
    if (val==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad value at %d", pos+30);
      GWEN_Buffer_free(tmp);
      return -1;
    }
    if (AB_Value_Compare(sumAccountIds, val)!=0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Bad account id checksum (is %f, should be %f)",
		AB_Value_GetValueAsDouble(sumAccountIds),
		AB_Value_GetValueAsDouble(val));
      AB_Value_free(val);
      GWEN_Buffer_free(tmp);
      return -1;
    }
    else {
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "Account id checksum ok");
      AB_Value_free(val);
    }
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No account id checksum");
  }

  /* checksum of bank codes */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+47, 17)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading value at %d", pos+30);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Bank code checksum: %s",
	      GWEN_Buffer_GetStart(tmp));
    val=AB_Value_fromString(GWEN_Buffer_GetStart(tmp));
    if (val==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad bank code checksum: %s",
		GWEN_Buffer_GetStart(tmp));
      GWEN_Buffer_free(tmp);
      return -1;
    }

    if (AB_Value_Compare(sumBankCodes, val)!=0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Bad bank code checksum (is %f, should be %f)",
		AB_Value_GetValueAsDouble(sumBankCodes),
		AB_Value_GetValueAsDouble(val));
      AB_Value_free(val);
      GWEN_Buffer_free(tmp);
      return -1;
    }
    else {
      AB_Value_free(val);
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "Bank code checksum ok");
    }
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No bank code checksum");
  }

  /* checksum of EUR values */
  GWEN_Buffer_Reset(tmp);
  if (AHB_DTAUS__ReadWord(src, tmp, pos+64, 13)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading value at %d", pos+64);
    GWEN_Buffer_free(tmp);
    return -1;
  }
  if (GWEN_Buffer_GetUsedBytes(tmp)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "EUR checksum: %s",
	      GWEN_Buffer_GetStart(tmp));
    GWEN_Buffer_AppendString(tmp, "/100");
    val=AB_Value_fromString(GWEN_Buffer_GetStart(tmp));
    if (val==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad value at %d", pos+64);
      GWEN_Buffer_free(tmp);
      return -1;
    }

    if (AB_Value_Compare(sumEUR, val)!=0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Bad EUR checksum (is %.2f, should be %.2f)",
		AB_Value_GetValueAsDouble(sumEUR),
		AB_Value_GetValueAsDouble(val));
      AB_Value_free(val);
      GWEN_Buffer_free(tmp);
      return -1;
    }
    else {
      AB_Value_free(val);
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "EUR checksum ok");
    }
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No EUR checksum");
  }

  GWEN_Buffer_free(tmp);
  return 128;
}



int AHB_DTAUS__ReadDocument(GWEN_BUFFER *src,
                            unsigned int pos,
                            GWEN_DB_NODE *cfg) {
  GWEN_DB_NODE *dcfg=0;
  GWEN_DB_NODE *xa;
  int rv;
  unsigned int cSets;
  AB_VALUE *sumEUR;
  AB_VALUE *sumDEM;
  AB_VALUE *sumBankCodes;
  AB_VALUE *sumAccountIds;
  int hasESet;
  int sn;
  int isDebitNote;
  const char *p;

  /* preset */
  hasESet=0;
  dcfg=0;
  cSets=0;
  sumEUR=AB_Value_new();
  sumDEM=AB_Value_new();
  sumBankCodes=AB_Value_new();
  sumAccountIds=AB_Value_new();

  /* read A set */
  DBG_INFO(AQBANKING_LOGDOMAIN, "Reading A set (pos=%d)", pos);
  GWEN_Buffer_SetPos(src, pos+4);

  sn=GWEN_Buffer_PeekByte(src);
  if (sn==-1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few data");
    AB_Value_free(sumAccountIds);
    AB_Value_free(sumBankCodes);
    AB_Value_free(sumDEM);
    AB_Value_free(sumEUR);
    return -1;
  }

  if (sn=='A') {
    /* create template */
    dcfg=GWEN_DB_Group_new("dcfg");
    rv=AHB_DTAUS__ParseSetA(src, pos, dcfg);
    if (rv==-1) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in A set");
      AB_Value_free(sumAccountIds);
      AB_Value_free(sumBankCodes);
      AB_Value_free(sumDEM);
      AB_Value_free(sumEUR);
      GWEN_DB_Group_free(dcfg);
      return -1;
    }
    pos+=rv;
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "DTAUS record does not start with an A set");
    AB_Value_free(sumAccountIds);
    AB_Value_free(sumBankCodes);
    AB_Value_free(sumDEM);
    AB_Value_free(sumEUR);
    return -1;
  }

  isDebitNote=(strcasecmp(GWEN_DB_GetCharValue(dcfg, "type", 0, "transfer"),
                          "debitnote")==0);

  /* now read C sets */
  for (;;) {
    GWEN_Buffer_SetPos(src, pos+4);
    sn=GWEN_Buffer_PeekByte(src);
    if (sn==-1) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few data");
      AB_Value_free(sumAccountIds);
      AB_Value_free(sumBankCodes);
      AB_Value_free(sumDEM);
      AB_Value_free(sumEUR);
      GWEN_DB_Group_free(dcfg);
      return -1;
    }
    if (sn=='C') {
      GWEN_DB_NODE *dbDate;

      cSets++;
      DBG_INFO(AQBANKING_LOGDOMAIN, "Reading C set (pos=%d)", pos);
      if (isDebitNote)
        xa=GWEN_DB_Group_new("debitnote");
      else
        xa=GWEN_DB_Group_new("transfer");
      GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT,
                           "value/currency",
                           GWEN_DB_GetCharValue(dcfg, "currency", 0, "EUR"));
      p=GWEN_DB_GetCharValue(dcfg, "localBankCode", 0, 0);
      if (!p)
        p=GWEN_DB_GetCharValue(cfg, "bankCode", 0, 0);
      if (p)
        GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT,
                             "localBankCode", p);
      p=GWEN_DB_GetCharValue(dcfg, "localAccountNumber", 0, 0);
      if (!p)
        p=GWEN_DB_GetCharValue(cfg, "acccountId", 0, 0);
      if (p)
        GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT,
                             "localAccountNumber", p);
      p=GWEN_DB_GetCharValue(cfg, "name", 0, 0);
      if (p)
        GWEN_DB_SetCharValue(xa, GWEN_DB_FLAGS_DEFAULT,
                             "localName", p);

      dbDate=GWEN_DB_GetGroup(dcfg, GWEN_PATH_FLAGS_NAMEMUSTEXIST,"execDate");
      if (!dbDate)
        dbDate=GWEN_DB_GetGroup(dcfg, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "date");
      if (dbDate) {
        GWEN_DB_NODE *dbX;

        dbX=GWEN_DB_GetGroup(xa, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                             "date");
        assert(dbX);
        GWEN_DB_AddGroupChildren(dbX, dbDate);
      }

      rv=AHB_DTAUS__ParseSetC(src, pos, xa,
			      sumEUR,
                              sumDEM,
                              sumBankCodes,
                              sumAccountIds);
      if (rv==-1) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in C set");
        GWEN_DB_Group_free(xa);
        GWEN_DB_Group_free(dcfg);
	AB_Value_free(sumAccountIds);
	AB_Value_free(sumBankCodes);
	AB_Value_free(sumDEM);
	AB_Value_free(sumEUR);
	return -1;
      }
      DBG_INFO(AQBANKING_LOGDOMAIN, "Size of C set was %d", rv);
      pos+=rv;
      GWEN_DB_AddGroup(cfg, xa);
    } /* if C set */
    else if (sn=='E') {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Reading E set (pos=%d)", pos);
      /* E set, check */
      rv=AHB_DTAUS__ParseSetE(src, pos,
                              cSets,
                              sumEUR,
                              sumDEM,
                              sumBankCodes,
                              sumAccountIds);
      if (rv==-1) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in E set");
        GWEN_DB_Group_free(dcfg);
	AB_Value_free(sumAccountIds);
	AB_Value_free(sumBankCodes);
	AB_Value_free(sumDEM);
	AB_Value_free(sumEUR);
	return -1;
      }
      hasESet=1;
      DBG_INFO(AQBANKING_LOGDOMAIN, "Size of E set was %d", rv);
      pos+=rv;
      break;
    } /* if E set */
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown set \"%c\" at %d",
                sn, pos+4);
      GWEN_DB_Group_free(dcfg);
      AB_Value_free(sumAccountIds);
      AB_Value_free(sumBankCodes);
      AB_Value_free(sumDEM);
      AB_Value_free(sumEUR);
      return -1;
    }
  } /* for */

  if (!hasESet) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "DTAUS record does not have an E set");
  }

  GWEN_DB_Group_free(dcfg);

  dcfg=GWEN_DB_GetGroup(cfg, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "summary");
  assert(dcfg);

  GWEN_DB_SetIntValue(dcfg, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "cSets", cSets);
  GWEN_DB_SetIntValue(dcfg, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "isDebitNote", isDebitNote);


  AB_Value_free(sumAccountIds);
  AB_Value_free(sumBankCodes);
  AB_Value_free(sumDEM);
  AB_Value_free(sumEUR);

  return pos;
}



int AHB_DTAUS__Import(GWEN_DBIO *dbio,
		      GWEN_SYNCIO *sio,
                      GWEN_DB_NODE *data,
		      GWEN_DB_NODE *cfg,
		      uint32_t flags) {
  GWEN_BUFFER *src;
  int rv;
  unsigned int pos;

  src=GWEN_Buffer_new(0, 1024, 0, 1);
  GWEN_Buffer_AddMode(src, GWEN_BUFFER_MODE_USE_SYNCIO);
  GWEN_Buffer_SetSourceSyncIo(src, sio, 0);

  pos=0;
  rv=0;
  rv=AHB_DTAUS__ReadDocument(src, pos, data);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error reading DTAUS record (%d)", rv);
  }
  else {
    /*GWEN_DB_Dump(data, stderr, 3);*/
  }

  GWEN_Buffer_free(src);
  return rv;
}




GWEN_DBIO_CHECKFILE_RESULT AHB_DTAUS__ReallyCheckFile(GWEN_BUFFER *src,
                                                      unsigned int pos) {
  int sn;
  int rv;

  /* read A set */
  DBG_INFO(AQBANKING_LOGDOMAIN, "Checking for A set (pos=%d)",
           pos);
  GWEN_Buffer_SetPos(src, pos+4);
  sn=GWEN_Buffer_PeekByte(src);
  if (sn==-1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few data");
    return GWEN_DBIO_CheckFileResultNotOk;
  }

  if (sn=='A') {
    GWEN_DB_NODE *dcfg;

    /* create template */
    dcfg=GWEN_DB_Group_new("dcfg");
    rv=AHB_DTAUS__ParseSetA(src, pos, dcfg);
    GWEN_DB_Group_free(dcfg);
    if (rv==-1) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in A set");
      return GWEN_DBIO_CheckFileResultNotOk;
    }
    pos+=rv;
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "DTAUS record does not start with an A set");
    return GWEN_DBIO_CheckFileResultNotOk;
  }

  return GWEN_DBIO_CheckFileResultOk;
}



GWEN_DBIO_CHECKFILE_RESULT AHB_DTAUS__CheckFile(GWEN_DBIO *dbio, const char *fname) {
  GWEN_BUFFER *src;
  GWEN_DBIO_CHECKFILE_RESULT rv;
  int rv_int;
  unsigned int pos;
  GWEN_SYNCIO *sio;

  assert(dbio);
  assert(fname);

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv_int = GWEN_SyncIo_Connect(sio);
  if (rv_int < 0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv_int);
    GWEN_SyncIo_free(sio);
    return GWEN_DBIO_CheckFileResultNotOk;
  }

  src=GWEN_Buffer_new(0, 1024, 0, 1);
  GWEN_Buffer_AddMode(src, GWEN_BUFFER_MODE_USE_SYNCIO);
  GWEN_Buffer_SetSourceSyncIo(src, sio, 0);

  pos=0;
  rv=AHB_DTAUS__ReallyCheckFile(src, pos);

  GWEN_Buffer_free(src);
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  return rv;
}





