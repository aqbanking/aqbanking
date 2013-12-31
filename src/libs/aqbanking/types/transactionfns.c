/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004-2010 by Martin Preuss
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



int AB_Transaction_ValidatePurposeAgainstLimits(AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  const GWEN_STRINGLIST *sl;
  int maxn;
  int maxs;
  int n;

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
  sl=AB_Transaction_GetPurpose(t);
  n=0;
  if (sl) {
    GWEN_STRINGLISTENTRY *se;
    GWEN_STRINGLIST *nsl;
    const char *p;

    nsl=GWEN_StringList_new();
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
	  GWEN_StringList_free(nsl);
	  return GWEN_ERROR_INVALID;
        }
        else {
          char *np;
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
            GWEN_StringList_free(nsl);
            return GWEN_ERROR_INVALID;
          }
          np=(char*)GWEN_Memory_malloc(l+1);
          memmove(np, GWEN_Buffer_GetStart(tbuf), l+1);
          GWEN_Buffer_free(tbuf);
          /* let string list take the newly alllocated string */
          GWEN_StringList_AppendString(nsl, np, 1, 0);
        }
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
    AB_Transaction_SetPurpose(t, nsl);
    GWEN_StringList_free(nsl);
  }
  if (!n) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No purpose lines");
    return GWEN_ERROR_INVALID;
  }

  return 0;
}



int AB_Transaction_ValidateNamesAgainstLimits(AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  const GWEN_STRINGLIST *sl;
  int maxn;
  int maxs;
  int n;
  const char *s;

  /* check remote name */
  if (lim) {
    maxn=AB_TransactionLimits_GetMaxLinesRemoteName(lim);
    maxs=AB_TransactionLimits_GetMaxLenRemoteName(lim);
  }
  else {
    maxn=0;
    maxs=0;
  }
  sl=AB_Transaction_GetRemoteName(t);
  n=0;
  if (sl) {
    GWEN_STRINGLISTENTRY *se;
    GWEN_STRINGLIST *nsl;
    const char *p;

    nsl=GWEN_StringList_new();
    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      p=GWEN_StringListEntry_Data(se);
      if (p && *p) {
	n++;
	if (maxn && n>maxn) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Too many remote name lines (%d>%d)",
		    n, maxn);
          GWEN_StringList_free(nsl);
	  return GWEN_ERROR_INVALID;
        }
        else {
          GWEN_BUFFER *tbuf;
          char *np;
          int l;

          tbuf=GWEN_Buffer_new(0, 256, 0, 1);
          AB_ImExporter_Utf8ToDta(p, -1, tbuf);
          GWEN_Text_CondenseBuffer(tbuf);
          l=GWEN_Buffer_GetUsedBytes(tbuf);
          if (maxs>0 && l>maxs) {
            DBG_ERROR(AQBANKING_LOGDOMAIN,
                      "Too many chars in remote name line %d (%d>%d)",
                      n, l, maxs);
            GWEN_Buffer_free(tbuf);
            GWEN_StringList_free(nsl);
            return GWEN_ERROR_INVALID;
          }
          np=(char*)GWEN_Memory_malloc(l+1);
          memmove(np, GWEN_Buffer_GetStart(tbuf), l+1);
          GWEN_Buffer_free(tbuf);
          /* let string list take the newly alllocated string */
          GWEN_StringList_AppendString(nsl, np, 1, 0);
        }
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
    AB_Transaction_SetRemoteName(t, nsl);
    GWEN_StringList_free(nsl);
  }
  if (!n) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No remote name lines");
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
    AB_Transaction_SetLocalName(t, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing local name");
    return GWEN_ERROR_INVALID;
  }


  return 0;
}



int AB_Transaction_ValidateTextKeyAgainstLimits(AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim, int defaultTextKey) {
  int n;

  /* check text key */
  if (lim) {
    if (GWEN_StringList_Count(AB_TransactionLimits_GetValuesTextKey(lim))){
      char numbuf[32];

      n=AB_Transaction_GetTextKey(t);
      if (n==0) {
        n=defaultTextKey;
        AB_Transaction_SetTextKey(t, n);
      }

      snprintf(numbuf, sizeof(numbuf), "%d", n);
      if (!AB_TransactionLimits_HasValuesTextKey(lim, numbuf)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Text key \"%s\" not supported by bank", numbuf);
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Text key \"%d\" not supported by the bank"),
                              n);
        return GWEN_ERROR_INVALID;
      }
    }
  }

  return 0;
}



