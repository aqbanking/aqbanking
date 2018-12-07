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

#include "qif_p.h"
#include "i18n_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <aqbanking/transaction.h>
#include <ctype.h>


GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_QIF);


AB_IMEXPORTER *qif_factory(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_QIF *ieh;

  ie=AB_ImExporter_new(ab, "qif");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_QIF, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_QIF, ie, ieh,
                       AH_ImExporterQIF_FreeData);
  ieh->dbData=db;

  AB_ImExporter_SetImportFn(ie, AH_ImExporterQIF_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterQIF_Export);
  /* AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterQIF_CheckFile); -- not yet implemented?! */
  return ie;
}



void GWENHYWFAR_CB AH_ImExporterQIF_FreeData(void *bp, void *p){
}




int AH_ImExporterQIF__GetDate(AB_IMEXPORTER *ie,
                              GWEN_DB_NODE *params,
                              const char *paramName,
                              const char *paramDescr,
                              const char *paramContent,
                              GWEN_TIME **pti) {
  const char *dateFormat;
  char dfbuf[32];
  int rv;
  GWEN_TIME *ti=0;
  AH_IMEXPORTER_QIF *ieqif;
  int first=1;

  assert(ie);
  ieqif=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_QIF, ie);
  assert(ieqif);

  dateFormat=GWEN_DB_GetCharValue(params, paramName, 0, 0);
  if (!dateFormat)
    dateFormat=GWEN_DB_GetCharValue(ieqif->dbData, paramName, 0, 0);
  if (!dateFormat)
    dateFormat=GWEN_DB_GetCharValue(params, "dateFormat", 0, 0);

  while(!ti) {
    if (!dateFormat) {
      GWEN_BUFFER *tbuf;
      const char *t1a=I18N_NOOP("Please enter the date format for the "
                               "following item:\n");
      const char *t1h=I18N_NOOP("<html>"
                                "Please enter the date format for the "
                                "following item:<br>");
      const char *t2a=I18N_NOOP
        ("The following characters can be used:\n"
         "- \'Y\': digit of the year\n"
         "- \'M\': digit of the month\n"
         "- \'D\': digit of the day\n"
         "\n"
         "Examples:\n"
         " \"YYYY/MM/DD\" (-> 2005/02/25)\n"
         " \"DD.MM.YYYY\" (-> 25.02.2005)\n"
         " \"MM/DD/YY\"   (-> 02/25/05)\n");
      const char *t2h=I18N_NOOP
        ("The following characters can be used:"
         "<table>"
         " <tr><td><i>Y</i></td><td>digit of the year</td></tr>\n"
         " <tr><td><i>M</i></td><td>digit of the month</td></tr>\n"
         " <tr><td><i>D</i></td><td>digit of the day</td></tr>\n"
         "</table>\n"
         "<br>"
         "Examples:"
         "<table>"
         " <tr><td><i>YYYY/MM/DD</i></td><td>(-> 2005/02/25)</td></tr>\n"
         " <tr><td><i>DD.MM.YYYY</i></td><td>(-> 25.02.2005)</td></tr>\n"
         " <tr><td><i>MM/DD/YY</i></td><td>(-> 02/25/05)</td></tr>\n"
         "</html>");

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      /* ASCII version */
      GWEN_Buffer_AppendString(tbuf, I18N(t1a));
      GWEN_Buffer_AppendString(tbuf, paramDescr);
      GWEN_Buffer_AppendString(tbuf, " (");
      GWEN_Buffer_AppendString(tbuf, paramContent);
      GWEN_Buffer_AppendString(tbuf, " )\n");
      GWEN_Buffer_AppendString(tbuf, I18N(t2a));
      /* HTML version */
      GWEN_Buffer_AppendString(tbuf, I18N(t1h));
      GWEN_Buffer_AppendString(tbuf, paramDescr);
      GWEN_Buffer_AppendString(tbuf, " (");
      GWEN_Buffer_AppendString(tbuf, paramContent);
      GWEN_Buffer_AppendString(tbuf, " )\n");
      GWEN_Buffer_AppendString(tbuf, I18N(t2h));

      rv=GWEN_Gui_InputBox(0,
			   first?I18N("Enter Date Format"):
			   I18N("Enter Correct Date Format"),
			   GWEN_Buffer_GetStart(tbuf),
			   dfbuf, 4,  sizeof(dfbuf)-1,
			   0);
      GWEN_Buffer_free(tbuf);
      if (rv) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "here");
        return rv;
      }
      dateFormat=dfbuf;

      ti=GWEN_Time_fromString(paramContent, dateFormat);
      if (ti) {
        /* store particular date format */
        GWEN_DB_SetCharValue(ieqif->dbData,
                             GWEN_DB_FLAGS_OVERWRITE_VARS,
                             paramName, dfbuf);
        break;
      }
      dateFormat=0;
    }
    else
      break;
  } /* for */

  *pti=ti;
  return 0;
}



