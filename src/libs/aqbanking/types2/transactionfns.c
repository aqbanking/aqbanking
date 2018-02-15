/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "transaction_p.h"

#include <aqbanking/transactionfns.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/i18n.h>
#include <gwenhywfar/syncio_file.h>

#include <errno.h>
#include <string.h>



#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18N_NOOP(msg) msg
#define I18S(msg) msg



int AB_Transaction_Compare(const AB_TRANSACTION *t1,
                           const AB_TRANSACTION *t0) {
  if (t1==t0)
    return 0;

  if (t1 && t0) {
    GWEN_DB_NODE *dbT;
    GWEN_BUFFER *buf1;
    GWEN_BUFFER *buf0;

    buf1=GWEN_Buffer_new(0, 256, 0, 1);
    buf0=GWEN_Buffer_new(0, 256, 0, 1);

    /* prepare first buffer */
    dbT=GWEN_DB_Group_new("transaction");
    if (AB_Transaction_toDb(t1, dbT)==0) {
      int err;

      /* remove variables from comparison */
      GWEN_DB_DeleteVar(dbT, "status");

      err=GWEN_DB_WriteToBuffer(dbT, buf1, GWEN_DB_FLAGS_COMPACT);
      if (err) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "INTERNAL: Error writing DB to buffer");
	GWEN_Buffer_free(buf0);
	GWEN_Buffer_free(buf1);
	GWEN_DB_Group_free(dbT);
	return -1;
      }
    }
    GWEN_DB_Group_free(dbT);
  
    /* prepare second buffer */
    dbT=GWEN_DB_Group_new("transaction");
    if (AB_Transaction_toDb(t0, dbT)==0) {
      int err;

      /* remove variables from comparison */
      GWEN_DB_DeleteVar(dbT, "status");

      err=GWEN_DB_WriteToBuffer(dbT, buf0, GWEN_DB_FLAGS_COMPACT);
      if (err) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "INTERNAL: Error writing DB to buffer");
	GWEN_Buffer_free(buf0);
	GWEN_Buffer_free(buf1);
	GWEN_DB_Group_free(dbT);
	return -1;
      }
    }
    GWEN_DB_Group_free(dbT);
  
    /* actually compare */
    if (strcasecmp(GWEN_Buffer_GetStart(buf1),
                   GWEN_Buffer_GetStart(buf0))==0) {
      GWEN_Buffer_free(buf0);
      GWEN_Buffer_free(buf1);
      return 0;
    }
    GWEN_Buffer_free(buf0);
    GWEN_Buffer_free(buf1);
  }

  return 1;
}



void AB_Transaction_FillLocalFromAccount(AB_TRANSACTION *t, const AB_ACCOUNT *a) {
  const char *s;

  assert(t);
  assert(a);

  /* local account */
  s=AB_Account_GetCountry(a);
  if (!s || !*s)
    s="de";
  AB_Transaction_SetLocalCountry(t, s);
  AB_Transaction_SetRemoteCountry(t, s);

  s=AB_Account_GetBankCode(a);
  if (s && *s)
    AB_Transaction_SetLocalBankCode(t, s);

  s=AB_Account_GetAccountNumber(a);
  if (s && *s)
    AB_Transaction_SetLocalAccountNumber(t, s);

  s=AB_Account_GetOwnerName(a);
  if (s && *s)
    AB_Transaction_SetLocalName(t, s);

  s=AB_Account_GetBIC(a);
  if (s && *s)
    AB_Transaction_SetLocalBic(t, s);

  s=AB_Account_GetIBAN(a);
  if (s && *s)
    AB_Transaction_SetLocalIban(t, s);
}