int AB_Transaction_ValidateRecurrenceAgainstLimits(AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  if (lim) {
    /* check period */
    if (AB_Transaction_GetPeriod(t)==AB_Transaction_PeriodMonthly) {
      const GWEN_STRINGLIST *sl;

      /* check cycle */
      sl=AB_TransactionLimits_GetValuesCycleMonth(lim);
      if (GWEN_StringList_Count(sl)){
        char numbuf[32];
        int n;

        n=AB_Transaction_GetCycle(t);
        if (n==0) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "No cycle given");
          return GWEN_ERROR_INVALID;
        }

        snprintf(numbuf, sizeof(numbuf), "%d", n);
        if (!AB_TransactionLimits_HasValuesCycleMonth(lim, numbuf) &&
            !AB_TransactionLimits_HasValuesCycleMonth(lim, "0")) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Month day \"%s\" not supported by bank", numbuf);
          GWEN_Gui_ProgressLog2(0,
                                GWEN_LoggerLevel_Error,
                                I18N("Month day \"%d\" not supported by bank"),
                                n);
          return GWEN_ERROR_INVALID;
        }
      }

      /* check execution day */
      sl=AB_TransactionLimits_GetValuesExecutionDayMonth(lim);
      if (GWEN_StringList_Count(sl)){
        char numbuf[32];
        int n;

        n=AB_Transaction_GetExecutionDay(t);
        if (n==0) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "No execution day given");
          return GWEN_ERROR_INVALID;
        }

        snprintf(numbuf, sizeof(numbuf), "%d", n);
        if (!AB_TransactionLimits_HasValuesExecutionDayMonth(lim, numbuf) &&
            !AB_TransactionLimits_HasValuesExecutionDayMonth(lim, "0")) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Execution month day \"%s\" not supported by bank",
                    numbuf);
          GWEN_Gui_ProgressLog2(0,
                                GWEN_LoggerLevel_Error,
                                I18N("Execution month day \"%d\" not supported by bank"),
                                n);
          return GWEN_ERROR_INVALID;
        }
      } /* if there are limits */
    }
    else if (AB_Transaction_GetPeriod(t)==AB_Transaction_PeriodWeekly) {
      const GWEN_STRINGLIST *sl;

      /* check cycle */
      sl=AB_TransactionLimits_GetValuesCycleWeek(lim);
      if (GWEN_StringList_Count(sl)) {
        char numbuf[32];
        int n;

        n=AB_Transaction_GetCycle(t);
        if (n==0) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "No cycle given");
          return GWEN_ERROR_INVALID;
        }

        snprintf(numbuf, sizeof(numbuf), "%d", n);
        if (!AB_TransactionLimits_HasValuesCycleWeek(lim, numbuf) &&
            !AB_TransactionLimits_HasValuesCycleWeek(lim, "0")) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Week day \"%s\" not supported by bank",
                    numbuf);
          GWEN_Gui_ProgressLog2(0,
                                GWEN_LoggerLevel_Error,
                                I18N("Week day \"%d\" not supported by bank"),
                                n);
          return GWEN_ERROR_INVALID;
        }
      } /* if there are limits */

      /* check execution day */
      sl=AB_TransactionLimits_GetValuesExecutionDayWeek(lim);
      if (GWEN_StringList_Count(sl)){
        char numbuf[32];
        int n;

        n=AB_Transaction_GetExecutionDay(t);
        if (n==0) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "No execution day given");
          return GWEN_ERROR_INVALID;
        }

        snprintf(numbuf, sizeof(numbuf), "%d", n);
        if (!AB_TransactionLimits_HasValuesExecutionDayWeek(lim, numbuf) &&
            !AB_TransactionLimits_HasValuesExecutionDayWeek(lim, "0")) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Execution month day \"%s\" not supported by bank", numbuf);
          GWEN_Gui_ProgressLog2(0,
                                GWEN_LoggerLevel_Error,
                                I18N("Execution month day \"%d\" not supported by bank"),
                                n);
          return GWEN_ERROR_INVALID;
        }
      } /* if there are limits */
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Unsupported period %d", AB_Transaction_GetPeriod(t));
      return GWEN_ERROR_INVALID;
    }
  } /* if limits */

  return 0;
}



int AB_Transaction_CheckFirstExecutionDateAgainstLimits(AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  if (lim) {
    const GWEN_TIME *ti;

    /* check setup times */
    ti=AB_Transaction_GetFirstExecutionDate(t);
    if (ti) {
      GWEN_TIME *currDate;
      int dt;
      int n;
  
      currDate=GWEN_CurrentTime();
      assert(currDate);
      dt=((int)GWEN_Time_DiffSeconds(ti, currDate))/(60*60*24);
      GWEN_Time_free(currDate);
  
      /* check minimum setup time */
      n=AB_TransactionLimits_GetMinValueSetupTime(lim);
      if (n && dt<n) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Minimum setup time violated");
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
                  "Maximum setup time violated");
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



int AB_Transaction_CheckDateAgainstLimits(AB_TRANSACTION *t, const AB_TRANSACTION_LIMITS *lim) {
  if (lim) {
    const GWEN_TIME *ti;

    ti=AB_Transaction_GetDate(t);
    if (ti) {
      GWEN_TIME *currDate;
      int dt;
      int n;

      currDate=GWEN_CurrentTime();
      assert(currDate);
      dt=((int)GWEN_Time_DiffSeconds(ti, currDate))/(60*60*24);
      GWEN_Time_free(currDate);

      /* check minimum setup time */
      n=AB_TransactionLimits_GetMinValueSetupTime(lim);
      if (n && dt<n) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Minimum setup time violated");
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
                  "Maximum setup time violated");
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