int AH_ImExporterQIF__GetValue(AB_IMEXPORTER *ie,
                               GWEN_DB_NODE *params,
                               const char *paramName,
                               const char *paramDescr,
                               const char *paramContent,
			       AB_VALUE **pv) {
  const char *s;
  char komma = 0;
  char fixpoint = 0;
  AH_IMEXPORTER_QIF *ieqif;
  char numbuf[64];
  int i;
  double dval;
  AB_VALUE *v;

  assert(ie);
  ieqif=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_QIF, ie);
  assert(ieqif);

  /* get komma character */
  s=GWEN_DB_GetCharValue(params, "value/komma", 0, 0);
  if (!s)
    s=GWEN_DB_GetCharValue(ieqif->dbData, "value/komma", 0, 0);
  if (s)
    komma=*s;
  /* get fixpoint character */
  s=GWEN_DB_GetCharValue(params, "value/fixpoint", 0, 0);
  if (!s)
    s=GWEN_DB_GetCharValue(ieqif->dbData, "value/fixpoint", 0, 0);
  if (s)
    fixpoint=*s;

  if (!fixpoint) {
    const char *lastKommaPos = NULL;
    char lastKommaChar=0;
    int komma1Count=0;
    int komma2Count=0;
    int kommaTypeCount=0;

    fixpoint=0;
    komma=0;

    /* nothing known about fixpoint, elaborate */
    s=paramContent;
    while(*s) {
      if (*s=='.' || *s==',') {
	if (*s==',')
	  komma1Count++;
	else
	  komma2Count++;
	lastKommaChar=*s;
	lastKommaChar=*s;
	kommaTypeCount++;
	lastKommaPos=s;
      }
      s++;
    } /* while */
    if ( ( (komma1Count+komma2Count)==1 ) && lastKommaPos) {
      int i=0;

      /* only one komma, check for digits behind it */
      s=lastKommaPos;
      s++;
      while(*s && isdigit(*s)) {
	s++;
	i++;
      }
      if (i<3) {
	/* most likely got the fixpoint */
	fixpoint=lastKommaChar;
      }
    }
    else if ((komma1Count==1 && komma2Count>0) ||
	     (komma2Count==1 && komma1Count>0)) {
      if (komma1Count==1) {
	fixpoint=',';
        komma='.';
      }
      else {
	fixpoint='.';
        komma=',';
      }
    }
    else {
      GWEN_BUFFER *tbuf;
      int rv;
      const char *t1a=
	I18N_NOOP("The following value could not be parsed: \n");
      const char *t2a=
	I18N_NOOP("There are now two possibilities of what character\n"
		  "represents the decimal fixpoint:\n"
		  " 1) \'.\' (as in \"123.45\")\n"
		  " 2) \',\' (as in \"123,45\")\n"
		  "What is the fixpoint in the value above?");
      const char *t1h=
	I18N_NOOP("<htlm>The following value could not be parsed: <br>");
      const char *t2h=
	I18N_NOOP("<br>"
		  "There are now two possibilities of what character "
		  "represents the decimal fixpoint: "
                  "<ol>"
		  " <li>\'.\' (as in \"123.45\")</li>\n"
		  " <li>\',\' (as in \"123,45\")</li>\n"
		  "</ol>"
		  "What is the fixpoint in the value above?"
		  "</html>");

      /* this is weird, ask the user */
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Buffer_AppendString(tbuf, t1a);
      GWEN_Buffer_AppendString(tbuf, paramContent);
      GWEN_Buffer_AppendString(tbuf, t2a);
      GWEN_Buffer_AppendString(tbuf, t1h);
      GWEN_Buffer_AppendString(tbuf, paramContent);
      GWEN_Buffer_AppendString(tbuf, t2h);
      rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN |
			     GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL |
			     GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
			     I18N("Value Parsing"),
			     I18N(GWEN_Buffer_GetStart(tbuf)),
			     I18N("Possibility 1"),
			     I18N("Possibility 2"),
			     0,
			     0);
      GWEN_Buffer_free(tbuf);
      if (rv==1) {
	fixpoint='.';
	komma=',';
      }
      else if (rv==2) {
	fixpoint=',';
	komma='.';
      }
      else {
	DBG_INFO(AQBANKING_LOGDOMAIN, "here");
        return rv;
      }
    }
  } /* if !fixpoint */

  /* now we know what the fixpoint is, store it */
  numbuf[0]=komma;
  numbuf[1]=0;
  GWEN_DB_SetCharValue(ieqif->dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "value/komma", numbuf);
  numbuf[0]=fixpoint;
  GWEN_DB_SetCharValue(ieqif->dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "value/fixpoint", numbuf);

  i=0;
  s=paramContent;
  while(*s && i<(int)sizeof(numbuf)) {
    if (*s==fixpoint)
      numbuf[i++]=',';
    else if (*s=='-' || *s=='+' || *s==isdigit(*s))
      numbuf[i++]=*s;
    else if (*s!=komma) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Bad character in value string");
      return GWEN_ERROR_BAD_DATA;
    }
  }
  if (i>=(int)sizeof(numbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Value string too long");
    return GWEN_ERROR_BAD_DATA;
  }
  numbuf[i]=0;

  if (GWEN_Text_StringToDouble(numbuf, &dval)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Value string does not contain a floating point value.");
    return GWEN_ERROR_BAD_DATA;
  }

  v=AB_Value_fromDouble(dval);
  *pv=v;
  return 0;
}