int AB_Transaction_CheckPurposeAgainstLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  int maxn;
  int maxs;
  int n;
  const char *sPurpose;

  /* check purpose */
  if (lim) {
    maxn=AB_TransactionLimits_GetMaxLinesPurpose(lim);
    maxs=AB_TransactionLimits_GetMaxLenPurpose(lim);
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No transaction limits");
    maxn=0;
    maxs=0;
  }
  sPurpose=AB_Transaction_GetPurpose(t);
  if (sPurpose && *sPurpose) {
    GWEN_STRINGLIST *sl;

    n=0;
    sl=GWEN_StringList_fromString(sPurpose, "\n", 0);
    if (sl) {
      GWEN_STRINGLISTENTRY *se;
      const char *p;

      se=GWEN_StringList_FirstEntry(sl);
      while(se) {
        p=GWEN_StringListEntry_Data(se);
        if (p && *p) {
          n++;
          if (maxn && n>maxn) {
            DBG_ERROR(AQBANKING_LOGDOMAIN,
                      "Too many purpose lines (%d>%d)", n, maxn);
            GWEN_Gui_ProgressLog2(0,
                                  GWEN_LoggerLevel_Error,
                                  I18N("Too many purpose lines (%d>%d)"),
                                  n, maxn);
            GWEN_StringList_free(sl);
            return GWEN_ERROR_INVALID;
          }
          else {
            int l;
            GWEN_BUFFER *tbuf;

            tbuf=GWEN_Buffer_new(0, maxs, 0, 1);
            AB_ImExporter_Utf8ToDta(p, -1, tbuf);
            GWEN_Text_CondenseBuffer(tbuf);
            l=GWEN_Buffer_GetUsedBytes(tbuf);
            if (maxs && l>maxs) {
              DBG_ERROR(AQBANKING_LOGDOMAIN, "Too many chars in purpose line %d (%d>%d)", n, l, maxs);
              GWEN_Gui_ProgressLog2(0,
                                    GWEN_LoggerLevel_Error,
                                    I18N("Too many chars in purpose line %d (%d>%d)"),
                                    n, l, maxs);
              GWEN_Buffer_free(tbuf);
              GWEN_StringList_free(sl);
              return GWEN_ERROR_INVALID;
            }
            GWEN_Buffer_free(tbuf);
          }
        }
        se=GWEN_StringListEntry_Next(se);
      } /* while */
    }
    GWEN_StringList_free(sl);

    if (!n) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No purpose lines");
      return GWEN_ERROR_INVALID;
    }
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Empty purpose");
    return GWEN_ERROR_INVALID;
  }

  return 0;
}



int AB_Transaction_CheckNamesAgainstLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  int maxs;
  const char *s;

  /* check remote name */
  if (lim)
    maxs=AB_TransactionLimits_GetMaxLenRemoteName(lim);
  else
    maxs=0;
  s=AB_Transaction_GetRemoteName(t);
  if (s && *s) {
    int l;
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    AB_ImExporter_Utf8ToDta(s, -1, tbuf);
    GWEN_Text_CondenseBuffer(tbuf);
    l=GWEN_Buffer_GetUsedBytes(tbuf);
    if (maxs>0 && l>maxs) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Too many chars in remote name (%d>%d)", l, maxs);
      GWEN_Buffer_free(tbuf);
      return GWEN_ERROR_INVALID;
    }
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing remote name");
    return GWEN_ERROR_INVALID;
  }


  /* check local name */
  if (lim)
    maxs=AB_TransactionLimits_GetMaxLenLocalName(lim);
  else
    maxs=0;
  s=AB_Transaction_GetLocalName(t);
  if (s && *s) {
    int l;
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    AB_ImExporter_Utf8ToDta(s, -1, tbuf);
    GWEN_Text_CondenseBuffer(tbuf);
    l=GWEN_Buffer_GetUsedBytes(tbuf);
    if (maxs>0 && l>maxs) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Too many chars in local name (%d>%d)", l, maxs);
      GWEN_Buffer_free(tbuf);
      return GWEN_ERROR_INVALID;
    }
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing local name");
    return GWEN_ERROR_INVALID;
  }

  return 0;
}



