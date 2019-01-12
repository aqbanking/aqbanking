/***************************************************************************
 begin       : Thu Oct 04 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */



int AB_Banking_CheckTransactionAgainstLimits_Purpose(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  int maxn;
  int maxs;
  const char *purpose;

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

  purpose=AB_Transaction_GetPurpose(t);
  if (purpose && *purpose) {
    GWEN_STRINGLIST *sl;

    sl=GWEN_StringList_fromString(purpose, "\n", 0);
    if (sl && GWEN_StringList_Count(sl)) {
      int n;
      GWEN_STRINGLISTENTRY *se;
      const char *p;

      n=0;
      se=GWEN_StringList_FirstEntry(sl);
      while(se) {
        p=GWEN_StringListEntry_Data(se);
        if (p && *p) {
          n++;
          if (maxn && n>maxn) {
            DBG_ERROR(AQBANKING_LOGDOMAIN, "Too many purpose lines (%d>%d)", n, maxn);
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
      if (!n) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "No purpose lines");
        GWEN_StringList_free(sl);
        return GWEN_ERROR_INVALID;
      }
    }
    GWEN_StringList_free(sl);
  }
  return 0;
}



int AB_Banking_CheckTransactionAgainstLimits_Names(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
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





int AB_Banking_CheckTransactionAgainstLimits_Recurrence(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  if (lim && (AB_Transaction_GetCommand(t)!=AB_Transaction_CommandSepaDeleteStandingOrder)) {
    /* check period */
    if (AB_Transaction_GetPeriod(t)==AB_Transaction_PeriodMonthly) {
      int n;

      n=AB_Transaction_GetCycle(t);
      if (n==0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "No cycle given");
        return GWEN_ERROR_INVALID;
      }

      if (AB_TransactionLimits_GetValuesCycleMonthUsed(lim) &&
          !AB_TransactionLimits_ValuesCycleMonthHas(lim, n) &&
          !AB_TransactionLimits_ValuesCycleMonthHas(lim, 0)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Month day \"%d\" not supported by bank", n);
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Month day \"%d\" not supported by bank"),
                              n);
        return GWEN_ERROR_INVALID;
      }

      /* check execution day */
      n=AB_Transaction_GetExecutionDay(t);
      if (n==0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "No execution day given");
        return GWEN_ERROR_INVALID;
      }

      if (AB_TransactionLimits_GetValuesExecutionDayMonthUsed(lim) &&
          !AB_TransactionLimits_ValuesExecutionDayMonthHas(lim, n) &&
          !AB_TransactionLimits_ValuesExecutionDayMonthHas(lim, 0)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Execution month day \"%d\" not supported by bank", n);
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Execution month day \"%d\" not supported by bank"),
                              n);
        return GWEN_ERROR_INVALID;
      }
    } /* if (AB_Transaction_GetPeriod(t)==AB_Transaction_PeriodMonthly) */
    else if (AB_Transaction_GetPeriod(t)==AB_Transaction_PeriodWeekly) {
      int n;

      n=AB_Transaction_GetCycle(t);
      if (n==0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "No cycle given");
        return GWEN_ERROR_INVALID;
      }

      if (AB_TransactionLimits_GetValuesCycleWeekUsed(lim) &&
          !AB_TransactionLimits_ValuesCycleWeekHas(lim, n) &&
          !AB_TransactionLimits_ValuesCycleWeekHas(lim, 0)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Week day \"%d\" not supported by bank", n);
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Week day \"%d\" not supported by bank"),
                              n);
        return GWEN_ERROR_INVALID;
      }

      /* check execution day */
      n=AB_Transaction_GetExecutionDay(t);
      if (n==0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "No execution day given");
        return GWEN_ERROR_INVALID;
      }

      if (AB_TransactionLimits_GetValuesExecutionDayWeekUsed(lim) &&
          !AB_TransactionLimits_ValuesExecutionDayWeekHas(lim, n) &&
          !AB_TransactionLimits_ValuesExecutionDayWeekHas(lim, 0)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Execution week day \"%d\" not supported by bank", n);
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Execution week day \"%d\" not supported by bank"),
                              n);
        return GWEN_ERROR_INVALID;
      }
    } /* if (AB_Transaction_GetPeriod(t)==AB_Transaction_PeriodWeekly) */
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unsupported period %d", AB_Transaction_GetPeriod(t));
      return GWEN_ERROR_INVALID;
    }
  } /* if limits */

  return 0;
}