int AH_ImExporterQIF__ImportAccount(AB_IMEXPORTER *ie,
				    AB_IMEXPORTER_CONTEXT *iec,
                                    GWEN_BUFFEREDIO *bio,
                                    GWEN_BUFFER *buf,
                                    GWEN_DB_NODE *params){
  AH_IMEXPORTER_QIF *ieqif;
  GWEN_DB_NODE *dbData;
  AB_IMEXPORTER_ACCOUNTINFO *iea = 0;
  int done=0;
  const char *s;
  GWEN_TIME *ti=0;
  AB_VALUE *vCreditLine=0;
  AB_VALUE *vBalance=0;

  assert(ie);
  ieqif=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_QIF, ie);
  assert(ieqif);

  dbData=GWEN_DB_Group_new("data");
  while (!done) {
    const char *p;

    if (!GWEN_Buffer_GetUsedBytes(buf)) {
      int err;

      if (GWEN_BufferedIO_CheckEOF(bio)) {
        done=1;
        continue;
      }

      err=GWEN_BufferedIO_ReadLine2Buffer(bio, buf);
      if (err) {
	DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
	GWEN_DB_Group_free(dbData);
	return err;
      }
    }

    p=GWEN_Buffer_GetStart(buf);
    while(isspace(*p))
      p++;

    switch(toupper(*p)) {
    case 'N': /* account name */
      GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "name", p+1);
      break;
    case 'T': /* account type */
      GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "type", p+1);
      break;
    case 'D': /* description */
      GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "descr", p+1);
      break;
    case 'L': /* credit line (credit card accounts only */
      GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "creditLine", p+1);
      break;
    case '/': /* date of statement balance */
      GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "date", p+1);
      break;
    case '$': /* statement balance */
      GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "balance", p+1);
      break;
    case '^': /* end of record */
      done=1;
      break;
    default: /* unknown line, ignore */
      DBG_WARN(AQBANKING_LOGDOMAIN,
               "Unknown item \"%s\", ignoring",
               GWEN_Buffer_GetStart(buf));
    } /* switch */

    GWEN_Buffer_Reset(buf);
  } /* while not end of block reached */

  /* find account info by account name */
  s=GWEN_DB_GetCharValue(dbData, "name", 0, 0);
  if (s) {
    iea=AB_ImExporterContext_GetFirstAccountInfo(iec);
    while(iea) {
      if (strcasecmp(AB_ImExporterAccountInfo_GetAccountName(iea), s)==0)
        break;
      iea=AB_ImExporterContext_GetNextAccountInfo(iec);
    } /* while */
  }

  if (!iea) {
    /* not found, add it */
    iea=AB_ImExporterAccountInfo_new();
    AB_ImExporterContext_AddAccountInfo(iec, iea);
    /* set account info */
    if (s)
      AB_ImExporterAccountInfo_SetAccountName(iea, s);
    s=GWEN_DB_GetCharValue(dbData, "descr", 0, 0);
    if (s)
      AB_ImExporterAccountInfo_SetDescription(iea, s);
    s=GWEN_DB_GetCharValue(dbData, "type", 0, 0);
    if (s) {
      if (strcasecmp(s, "bank")==0)
        AB_ImExporterAccountInfo_SetType(iea, AB_AccountType_Bank);
      else if (strcasecmp(s, "Invst")==0)
        AB_ImExporterAccountInfo_SetType(iea, AB_AccountType_Investment);
      else if (strcasecmp(s, "CCard")==0)
        AB_ImExporterAccountInfo_SetType(iea, AB_AccountType_CreditCard);
      else if (strcasecmp(s, "Cash")==0)
        AB_ImExporterAccountInfo_SetType(iea, AB_AccountType_Cash);
      else
        AB_ImExporterAccountInfo_SetType(iea, AB_AccountType_Unknown);
    }
  }
  assert(iea);
  ieqif->currentAccount=iea;

  s=GWEN_DB_GetCharValue(dbData, "date", 0, 0);
  if (s) {
    int rv;

    rv=AH_ImExporterQIF__GetDate(ie, params,
				 "account/statement/dateFormat",
				 I18N("Account statement date"),
				 s, &ti);
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      GWEN_DB_Group_free(dbData);
      return rv;
    }
  }/* if date */

  s=GWEN_DB_GetCharValue(dbData, "creditLine", 0, 0);
  if (s) {
    int rv;

    rv=AH_ImExporterQIF__GetValue(ie, params,
                                  "account/statement/creditLineFormat",
                                  I18N("Account statement credit line value"),
                                  s, &vCreditLine);
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      GWEN_Time_free(ti);
      GWEN_DB_Group_free(dbData);
      return rv;
    }
  }/* if date */

  s=GWEN_DB_GetCharValue(dbData, "balance", 0, 0);
  if (s) {
    int rv;

    rv=AH_ImExporterQIF__GetValue(ie, params,
                                  "account/statement/balanceFormat",
                                  I18N("Account statement balance value"),
                                  s, &vBalance);
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      AB_Value_free(vCreditLine);
      GWEN_Time_free(ti);
      GWEN_DB_Group_free(dbData);
      return rv;
    }
  }/* if date */

  if (ti && (vBalance || vCreditLine)) {
    AB_BALANCE *balance=0;
    AB_ACCOUNT_STATUS *ast=0;

    if (vBalance && ti)
      balance=AB_Balance_new(vBalance, ti);

    ast=AB_AccountStatus_new();
    if (ti)
      AB_AccountStatus_SetTime(ast, ti);
    if (vCreditLine)
      AB_AccountStatus_SetBankLine(ast, vCreditLine);
    if (balance)
      AB_AccountStatus_SetBookedBalance(ast, balance);
    /* add account status */
    AB_ImExporterAccountInfo_AddAccountStatus(iea, ast);

    AB_AccountStatus_free(ast);
    AB_Balance_free(balance);
  }
  AB_Value_free(vBalance);
  AB_Value_free(vCreditLine);
  GWEN_Time_free(ti);
  GWEN_DB_Group_free(dbData);

  return 0;
}