int AB_Transaction_CheckRecurrenceAgainstLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  if (lim) {
    int n;
    const uint8_t *aptr;
    int alen;

    /* check period */
    if (AB_Transaction_GetPeriod(t)==AB_Transaction_PeriodMonthly) {
      /* check cycle */
      n=AB_Transaction_GetCycle(t);
      if (n==0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "No cycle given");
        return GWEN_ERROR_INVALID;
      }

      alen=AB_TransactionLimits_GetValuesCycleMonthArraySize();
      aptr=AB_TransactionLimits_GetValuesCycleMonth(lim);
      if (alen && aptr) {
        int i;

        for (i=0; i<alen; i++) {
          if (aptr[i]==n)
            break;
        }
        if (i>=alen) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Month day \"%d\" not supported by bank", n);
          GWEN_Gui_ProgressLog2(0,
                                GWEN_LoggerLevel_Error,
                                I18N("Month day \"%d\" not supported by bank"),
                                n);
          return GWEN_ERROR_INVALID;
        }
      } /* if limits available */

      /* check execution day */
      n=AB_Transaction_GetExecutionDay(t);
      if (n==0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "No execution day given");
        return GWEN_ERROR_INVALID;
      }

      aptr=AB_TransactionLimits_GetValuesExecutionDayMonth(lim);
      alen=AB_TransactionLimits_GetValuesExecutionDayMonthArraySize();
      if (alen && aptr) {
        int i;

        for (i=0; i<alen; i++) {
          if (aptr[i]==n)
            break;
        }
        if (i>=alen) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Execution month day \"%d\" not supported by bank", n);
          GWEN_Gui_ProgressLog2(0,
                                GWEN_LoggerLevel_Error,
                                I18N("Execution month day \"%d\" not supported by bank"),
                                n);
          return GWEN_ERROR_INVALID;
        }
      } /* if limits available */
    }
    else if (AB_Transaction_GetPeriod(t)==AB_Transaction_PeriodWeekly) {
      /* check cycle */
      n=AB_Transaction_GetCycle(t);
      if (n==0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "No cycle given");
        return GWEN_ERROR_INVALID;
      }

      alen=AB_TransactionLimits_GetValuesCycleWeekArraySize();
      aptr=AB_TransactionLimits_GetValuesCycleWeek(lim);
      if (alen && aptr) {
        int i;

        for (i=0; i<alen; i++) {
          if (aptr[i]==n)
            break;
        }
        if (i>=alen) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Week day \"%d\" not supported by bank", n);
          GWEN_Gui_ProgressLog2(0,
                                GWEN_LoggerLevel_Error,
                                I18N("Week day \"%d\" not supported by bank"),
                                n);
          return GWEN_ERROR_INVALID;
        }
      } /* if limits available */

      /* check execution day */
      n=AB_Transaction_GetExecutionDay(t);
      if (n==0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "No execution day given");
        return GWEN_ERROR_INVALID;
      }

      aptr=AB_TransactionLimits_GetValuesExecutionDayWeek(lim);
      alen=AB_TransactionLimits_GetValuesExecutionDayWeekArraySize();
      if (alen && aptr) {
        int i;

        for (i=0; i<alen; i++) {
          if (aptr[i]==n)
            break;
        }
        if (i>=alen) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Execution week day \"%d\" not supported by bank", n);
          GWEN_Gui_ProgressLog2(0,
                                GWEN_LoggerLevel_Error,
                                I18N("Execution week day \"%d\" not supported by bank"),
                                n);
          return GWEN_ERROR_INVALID;
        }
      } /* if limits available */
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Unsupported period %d", AB_Transaction_GetPeriod(t));
      return GWEN_ERROR_INVALID;
    }
  } /* if limits */

  return 0;
}



