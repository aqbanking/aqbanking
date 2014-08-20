/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobeutransfer_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include <aqhbci/account.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/text.h>

#include <aqbanking/jobeutransfer.h>
#include <aqbanking/jobeutransfer_be.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>


GWEN_INHERIT(AH_JOB, AH_JOB_EUTRANSFER);



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_EuTransfer_new(AB_USER *u,
			      AB_ACCOUNT *account) {
  AH_JOB *j;

  j=AH_Job_EuTransferBase_new(u, account, 1);
  if (j!=NULL)
    AH_Job_SetChallengeClass(j, 20);

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_EuTransferBase_new(AB_USER *u,
                                  AB_ACCOUNT *account,
                                  int isTransfer) {
  AH_JOB *j;
  AH_JOB_EUTRANSFER *aj;
  GWEN_DB_NODE *dbArgs;

  j=AH_AccountJob_new("JobEuTransfer",
                      u, account);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_EUTRANSFER, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_EUTRANSFER, j, aj,
                       AH_Job_EuTransfer_FreeData);
  aj->isTransfer=isTransfer;
  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_EuTransfer_Process);
  AH_Job_SetExchangeFn(j, AH_Job_EuTransfer_Exchange);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);


  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_EuTransfer_FreeData(void *bp, void *p){
  AH_JOB_EUTRANSFER *aj;

  aj=(AH_JOB_EUTRANSFER*)p;

  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_EuTransfer_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_EUTRANSFER *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_EUTRANSFER, j);
  assert(aj);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing %s",
           "JobEuTransfer");

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_EuTransfer__ValidateTransfer(AB_JOB *bj,
                                        AH_JOB *mj,
                                        AB_TRANSACTION *t) {
  const GWEN_STRINGLIST *sl;
  int maxn;
  int maxs;
  int n;
  const char *s;
  AH_JOB_EUTRANSFER *aj;
  const AB_EUTRANSFER_INFO *ei=0;
  const AB_TRANSACTION_LIMITS *lim=0;

  assert(mj);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_EUTRANSFER, mj);
  assert(aj);

  /* get country info (check for IBAN) */
  s=AB_Transaction_GetRemoteIban(t);
  if (s) {
    char cnt[3];

    if (AB_JobEuTransfer_GetIbanAllowed(bj)) {
      memmove(cnt, s, 2);
      cnt[2]=0;
      ei=AB_JobEuTransfer_FindCountryInfo(bj, cnt);
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "IBAN not allowed for this account");
      return GWEN_ERROR_INVALID;
    }
  }
  else {
    s=AB_Transaction_GetRemoteCountry(t);
    if (!s) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "remote country code not set");
      return GWEN_ERROR_INVALID;
    }
    ei=AB_JobEuTransfer_FindCountryInfo(bj, s);
  }

  if (ei)
    lim=AB_EuTransferInfo_GetFieldLimits(ei);

  /* check purpose */
  if (lim) {
    maxn=AB_TransactionLimits_GetMaxLinesPurpose(lim);
    if (maxn==-1)
      maxn=1;
    maxs=AB_TransactionLimits_GetMaxLenPurpose(lim);
    if (maxs==-1)
      maxs=27;
  }
  else {
    maxn=1;
    maxs=27;
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
        char *np;
        int l;
        GWEN_BUFFER *tbuf;

        n++;
        if (maxn!=-1 && n>maxn) {
          DBG_WARN(AQHBCI_LOGDOMAIN,
                   "Too many purpose lines (%d>%d), cutting off", n, maxn);
          break;
        }
        tbuf=GWEN_Buffer_new(0, maxs, 0, 1);
        AB_ImExporter_Utf8ToDta(p, -1, tbuf);
        l=GWEN_Buffer_GetUsedBytes(tbuf);
        if (l>maxs) {
          DBG_WARN(AQHBCI_LOGDOMAIN,
                   "Too many chars in line %d (%d>27), cutting off", n, l);
          l=maxs;
        }
	np=(char*)malloc(l+1);
        memmove(np, GWEN_Buffer_GetStart(tbuf), l+1);
        GWEN_Buffer_free(tbuf);
        /* let string list take the newly alllocated string */
        GWEN_StringList_AppendString(nsl, np, 1, 0);
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
    AB_Transaction_SetPurpose(t, nsl);
  }

  /* check remote name */
  maxn=1;
  if (lim) {
    maxn=AB_TransactionLimits_GetMaxLinesRemoteName(lim);
    if (maxn==-1)
      maxn=1;
    maxs=AB_TransactionLimits_GetMaxLenRemoteName(lim);
    if (maxs==-1)
      maxs=27;
  }
  else {
    maxs=27;
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
	char *np;
	int l;
        GWEN_BUFFER *tbuf;

	n++;
	if (maxn!=-1 && n>maxn) {
	  DBG_WARN(AQHBCI_LOGDOMAIN,
		   "Too many remote name lines (%d>%d), cutting off",
		   n, maxn);
	  break;
	}
        tbuf=GWEN_Buffer_new(0, maxs, 0, 1);
        AB_ImExporter_Utf8ToDta(p, -1, tbuf);
	l=GWEN_Buffer_GetUsedBytes(tbuf);
	if (l>maxs) {
	  DBG_WARN(AQHBCI_LOGDOMAIN,
		   "Too many chars in line %d (%d>27), cutting off", n, l);
	  l=maxs;
	}
	np=(char*)malloc(l+1);
	memmove(np, GWEN_Buffer_GetStart(tbuf), l+1);
	GWEN_Buffer_free(tbuf);
	/* let string list take the newly alllocated string */
	GWEN_StringList_AppendString(nsl, np, 1, 0);
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
    AB_Transaction_SetRemoteName(t, nsl);
  }

  /* check local name */
  s=AB_Transaction_GetLocalName(t);
  if (!s) {
    AB_ACCOUNT *a;

    DBG_WARN(AQHBCI_LOGDOMAIN,
	     "No local name, filling in");
    a=AB_Job_GetAccount(bj);
    assert(a);
    s=AB_Account_GetOwnerName(a);
    if (!s) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"No owner name in account, giving up");
      return GWEN_ERROR_INVALID;
    }
    AB_Transaction_SetLocalName(t, s);
  }

  s=AB_Transaction_GetLocalName(t);
  if (s) {
    int l;
    GWEN_BUFFER *tbuf;

    if (lim) {
      maxs=AB_TransactionLimits_GetMaxLenRemoteName(lim);
      if (maxs==-1)
        maxs=27;
    }
    else {
      maxs=27;
    }

    tbuf=GWEN_Buffer_new(0, maxs, 0, 1);
    AB_ImExporter_Utf8ToDta(s, -1, tbuf);
    l=GWEN_Buffer_GetUsedBytes(tbuf);
    if (l>maxs) {
      DBG_WARN(AQHBCI_LOGDOMAIN,
               "Too many chars in local name (%d>27), cutting off", l);
      GWEN_Buffer_Crop(tbuf, 0, maxs);
    }
    AB_Transaction_SetLocalName(t, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  /* check local bank code */
  s=AB_Transaction_GetLocalBankCode(t);
  if (!s) {
    AB_ACCOUNT *a;

    DBG_WARN(AQHBCI_LOGDOMAIN,
	     "No local bank code, filling in");
    a=AH_AccountJob_GetAccount(mj);
    assert(a);
    s=AB_Account_GetBankCode(a);
    assert(s);
    AB_Transaction_SetLocalBankCode(t, s);
  }

  /* check local account number */
  s=AB_Transaction_GetLocalAccountNumber(t);
  if (!s) {
    AB_ACCOUNT *a;

    DBG_WARN(AQHBCI_LOGDOMAIN,
	     "No local account number, filling in");
    a=AH_AccountJob_GetAccount(mj);
    assert(a);
    s=AB_Account_GetAccountNumber(a);
    assert(s);
    AB_Transaction_SetLocalAccountNumber(t, s);
  }

  /* check local account suffix */
  s=AB_Transaction_GetLocalSuffix(t);
  if (!s) {
    AB_ACCOUNT *a;

    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "No local suffix, filling in (if possible)");
    a=AH_AccountJob_GetAccount(mj);
    assert(a);
    s=AB_Account_GetSubAccountId(a);
    if (s && *s)
      AB_Transaction_SetLocalSuffix(t, s);
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_EuTransfer_Exchange(AH_JOB *j, AB_JOB *bj,
			       AH_JOB_EXCHANGE_MODE m,
			       AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_EUTRANSFER *aj;
  AB_BANKING *ab;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_EUTRANSFER, j);
  assert(aj);

  ab=AB_Account_GetBanking(AB_Job_GetAccount(bj));
  assert(ab);

  if (AB_Job_GetType(bj)!=AB_Job_TypeEuTransfer) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Not a %s job job",
              "EuTransfer");
    return GWEN_ERROR_INVALID;
  }

  switch(m) {
  case AH_Job_ExchangeModeParams: {
    GWEN_DB_NODE *dbParams;
    GWEN_DB_NODE *dbT;
    int i;
    const char *s;

    dbParams=AH_Job_GetParams(j);
    /*DBG_NOTICE(AQHBCI_LOGDOMAIN, "Have this parameters to exchange:");
    GWEN_DB_Dump(dbParams, stderr, 2);*/
    s=GWEN_DB_GetCharValue(dbParams, "IbanAllowed", 0, "N");
    AB_JobEuTransfer_SetIbanAllowed(bj, strcasecmp(s, "J")==0);

    dbT=GWEN_DB_FindFirstGroup(dbParams, "country");
    if (dbT) {
      AB_EUTRANSFER_INFO_LIST *eil;

      eil=AB_EuTransferInfo_List_new();
      while(dbT) {
        i=GWEN_DB_GetIntValue(dbT, "code", 0, 0);
        if (i) {
          const AB_COUNTRY *cnt;
          AB_BANKING *ab;

          ab=AB_Account_GetBanking(AB_Job_GetAccount(bj));
          assert(ab);
          cnt=AB_Banking_FindCountryByNumeric(ab, i);
          if (cnt) {
            AB_EUTRANSFER_INFO *ei;
            const char *v, *cur;
            AB_TRANSACTION_LIMITS *lim;

            ei=AB_EuTransferInfo_new();
            lim=AB_TransactionLimits_new();

            AB_TransactionLimits_SetNeedDate(lim, -1);

            AB_EuTransferInfo_SetCountryCode(ei, AB_Country_GetCode(cnt));

            i=GWEN_DB_GetIntValue(dbT, "MaxLenOurName", 0, -1);
            AB_TransactionLimits_SetMaxLenLocalName(lim, i);

            i=GWEN_DB_GetIntValue(dbT, "MaxLenOtherName", 0, -1);
            AB_TransactionLimits_SetMaxLenRemoteName(lim, i);
            AB_TransactionLimits_SetMaxLinesRemoteName(lim, 1);

            i=GWEN_DB_GetIntValue(dbT, "MaxLenPurpose", 0, -1);
            AB_TransactionLimits_SetMaxLenPurpose(lim, i);
            AB_TransactionLimits_SetMaxLinesPurpose(lim, 1);

            v=GWEN_DB_GetCharValue(dbT, "LimitLocalValue", 0, 0);
            cur=GWEN_DB_GetCharValue(dbT, "LimitLocalCurrency", 0, 0);
            if (v) {
              double dv;

              if (GWEN_Text_StringToDouble(v, &dv)) {
                DBG_WARN(AQHBCI_LOGDOMAIN, "Bad value");
              }
              else {
                AB_VALUE *val;

		val=AB_Value_fromDouble(dv);
                assert(val);
		AB_Value_SetCurrency(val, cur);
                AB_EuTransferInfo_SetLimitLocalValue(ei, val);
                AB_Value_free(val);
              }
            }

            v=GWEN_DB_GetCharValue(dbT, "LimitForeignValue", 0, 0);
            cur=GWEN_DB_GetCharValue(dbT, "LimitforeignCurrency", 0, 0);
            if (v) {
              double dv;

              if (GWEN_Text_StringToDouble(v, &dv)) {
                DBG_WARN(AQHBCI_LOGDOMAIN, "Bad value");
              }
              else {
                AB_VALUE *val;

                val=AB_Value_fromDouble(dv);
                assert(val);
		AB_Value_SetCurrency(val, cur);
                AB_EuTransferInfo_SetLimitForeignValue(ei, val);
                AB_Value_free(val);
              }
            }

            /* set limits */
            AB_EuTransferInfo_SetFieldLimits(ei, lim);

            /* add country info */
            AB_EuTransferInfo_List_Add(ei, eil);
          } /* if cnt */
        } /* if numeric country code given */
        dbT=GWEN_DB_FindNextGroup(dbT, "country");
      } /* while dbT */
      AB_JobEuTransfer_SetCountryInfoList(bj, eil);
    } /* if countryInfo in params */

    return 0;
  }

  case AH_Job_ExchangeModeArgs: {
    GWEN_DB_NODE *dbArgs;
    const AB_TRANSACTION *ot;
    const AB_VALUE *v;

    dbArgs=AH_Job_GetArguments(j);
    assert(dbArgs);
    ot=AB_JobEuTransfer_GetTransaction(bj);
    if (ot) {
      GWEN_DB_NODE *dbT;
      const char *p;
      const GWEN_STRINGLIST *sl;
      AB_TRANSACTION *t;
      const AB_COUNTRY *cy=0;
      int i;

      t=AB_Transaction_dup(ot);
      assert(t);
      if (AH_Job_EuTransfer__ValidateTransfer(bj, j, t)) {
	DBG_ERROR(AQHBCI_LOGDOMAIN,
		  "Invalid transaction");
	AB_Job_SetStatus(bj, AB_Job_StatusError);
        return GWEN_ERROR_INVALID;
      }

      /* get remote country information */
      p=AB_Transaction_GetRemoteIban(t);
      if (p) {
        char cnt[3];
    
        memmove(cnt, p, 2);
        cnt[2]=0;
        cy=AB_Banking_FindCountryByCode(ab, cnt);
      }
      else {
        p=AB_Transaction_GetRemoteCountry(t);
        if (!p) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "remote country code not set");
          return GWEN_ERROR_INVALID;
        }
        cy=AB_Banking_FindCountryByCode(ab, p);
      }
      if (!cy) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No country info available");
        return GWEN_ERROR_INVALID;
      }

      /* store the validated transaction back into application job,
       * to allow the application to recognize answers to this job later */
      AB_JobEuTransfer_SetTransaction(bj, t);

      dbT=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                           "transaction");
      assert(dbT);

      /* store local account */
      GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "ourAccount/country", 280);
      GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			   "ourAccount/bankCode",
			   AB_Transaction_GetLocalBankCode(t));
      GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "ourAccount/accountId",
                           AB_Transaction_GetLocalAccountNumber(t));

      p=AB_Transaction_GetLocalSuffix(t);
      if (p)
	GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			     "ourAccount/accountsubid", p);
      GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			   "ourName",
			   AB_Transaction_GetLocalName(t));

      p=AB_Transaction_GetRemoteIban(t);
      if (p) {
	/* store IBAN */
	GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			     "iban",
			     p);
      }
      else {
	const char *s;

	/* store remote account */
	if (cy) {
	  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			      "otherAccount/country",
			      AB_Country_GetNumericCode(cy));
	}

	s=AB_Transaction_GetRemoteBankCode(t);
        if (s)
	  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			       "otherAccount/bankCode",
			       s);
	s=AB_Transaction_GetRemoteAccountNumber(t);
        if (s)
	  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			       "otherAccount/accountId",
			       s);
	else {
	  DBG_ERROR(AQHBCI_LOGDOMAIN, "Remote account id not set");
	  return GWEN_ERROR_INVALID;
	}
      }

      p=AB_Transaction_GetRemoteBankName(t);
      if (p)
        GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "otherBankName",
                             p);


      /* store remote name */
      sl=AB_Transaction_GetRemoteName(t);
      if (sl) {
	GWEN_STRINGLISTENTRY *se;

	se=GWEN_StringList_FirstEntry(sl);
	GWEN_DB_DeleteVar(dbT, "otherName");
        while(se) {
          p=GWEN_StringListEntry_Data(se);
          if (p)
            GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_DEFAULT,
                                 "otherName", p);
          se=GWEN_StringListEntry_Next(se);
        } /* while */
      }

      /* store value */
      v=AB_Transaction_GetValue(t);
      if (v) {
	GWEN_DB_NODE *dbV;
        GWEN_BUFFER *nbuf;
        char *p;
        const char *s;
        int l;

	dbV=GWEN_DB_GetGroup(dbT, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "value");
        assert(dbV);

	nbuf=GWEN_Buffer_new(0, 32, 0, 1);
	if (GWEN_Text_DoubleToBuffer(AB_Value_GetValueAsDouble(v),
				     nbuf)) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN, "Buffer overflow");
          GWEN_Buffer_free(nbuf);
	  abort();
	}

	l=GWEN_Buffer_GetUsedBytes(nbuf);
	if (!l) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN, "Error in conversion");
	  GWEN_Buffer_free(nbuf);
	  abort();
        }

        /* replace "C" comma with "DE" comma, remove thousand's comma */
        p=GWEN_Buffer_GetStart(nbuf);
        s=p;
        while(*s) {
          if (*s=='.') {
            *p=',';
            p++;
          }
          else if (*s!=',') {
            *p=*s;
            p++;
          }
          s++;
        } /* while */
        *p=0;

	if (strchr(GWEN_Buffer_GetStart(nbuf), ',')) {
	  /* kill all trailing '0' behind the comma */
	  p=GWEN_Buffer_GetStart(nbuf)+l;
	  while(l--) {
	    --p;
            if (*p=='0')
              *p=0;
            else
              break;
          }
	}
	else
	  GWEN_Buffer_AppendString(nbuf, ",");

	/* store value */
	GWEN_DB_SetCharValue(dbV, GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "value",
			     GWEN_Buffer_GetStart(nbuf));
	GWEN_Buffer_free(nbuf);

	s=AB_Value_GetCurrency(v);
        if (!s)
          s="EUR";
        GWEN_DB_SetCharValue(dbV, GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "currency", s);
      } /* if value */

      /* store purpose */
      sl=AB_Transaction_GetPurpose(t);
      if (sl) {
	GWEN_STRINGLISTENTRY *se;

	se=GWEN_StringList_FirstEntry(sl);
	GWEN_DB_DeleteVar(dbT, "purpose");
	while(se) {
	  p=GWEN_StringListEntry_Data(se);
	  if (p)
	    GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_DEFAULT,
				 "purpose", p);
	  se=GWEN_StringListEntry_Next(se);
	} /* while */
      }

      /* store "chargeWhom" */
      switch (AB_JobEuTransfer_GetChargeWhom(bj)) {
      case AB_JobEuTransfer_ChargeWhom_Local:  i=1; break;
      case AB_JobEuTransfer_ChargeWhom_Remote: i=3; break;
      case AB_JobEuTransfer_ChargeWhom_Share:  i=2; break;
      default:                                 i=1; break;
      } /* switch */
      GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "chargeWhom", i);
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction");
      AB_Job_SetStatus(bj, AB_Job_StatusError);
      return GWEN_ERROR_NO_DATA;
    }


    return 0;
  }

  case AH_Job_ExchangeModeResults: {
    AH_RESULT_LIST *rl;
    AH_RESULT *r;
    int has10;
    int has20;

    rl=AH_Job_GetSegResults(j);
    assert(rl);

    r=AH_Result_List_First(rl);
    if (!r) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No segment results");
      AB_Job_SetStatus(bj, AB_Job_StatusError);
      return GWEN_ERROR_NO_DATA;
    }
    has10=0;
    has20=0;
    while(r) {
      int rcode;

      rcode=AH_Result_GetCode(r);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Found job result: %d", rcode);
      if (rcode<=19)
	has10=1;
      else if (rcode>=20 && rcode <=29)
	has20=1;
      r=AH_Result_List_Next(r);
    }

    if (has20) {
      AB_Job_SetStatus(bj, AB_Job_StatusFinished);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job finished");
    }
    else if (has10) {
      AB_Job_SetStatus(bj, AB_Job_StatusPending);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job pending");
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Can't determine the status (neither 0010 nor 0020)");
      AB_Job_SetStatus(bj, AB_Job_StatusError);
      return GWEN_ERROR_NO_DATA;
    }
    return 0;
  }

  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode");
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */
}