int AH_ImExporterQIF__ImportBank(AB_IMEXPORTER *ie,
                                 AB_IMEXPORTER_CONTEXT *iec,
                                 GWEN_BUFFEREDIO *bio,
                                 GWEN_BUFFER *buf,
                                 GWEN_DB_NODE *params){
  AH_IMEXPORTER_QIF *ieqif;
  GWEN_DB_NODE *dbData;
  AB_IMEXPORTER_ACCOUNTINFO *iea;
  int done=0;
  const char *s;
  GWEN_TIME *ti=0;
  AB_VALUE *vAmount=0;
  GWEN_DB_NODE *dbCurrentSplit=0;
  AB_TRANSACTION *t=0;

  assert(ie);
  ieqif=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_QIF, ie);
  assert(ieqif);

  dbData=GWEN_DB_Group_new("data");
  while (!done) {
    const char *p;

    if (!GWEN_Buffer_GetUsedBytes(buf)) {
      int err;

      if (GWEN_BufferedIO_CheckEOF(bio)) {
        done=1;
        continue;
      }

      err=GWEN_BufferedIO_ReadLine2Buffer(bio, buf);
      if (err) {
        DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
        GWEN_DB_Group_free(dbData);
	return err;
      }
    }

    p=GWEN_Buffer_GetStart(buf);
    while(isspace(*p))
      p++;

    switch(toupper(*p)) {
    case 'S':
      dbCurrentSplit=GWEN_DB_GetGroup(dbData,
                                      GWEN_PATH_FLAGS_CREATE_GROUP,
                                      "split");
      assert(dbCurrentSplit);
      GWEN_DB_SetCharValue(dbCurrentSplit,
                           GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "category", p+1);
      break;
    case '$': /* split amount */
      assert(dbCurrentSplit);
      GWEN_DB_SetCharValue(dbCurrentSplit, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "amount", p+1);
      break;
    case 'E': /* split memo */
      assert(dbCurrentSplit);
      GWEN_DB_SetCharValue(dbCurrentSplit, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "memo", p+1);
      break;

    case 'D': /* date */
      GWEN_DB_SetCharValue(dbData,
                           GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "date", p+1);
      break;
    case 'N': /* reference */
      GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "reference", p+1);
      break;
    case 'T': /* amount */
      GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "amount", p+1);
      break;
    case 'P': /* payee */
      GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_DEFAULT,
                           "payee", p+1);
      break;
    case 'M': /* memo */
      GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "memo", p+1);
      break;
    case 'A': /* address */
      GWEN_DB_SetCharValue(dbData,
                           GWEN_DB_FLAGS_DEFAULT,
                           "address", p+1);
      break;
    case 'L': /* category */
      GWEN_DB_SetCharValue(dbData,
                           GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "category", p+1);
      break;
    case 'C': /* cleared status */
      GWEN_DB_SetCharValue(dbData,
                           GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "cleared", p+1);
      break;
    case '^': /* end of record */
      done=1;
      break;
    default: /* unknown line, ignore */
      DBG_WARN(AQBANKING_LOGDOMAIN,
               "Unknown item \"%s\", ignoring",
               GWEN_Buffer_GetStart(buf));
    } /* switch */

    GWEN_Buffer_Reset(buf);
  } /* while not end of block reached */

  iea=ieqif->currentAccount;
  assert(iea);

  s=GWEN_DB_GetCharValue(dbData, "date", 0, 0);
  if (s) {
    int rv;

    rv=AH_ImExporterQIF__GetDate(ie, params,
				 "account/statement/dateFormat",
                                 I18N("Account statement date"),
                                 s, &ti);
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      GWEN_DB_Group_free(dbData);
      return rv;
    }
  }/* if date */

  s=GWEN_DB_GetCharValue(dbData, "amount", 0, 0);
  if (s) {
    int rv;

    rv=AH_ImExporterQIF__GetValue(ie, params,
                                  "bank/statement/amountFormat",
                                  I18N("Transaction statement amount value"),
                                  s, &vAmount);
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      GWEN_Time_free(ti);
      GWEN_DB_Group_free(dbData);
      return rv;
    }
  }/* if date */

  t=AB_Transaction_new();
  if (ti) {
    AB_Transaction_SetValutaDate(t, ti);
    AB_Transaction_SetDate(t, ti);
  }
  if (vAmount)
    AB_Transaction_SetValue(t, vAmount);

  s=GWEN_DB_GetCharValue(dbData, "payee", 0, 0);
  if (s)
    AB_Transaction_AddRemoteName(t, s, 0);
  s=GWEN_DB_GetCharValue(dbData, "memo", 0, 0);
  if (s)
    AB_Transaction_AddPurpose(t, s, 0);

  DBG_INFO(AQBANKING_LOGDOMAIN, "Adding transaction");
  AB_ImExporterAccountInfo_AddTransaction(iea, t);

  AB_Value_free(vAmount);
  GWEN_Time_free(ti);
  GWEN_DB_Group_free(dbData);

  return 0;
}