int AB_Transaction_CheckFirstExecutionDateAgainstLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  if (lim) {
    const GWEN_DATE *dt;

    /* check setup times */
    dt=AB_Transaction_GetFirstDate(t);
    if (dt) {
      GWEN_DATE *currDate;
      int diff;
      int n;
  
      currDate=GWEN_Date_CurrentDate();
      assert(currDate);
      diff=GWEN_Date_Diff(dt, currDate);
      GWEN_Date_free(currDate);

      /* check minimum setup time */
      n=AB_TransactionLimits_GetMinValueSetupTime(lim);
      if (n && diff<n) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Minimum setup time violated (given %d but required min=%d)", diff, n);
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Minimum setup time violated. "
                                   "Dated transactions need to be at least %d days away"),
                              n);
        return GWEN_ERROR_INVALID;
      }
  
      /* check maximum setup time */
      n=AB_TransactionLimits_GetMaxValueSetupTime(lim);
      if (n && diff>n) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Maximum setup time violated (given %d but allowed max=%d)", diff, n);
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Maximum setup time violated. "
                                   "Dated transactions need to be at most %d days away"),
                              n);
        return GWEN_ERROR_INVALID;
      }
    }

  }

  return 0;
}



int AB_Transaction_CheckDateAgainstLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  if (lim) {
    const GWEN_DATE *da;

    da=AB_Transaction_GetDate(t);
    if (da) {
      GWEN_DATE *currDate;
      int dt;
      int n;

      currDate=GWEN_Date_CurrentDate();
      assert(currDate);
      dt=GWEN_Date_Diff(da, currDate);
      GWEN_Date_free(currDate);

      /* check minimum setup time */
      n=AB_TransactionLimits_GetMinValueSetupTime(lim);
      if (n && dt<n) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Minimum setup time violated (given %d but required min=%d)", dt, n);
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Minimum setup time violated. "
                                   "Dated transactions need to be at least %d days away"),
                              n);
        return GWEN_ERROR_INVALID;
      }

      /* check maximum setup time */
      n=AB_TransactionLimits_GetMaxValueSetupTime(lim);
      if (n && dt>n) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Maximum setup time violated (given %d but allowed max=%d)", dt, n);
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Maximum setup time violated. "
                                   "Dated transactions need to be at most %d days away"),
                              n);
        return GWEN_ERROR_INVALID;
      }
    }

  }

  return 0;
}



int AB_Transaction_CheckDateAgainstSequenceLimits(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  if (lim) {
    const GWEN_DATE *da;

    da=AB_Transaction_GetDate(t);
    if (da) {
      GWEN_DATE *currDate;
      int dt;
      int minTime=0;
      int maxTime=0;

      currDate=GWEN_Date_CurrentDate();
      assert(currDate);
      dt=GWEN_Date_Diff(da, currDate);
      GWEN_Date_free(currDate);

      switch(AB_Transaction_GetSequence(t)) {
      case AB_Transaction_SequenceOnce:
	minTime=AB_TransactionLimits_GetMinValueSetupTimeOnce(lim);
	maxTime=AB_TransactionLimits_GetMaxValueSetupTimeOnce(lim);
	break;
      case AB_Transaction_SequenceFirst:
	minTime=AB_TransactionLimits_GetMinValueSetupTimeFirst(lim);
	maxTime=AB_TransactionLimits_GetMaxValueSetupTimeFirst(lim);
	break;
      case AB_Transaction_SequenceFollowing:
	minTime=AB_TransactionLimits_GetMinValueSetupTimeRecurring(lim);
	maxTime=AB_TransactionLimits_GetMaxValueSetupTimeRecurring(lim);
	break;
      case AB_Transaction_SequenceFinal:
	minTime=AB_TransactionLimits_GetMinValueSetupTimeFinal(lim);
	maxTime=AB_TransactionLimits_GetMaxValueSetupTimeFinal(lim);
	break;
      case AB_Transaction_SequenceUnknown:
	break;
      }

      if (minTime==0)
	minTime=AB_TransactionLimits_GetMinValueSetupTime(lim);
      if (maxTime==0)
	maxTime=AB_TransactionLimits_GetMaxValueSetupTime(lim);

      /* check minimum setup time */
      if (minTime && dt<minTime) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Minimum setup time violated (given %d but required min=%d for sequence type=%s)",
                  dt, minTime, AB_Transaction_Sequence_toString(AB_Transaction_GetSequence(t)));
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Minimum setup time violated. "
                                   "Dated transactions need to be at least %d days away but %d days are requested"),
                              minTime, dt);
        return GWEN_ERROR_INVALID;
      }

      /* check maximum setup time */
      if (maxTime && dt>maxTime) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Maximum setup time violated (given %d but allowed max=%d for sequence type=%s)",
                  dt, maxTime, AB_Transaction_Sequence_toString(AB_Transaction_GetSequence(t)));
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Maximum setup time violated. "
                                   "Dated transactions need to be at most %d days away but %d days are requested"),
                              maxTime, dt);
        return GWEN_ERROR_INVALID;
      }
    }

  }

  return 0;
}