int AB_Banking_CheckTransactionAgainstLimits_ExecutionDate(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
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
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Minimum setup time violated (given %d but required min=%d)", diff, n);
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
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Maximum setup time violated (given %d but allowed max=%d)", diff, n);
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



int AB_Banking_CheckTransactionAgainstLimits_Date(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  if (lim) {
    const GWEN_DATE *dt;

    dt=AB_Transaction_GetDate(t);
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
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Minimum setup time violated (given %d but required min=%d)", diff, n);
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
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Maximum setup time violated (given %d but allowed max=%d)", diff, n);
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



int AB_Banking_CheckTransactionAgainstLimits_Sequence(const AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  if (lim) {
    const GWEN_DATE *dt;

    dt=AB_Transaction_GetDate(t);
    if (dt) {
      GWEN_DATE *currDate;
      int diff;
      int minTime=0;
      int maxTime=0;

      currDate=GWEN_Date_CurrentDate();
      assert(currDate);
      diff=GWEN_Date_Diff(dt, currDate);
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
      if (minTime && diff<minTime) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Minimum setup time violated (given %d but required min=%d for sequence type=%s)",
                  diff, minTime, AB_Transaction_Sequence_toString(AB_Transaction_GetSequence(t)));
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Minimum setup time violated. "
                                   "Dated transactions need to be at least %d days away but %d days are requested"),
                              minTime, dt);
        return GWEN_ERROR_INVALID;
      }

      /* check maximum setup time */
      if (maxTime && diff>maxTime) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Maximum setup time violated (given %d but allowed max=%d for sequence type=%s)",
                  diff, maxTime, AB_Transaction_Sequence_toString(AB_Transaction_GetSequence(t)));
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
  char ascii_chars[]="'&*$%:?,-(+.)/ "; /* last is a blank! */
  const char *ascii;

#define RESTRICTED_CHARS_OFFSET 3

  assert(s);

  ascii=ascii_chars;
  if (restricted)
    ascii+=RESTRICTED_CHARS_OFFSET;

  while(*s) {
    unsigned char c=*s++;

    if (!((c>='A' && c<='Z') ||
          (c>='a' && c<='z') ||
          (c>='0' && c<='9') ||
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



int AB_Banking_CheckTransactionForSepaConformity(const AB_TRANSACTION *t, int restricted) {
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

    s=AB_Transaction_GetRemoteBic(t);
    if (!(s && *s)) {
      if (strncmp(AB_Transaction_GetLocalIban(t), AB_Transaction_GetRemoteIban(t), 2)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty remote BIC in transaction");
        return GWEN_ERROR_BAD_DATA;
      }
    }
    else {
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
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing transaction");
    return GWEN_ERROR_BAD_DATA;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN, "Transaction conforms to restricted SEPA charset");
  return 0;
}



void AB_Banking_FillTransactionFromAccountSpec(AB_TRANSACTION *t, const AB_ACCOUNT_SPEC *as) {
  const char *s;

  assert(t);
  assert(as);

  /* unique account id */
  AB_Transaction_SetUniqueAccountId(t, AB_AccountSpec_GetUniqueId(as));

  /* local account */
  s=AB_AccountSpec_GetCountry(as);
  if (!s || !*s)
    s="de";
  AB_Transaction_SetLocalCountry(t, s);
  AB_Transaction_SetRemoteCountry(t, s);

  s=AB_AccountSpec_GetBankCode(as);
  if (s && *s)
    AB_Transaction_SetLocalBankCode(t, s);

  s=AB_AccountSpec_GetAccountNumber(as);
  if (s && *s)
    AB_Transaction_SetLocalAccountNumber(t, s);

  s=AB_AccountSpec_GetOwnerName(as);
  if (s && *s)
    AB_Transaction_SetLocalName(t, s);

  s=AB_AccountSpec_GetBic(as);
  if (s && *s)
    AB_Transaction_SetLocalBic(t, s);

  s=AB_AccountSpec_GetIban(as);
  if (s && *s)
    AB_Transaction_SetLocalIban(t, s);
}