int AH_ImExporterQIF_Import(AB_IMEXPORTER *ie,
			    AB_IMEXPORTER_CONTEXT *ctx,
			    GWEN_BUFFEREDIO *bio,
			    GWEN_DB_NODE *params){
  GWEN_BUFFER *buf;
  char lastSectionName[256];
  AH_IMEXPORTER_QIF *ieqif;

  assert(ie);
  ieqif=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_QIF, ie);
  assert(ieqif);

  buf=GWEN_Buffer_new(0, 256, 0, 1);

  while(!GWEN_BufferedIO_CheckEOF(bio)) {
    int err;
    const char *p;
    int rv;

    err=GWEN_BufferedIO_ReadLine2Buffer(bio, buf);
    if (err) {
      DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
      GWEN_Buffer_free(buf);
      return err;
    }
    p=GWEN_Buffer_GetStart(buf);
    while(isspace(*p))
      p++;

    if (*p=='!') {
      p++;
      if (strlen(p)>=(int)sizeof(lastSectionName)) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Buffer too small. Internal error, should not occurr.");
	abort();
      }
      strcpy(lastSectionName, p);
      GWEN_Buffer_Reset(buf);
    }

    if (lastSectionName[0]) {
      if (strcasecmp(lastSectionName, "Account")==0)
	rv=AH_ImExporterQIF__ImportAccount(ie, ctx, bio, buf, params);
      else {
	DBG_WARN(AQBANKING_LOGDOMAIN,
		 "Unknown section \"%s\", ignoring",
		 GWEN_Buffer_GetStart(buf));
	lastSectionName[0]=0;
	rv=0; /* ignore this section */
      }
    }

    GWEN_Buffer_Reset(buf);
  } /* while */

  return 0;
}



int AH_ImExporterQIF_Export(AB_IMEXPORTER *ie,
			    AB_IMEXPORTER_CONTEXT *ctx,
			    GWEN_BUFFEREDIO *bio,
                            GWEN_DB_NODE *params){
  return GWEN_ERROR_NOT_SUPPORTED;
}