static int _checkStringForSepaCharset(const char *s, int restricted) {
  char *ascii = "':?,-(+.)/ &*$%";
#define DTAUSOFFSET 11

  assert(s);
  if (restricted)
    ascii[DTAUSOFFSET] = '\0';

  while(*s) {
    unsigned char c=*s++;

    if (!((c>='A' && c<='Z') ||
          (c>='a' && c<='z') ||
          (c>='0' && c<='9') ||
          (c=='\n') ||
          strchr(ascii, c)!=NULL)) {
      char errchr[7];
      int i = 0;

      if (c == 0xC3 && !restricted) {
	c = *s++;
	switch(c) {
	case 0x84:	/* AE */
	case 0xA4:	/* ae */
	case 0x96:	/* OE */
	case 0xB6:	/* oe */
	case 0x9C:	/* UE */
	case 0xBC:	/* ue */
	case 0x9F:	/* ss */
	  if ((*s & 0xC0) != 0x80)
	    break;
	  /* these are no umlauts, after all, so fall through */

	default:
	  errchr[i++]=0xC3;
	  if ((c & 0xC0) == 0x80)
	    errchr[i++]=c;
	  else
	    /* UTF-8 sequence ended prematurely */
	    s--;
	  break;
	}
      }
      else
	errchr[i++] = c;

      if (i) {
	while((*s & 0xC0) == 0x80)
	  if (i<6)
	    errchr[i++]=*s++;
	  else {
	    i++;
	    s++;
	  }

	if (i<7 && (i>1 || !(c & 0x80))) {
	  errchr[i] = '\0';
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid character in string: '%s'",
		    errchr);
	}
	else {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "String not properly UTF-8 encoded");
	}
	return GWEN_ERROR_BAD_DATA;
      }
    }
  }

  return 0;
}



/* This function does not check full UTF8, it only checks whether the given string contains characters
 * other than "A"-"Z", "a"-"z" and "0"-"9".
 * We don't use isalnum here because I'm not sure how that function handles UTF-8 chars with umlauts...
 */
static int _checkStringForAlNum(const char *s, int lcase) {
  assert(s);
  while(*s) {
    unsigned char c=*s;

    if (!((c>='0' && c<='9') ||
          (c>='A' && c<='Z') ||
          (lcase && c>='a' && c<='z'))) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid character in string: '%c'", c);
      return GWEN_ERROR_BAD_DATA;
    }
    s++;
  }

  return 0;
}



int AB_Transaction_CheckForSepaConformity(const AB_TRANSACTION *t, int restricted) {
  if (t) {
    const char *s;
    int rv;

    s=AB_Transaction_GetLocalIban(t);
    if (!(s && *s)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty local IBAN in transaction");
      return GWEN_ERROR_BAD_DATA;
    }
    rv=_checkStringForAlNum(s, 1);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid character in local IBAN");
      return rv;
    }

    s=AB_Transaction_GetLocalBic(t);
    if (!(s && *s)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty local BIC in transaction");
      return GWEN_ERROR_BAD_DATA;
    }
    rv=_checkStringForAlNum(s, 0);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid character in local BIC");
      return rv;
    }

    s=AB_Transaction_GetRemoteIban(t);
    if (!(s && *s)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty remote IBAN in transaction");
      return GWEN_ERROR_BAD_DATA;
    }
    rv=_checkStringForAlNum(s, 1);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid character in remote IBAN");
      return rv;
    }

    /* no longer enforce remote BIC */
    s=AB_Transaction_GetRemoteBic(t);
    if (s && *s) {
      rv=_checkStringForAlNum(s, 0);
      if (rv<0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid character in remote BIC");
        return rv;
      }
    }


    s=AB_Transaction_GetLocalName(t);
    if (!(s && *s)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty local name in transaction");
      return GWEN_ERROR_BAD_DATA;
    }
    rv=_checkStringForSepaCharset(s, restricted);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid character in local name");
      return rv;
    }

    s=AB_Transaction_GetRemoteName(t);
    if (!(s && *s)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty remote name in transaction");
      return GWEN_ERROR_BAD_DATA;
    }
    rv=_checkStringForSepaCharset(s, restricted);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid character in remote name");
      return rv;
    }

    /* check purpose, if any */
    s=AB_Transaction_GetPurpose(t);
    if (!(s && *s)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty purpose in transaction, ignoring");
    }
    else {
      rv=_checkStringForSepaCharset(s, restricted);
      if (rv<0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid character in remote name");
        return rv;
      }
    }
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing transaction");
    return GWEN_ERROR_BAD_DATA;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN, "Transaction conforms to restricted SEPA charset");
  return 0;
}



int AB_Transaction_WriteToFile(const AB_TRANSACTION *t, const char *tFile) {
  GWEN_DB_NODE *dbCtx;
  GWEN_SYNCIO *sio;
  int rv;

  dbCtx=GWEN_DB_Group_new("context");
  rv=AB_Transaction_toDb(t, dbCtx);
  if (rv<0) {
    DBG_ERROR(0, "Error transaction context to db");
    return rv;
  }

  if (tFile==NULL) {
    sio=GWEN_SyncIo_File_fromStdout();
    GWEN_SyncIo_AddFlags(sio,
			 GWEN_SYNCIO_FLAGS_DONTCLOSE |
			 GWEN_SYNCIO_FILE_FLAGS_WRITE);
  }
  else {
    sio=GWEN_SyncIo_File_new(tFile, GWEN_SyncIo_File_CreationMode_CreateAlways);
    GWEN_SyncIo_AddFlags(sio,
			 GWEN_SYNCIO_FILE_FLAGS_READ |
			 GWEN_SYNCIO_FILE_FLAGS_WRITE |
			 GWEN_SYNCIO_FILE_FLAGS_UREAD |
			 GWEN_SYNCIO_FILE_FLAGS_UWRITE |
			 GWEN_SYNCIO_FILE_FLAGS_GREAD |
			 GWEN_SYNCIO_FILE_FLAGS_GWRITE);
    rv=GWEN_SyncIo_Connect(sio);
    if (rv<0) {
      DBG_ERROR(0, "Error selecting output file: %s",
		strerror(errno));
      GWEN_SyncIo_free(sio);
      return rv;
    }
  }

  rv=GWEN_DB_WriteToIo(dbCtx, sio, GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    DBG_ERROR(0, "Error writing context (%d)", rv);
    GWEN_DB_Group_free(dbCtx);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  GWEN_DB_Group_free(dbCtx);

  return 0;
}



