/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "csv_p.h"
#include "csv_editprofile_l.h"
#include "aqbanking/i18n_l.h"

#include "aqbanking/backendsupport/imexporter_be.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/gui.h>


GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_CSV);



static AB_VALUE *_valueFromDb(GWEN_DB_NODE *dbV, int commaThousands, int commaDecimal);
static void _unsplitInOutValue(GWEN_DB_NODE *dbT, int commaThousands, int commaDecimal);
static void _collectPurposeStrings(AB_TRANSACTION *t, GWEN_DB_NODE *dbT);
static void _readValues(AB_TRANSACTION *t, GWEN_DB_NODE *dbT, int commaThousands, int commaDecimal);
static void _readDates(AB_TRANSACTION *t, GWEN_DB_NODE *dbT, const char *dateFormat);
static void _translateValuesSign(AB_TRANSACTION *t, GWEN_DB_NODE *dbT, GWEN_DB_NODE *dbParams);
static int _mustNegate(GWEN_DB_NODE *dbT, GWEN_DB_NODE *dbParams);
static void _switchLocalRemoteAccordingToSign(AB_TRANSACTION *t, int switchOnNegative);
static int _groupNameMatches(const char *groupName, GWEN_DB_NODE *dbParams);




AB_IMEXPORTER *AB_ImExporterCSV_new(AB_BANKING *ab)
{
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_CSV *ieh;

  ie=AB_ImExporter_new(ab, "csv");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_CSV, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CSV, ie, ieh, AH_ImExporterCSV_FreeData);
  ieh->dbio=GWEN_DBIO_GetPlugin("csv");
  if (!ieh->dbio) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "GWEN DBIO plugin \"CSV\" not available");
    AB_ImExporter_free(ie);
    return NULL;
  }

  AB_ImExporter_SetImportFn(ie, AH_ImExporterCSV_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterCSV_Export);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterCSV_CheckFile);
  AB_ImExporter_SetGetEditProfileDialogFn(ie, AH_ImExporterCSV_GetEditProfileDialog);

  /* announce special features */
  AB_ImExporter_AddFlags(ie, AB_IMEXPORTER_FLAGS_GETPROFILEEDITOR_SUPPORTED);

  return ie;
}



void GWENHYWFAR_CB AH_ImExporterCSV_FreeData(void *bp, void *p)
{
  AH_IMEXPORTER_CSV *ieh;

  ieh=(AH_IMEXPORTER_CSV *)p;
  GWEN_DBIO_free(ieh->dbio);
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterCSV_Import(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_SYNCIO *sio,
                            GWEN_DB_NODE *params)
{
  AH_IMEXPORTER_CSV *ieh;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbSubParams;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CSV, ie);
  assert(ieh);
  assert(ieh->dbio);

  dbSubParams=GWEN_DB_GetGroup(params, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                               "params");
  dbData=GWEN_DB_Group_new("transactions");
  rv=GWEN_DBIO_Import(ieh->dbio,
                      sio,
                      dbData,
                      dbSubParams,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error importing data (%d)", rv);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         "Error importing data");
    GWEN_DB_Group_free(dbData);
    return GWEN_ERROR_GENERIC;
  }

  /* transform DB to transactions */
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
                       I18N("Data imported, transforming to UTF-8"));
  rv=AB_ImExporter_DbFromIso8859_1ToUtf8(dbData);
  if (rv) {
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         "Error converting data");
    GWEN_DB_Group_free(dbData);
    return rv;
  }
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
                       "Transforming data to transactions");
  rv=AH_ImExporterCSV__ImportFromGroup(ctx, dbData, params);
  if (rv) {
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         "Error importing data");
    GWEN_DB_Group_free(dbData);
    return rv;
  }

  GWEN_DB_Group_free(dbData);
  return 0;
}



int AH_ImExporterCSV__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                      GWEN_DB_NODE *db,
                                      GWEN_DB_NODE *dbParams)
{
  GWEN_DB_NODE *dbT;
  AB_TRANSACTION_TYPE defaultType=AB_Transaction_TypeStatement;
  const char *dateFormat;
  int usePosNegField;
  int splitValueInOut;
  int switchLocalRemote;
  int switchOnNegative;
  int commaThousands=0;
  int commaDecimal=0;
  const char *s;

  dateFormat=GWEN_DB_GetCharValue(dbParams, "dateFormat", 0, "YYYY/MM/DD");
  usePosNegField=GWEN_DB_GetIntValue(dbParams, "usePosNegField", 0, 0);
  splitValueInOut=GWEN_DB_GetIntValue(dbParams, "splitValueInOut", 0, 0);
  switchLocalRemote=GWEN_DB_GetIntValue(dbParams, "switchLocalRemote", 0, 0);
  switchOnNegative=GWEN_DB_GetIntValue(dbParams, "switchOnNegative", 0, 1);

  s=GWEN_DB_GetCharValue(dbParams, "commaThousands", 0, 0);
  if (s)
    commaThousands=*s;
  s=GWEN_DB_GetCharValue(dbParams, "commaDecimal", 0, 0);
  if (s)
    commaDecimal=*s;

  s=GWEN_DB_GetCharValue(dbParams, "transactionType", 0, "statement");
  if (s && *s) {
    defaultType=AB_Transaction_Type_fromString(s);
    if (defaultType==AB_Transaction_TypeUnknown) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid default transaction type \"%s\", assuming \"statement\"", s);
      defaultType=AB_Transaction_TypeStatement;
    }
  }

  dbT=GWEN_DB_GetFirstGroup(db);
  while (dbT) {
    int i;

    if (_groupNameMatches(GWEN_DB_GroupName(dbT), dbParams)) {
      /* possibly merge in/out values */
      if (splitValueInOut)
	_unsplitInOutValue(dbT, commaThousands, commaDecimal);

      if (GWEN_DB_GetCharValue(dbT, "value/value", 0, NULL) || GWEN_DB_GetCharValue(dbT, "units", 0, NULL)) {
        AB_TRANSACTION *t;
        const char *p;
        GWEN_DB_NODE *dbV;

        t=AB_Transaction_fromDb(dbT);
        if (!t) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in imported transaction data");
          GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, "Error in imported transaction data");
          return GWEN_ERROR_GENERIC;
        }

	_collectPurposeStrings(t, dbT);
	_readValues(t, dbT, commaThousands, commaDecimal);
	_readDates(t, dbT, dateFormat);

	if (usePosNegField)
	  _translateValuesSign(t, dbT, dbParams);

	if (switchLocalRemote)
	  _switchLocalRemoteAccordingToSign(t, switchOnNegative);

	if (AB_Transaction_GetType(t)<=AB_Transaction_TypeNone)
	  AB_Transaction_SetType(t, defaultType);

        DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding transaction");
        AB_ImExporterContext_AddTransaction(ctx, t);
      }
      else {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Empty group (i.e. empty line in imported file)");
      }
    }
    else {
      int rv;

      DBG_INFO(AQBANKING_LOGDOMAIN, "Not a transaction, checking subgroups");
      /* not a transaction, check subgroups */
      rv=AH_ImExporterCSV__ImportFromGroup(ctx, dbT, dbParams);
      if (rv) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "here");
        return rv;
      }
    }

    dbT=GWEN_DB_GetNextGroup(dbT);
  } // while

  return 0;
}



void _unsplitInOutValue(GWEN_DB_NODE *dbT, int commaThousands, int commaDecimal)
{
  AB_VALUE *tv=NULL;
  const char *s;
  const char *tc;

  tc=GWEN_DB_GetCharValue(dbT, "value/currency", 0, NULL);
  s=GWEN_DB_GetCharValue(dbT, "valueIn/value", 0, 0);
  if (s && *s) {
    GWEN_DB_NODE *dbV;

    dbV=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "valueIn");
    tv=_valueFromDb(dbV, commaThousands, commaDecimal);
  }
  else {
    s=GWEN_DB_GetCharValue(dbT, "valueOut/value", 0, 0);
    if (s && *s) {
      GWEN_DB_NODE *dbV;

      dbV=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "valueOut");
      if (dbV) {
        tv=_valueFromDb(dbV, commaThousands, commaDecimal);
        if (!AB_Value_IsNegative(tv))
          /* outgoing but positive, negate */
          AB_Value_Negate(tv);
      }
    }
  }

  if (tv) {
    GWEN_DB_NODE *dbTV;

    if (tc)
      AB_Value_SetCurrency(tv, tc);
    dbTV=GWEN_DB_GetGroup(dbT, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "value");
    AB_Value_toDb(tv, dbTV);
    AB_Value_free(tv);
  }
}



void _collectPurposeStrings(AB_TRANSACTION *t, GWEN_DB_NODE *dbT)
{
  const char *p;

  /* possibly translate purpose */
  p=GWEN_DB_GetCharValue(dbT, "purpose", 1, 0); /* "1" is correct here! */
  if (p) {
    int i;
  
    /* there are multiple purpose lines, read them properly */
    AB_Transaction_SetPurpose(t, NULL);
    for (i=0; i<99; i++) {
      p=GWEN_DB_GetCharValue(dbT, "purpose", i, NULL);
      if (p && *p)
	AB_Transaction_AddPurposeLine(t, p);
    }
  }
}



void _readValues(AB_TRANSACTION *t, GWEN_DB_NODE *dbT, int commaThousands, int commaDecimal)
{
  GWEN_DB_NODE *dbV;
  AB_VALUE *v=NULL;

  /* translate values */
  dbV=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "value");
  if (dbV) {
    v=_valueFromDb(dbV, commaThousands, commaDecimal);
    AB_Transaction_SetValue(t, v);
    AB_Value_free(v);
  }

  dbV=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "priceValue");
  if (dbV) {
    v=_valueFromDb(dbV, commaThousands, commaDecimal);
    AB_Transaction_SetUnitPriceValue(t, v);
    AB_Value_free(v);
  }

  dbV=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "fees");
  if (dbV) {
    v=_valueFromDb(dbV, commaThousands, commaDecimal);
    AB_Transaction_SetFees(t, v);
    AB_Value_free(v);
  }

  dbV=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "taxes");
  if (dbV) {
    v=_valueFromDb(dbV, commaThousands, commaDecimal);
    AB_Transaction_SetTaxes(t, v);
    AB_Value_free(v);
  }

  dbV=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "commissionValue");
  if (dbV) {
    v=_valueFromDb(dbV, commaThousands, commaDecimal);
    AB_Transaction_SetCommissionValue(t, v);
    AB_Value_free(v);
  }
}



void _readDates(AB_TRANSACTION *t, GWEN_DB_NODE *dbT, const char *dateFormat)
{
  const char *p;

  p=GWEN_DB_GetCharValue(dbT, "date", 0, 0);
  if (p) {
    GWEN_DATE *da;
  
    da=GWEN_Date_fromStringWithTemplate(p, dateFormat);
    if (da)
      AB_Transaction_SetDate(t, da);
    GWEN_Date_free(da);
  }
  
  p=GWEN_DB_GetCharValue(dbT, "valutaDate", 0, 0);
  if (p) {
    GWEN_DATE *da;
  
    da=GWEN_Date_fromStringWithTemplate(p, dateFormat);
    if (da)
      AB_Transaction_SetValutaDate(t, da);
    GWEN_Date_free(da);
  }
  
  p=GWEN_DB_GetCharValue(dbT, "mandateDate", 0, 0);
  if (p) {
    GWEN_DATE *dt;
  
    dt=GWEN_Date_fromStringWithTemplate(p, dateFormat);
    if (dt) {
      AB_Transaction_SetMandateDate(t, dt);
      GWEN_Date_free(dt);
    }
  }
  p=GWEN_DB_GetCharValue(dbT, "firstDate", 0, 0);
  if (p) {
    GWEN_DATE *da;
  
    da=GWEN_Date_fromStringWithTemplate(p, dateFormat);
    if (da)
      AB_Transaction_SetFirstDate(t, da);
    GWEN_Date_free(da);
  }
  p=GWEN_DB_GetCharValue(dbT, "lastDate", 0, 0);
  if (p) {
    GWEN_DATE *da;
  
    da=GWEN_Date_fromStringWithTemplate(p, dateFormat);
    if (da)
      AB_Transaction_SetLastDate(t, da);
    GWEN_Date_free(da);
  }
  p=GWEN_DB_GetCharValue(dbT, "nextDate", 0, 0);
  if (p) {
    GWEN_DATE *da;
  
    da=GWEN_Date_fromStringWithTemplate(p, dateFormat);
    if (da)
      AB_Transaction_SetNextDate(t, da);
    GWEN_Date_free(da);
  }
}



AB_VALUE *_valueFromDb(GWEN_DB_NODE *dbV, int commaThousands, int commaDecimal)
{
  const char *sv;
  const char *sc;
  char *cbuf=NULL;
  AB_VALUE *val;

  sv=GWEN_DB_GetCharValue(dbV, "value", 0, 0);
  sc=GWEN_DB_GetCharValue(dbV, "currency", 0, "EUR");
  if (commaThousands || commaDecimal) {
    const char *pSrc;
    char *pDst;

    cbuf=(char *) malloc(strlen(sv)+1);
    pSrc=sv;
    pDst=cbuf;

    /* copy all but thousands commas to new buffer */
    while (*pSrc) {
      if (commaThousands && *pSrc==commaThousands) {
        /* skip thousands comma */
      }
      else if (commaDecimal && *pSrc==commaDecimal)
        /* replace whatever is given by a recognizable decimal point */
        *(pDst++)='.';
      else
        *(pDst++)=*pSrc;
      pSrc++;
    }
    /* add trailing 0 to end the string */
    *pDst=0;

    sv=(const char *) cbuf;
  }

  val=AB_Value_fromString(sv);
  if (cbuf)
    free(cbuf);
  if (val && sc)
    AB_Value_SetCurrency(val, sc);

  return val;
}



void _translateValuesSign(AB_TRANSACTION *t, GWEN_DB_NODE *dbT, GWEN_DB_NODE *dbParams)
{
  if (_mustNegate(dbT, dbParams)) {
    const AB_VALUE *pv;

    pv=AB_Transaction_GetValue(t);
    if (pv) {
      AB_VALUE *v;

      v=AB_Value_dup(pv);
      AB_Value_Negate(v);
      AB_Transaction_SetValue(t, v);
      AB_Value_free(v);
    }

    pv=AB_Transaction_GetUnits(t);
    if (pv) {
      AB_VALUE *v;

      v=AB_Value_dup(pv);
      AB_Value_Negate(v);
      AB_Transaction_SetUnits(t, v);
      AB_Value_free(v);
    }
  }
}



int _mustNegate(GWEN_DB_NODE *dbT, GWEN_DB_NODE *dbParams)
{
  int usePosNegField;
  const char *posNegFieldName;
  int defaultIsPositive;
  const char *s;
  int determined=0;

  usePosNegField=GWEN_DB_GetIntValue(dbParams, "usePosNegField", 0, 0);
  defaultIsPositive=GWEN_DB_GetIntValue(dbParams, "defaultIsPositive", 0, 1);
  posNegFieldName=GWEN_DB_GetCharValue(dbParams, "posNegFieldName", 0, "posNeg");
  
  /* get positive/negative mark */
  s=GWEN_DB_GetCharValue(dbT, posNegFieldName, 0, 0);
  if (s) {
    int j;

    /* try positive marks first */
    for (j=0; ; j++) {
      const char *patt;

      patt=GWEN_DB_GetCharValue(dbParams, "positiveValues", j, 0);
      if (!patt)
	break;
      if (-1!=GWEN_Text_ComparePattern(s, patt, 0)) {
	/* value already is positive, keep it that way */
	determined=1;
	break;
      }
    } /* for */

    if (!determined) {
      for (j=0; ; j++) {
	const char *patt;

	patt=GWEN_DB_GetCharValue(dbParams, "negativeValues", j, 0);
	if (!patt)
	  break;
	if (-1!=GWEN_Text_ComparePattern(s, patt, 0))
	  return 1;
      } /* for */
    }
  }

  /* still undecided? */
  if (!determined && !defaultIsPositive)
    return 1;

  return 0;
}



void _switchLocalRemoteAccordingToSign(AB_TRANSACTION *t, int switchOnNegative)
{
  const AB_VALUE *pv;
  
  /* value must be negated, because default is negative */
  pv=AB_Transaction_GetValue(t);
  if (pv) {
    if (!(AB_Value_IsNegative(pv) ^ (switchOnNegative!=0))) {
      const char *s;
      GWEN_BUFFER *b1;
      GWEN_BUFFER *b2;
  
      /* need to switch local/remote name */
      b1=GWEN_Buffer_new(0, 64, 0, 1);
      b2=GWEN_Buffer_new(0, 64, 0, 1);
  
      /* get data */
      s=AB_Transaction_GetLocalName(t);
      if (s && *s)
	GWEN_Buffer_AppendString(b1, s);
      s=AB_Transaction_GetRemoteName(t);
      if (s && *s)
	GWEN_Buffer_AppendString(b2, s);
  
      /* set reverse */
      if (GWEN_Buffer_GetUsedBytes(b1))
	AB_Transaction_SetRemoteName(t, GWEN_Buffer_GetStart(b1));
  
      if (GWEN_Buffer_GetUsedBytes(b2))
	AB_Transaction_SetLocalName(t, GWEN_Buffer_GetStart(b2));
  
      /* cleanup */
      GWEN_Buffer_free(b2);
      GWEN_Buffer_free(b1);
    }
  }
}



int _groupNameMatches(const char *groupName, GWEN_DB_NODE *dbParams)
{
  int i;
  const char *p;
  
  for (i=0; ; i++) {
    p=GWEN_DB_GetCharValue(dbParams, "groupNames", i, NULL);
    if (!p)
      break;
    if (strcasecmp(groupName, p)==0)
      return 1;
  } /* for */
  
  if (i==0) {
    /* no names given, check default */
    if ((strcasecmp(groupName, "transaction")==0) ||
	(strcasecmp(groupName, "debitnote")==0) ||
	(strcasecmp(groupName, "line")==0))
      return 1;
  }

  return 0;
}




int AH_ImExporterCSV_CheckFile(AB_IMEXPORTER *ie, const char *fname)
{
  AH_IMEXPORTER_CSV *ieh;
  GWEN_DBIO_CHECKFILE_RESULT rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CSV, ie);
  assert(ieh);
  assert(ieh->dbio);

  rv=GWEN_DBIO_CheckFile(ieh->dbio, fname);
  switch (rv) {
  case GWEN_DBIO_CheckFileResultOk:
    return 0;
  case GWEN_DBIO_CheckFileResultNotOk:
    return GWEN_ERROR_BAD_DATA;
  case GWEN_DBIO_CheckFileResultUnknown:
    return AB_ERROR_INDIFFERENT;
  default:
    return GWEN_ERROR_GENERIC;
  } /* switch */
}



int AH_ImExporterCSV_Export(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_SYNCIO *sio,
                            GWEN_DB_NODE *params)
{
  AH_IMEXPORTER_CSV *ieh;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbSubParams;
  int rv;
  const char *dateFormat;
  int usePosNegField;
  //int defaultIsPositive;
  int splitValueInOut;
  const char *posNegFieldName;
  const char *valueFormat;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CSV, ie);
  assert(ieh);
  assert(ieh->dbio);

  dbSubParams=GWEN_DB_GetGroup(params, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "params");
  dateFormat=GWEN_DB_GetCharValue(params, "dateFormat", 0, "YYYY/MM/DD");
  usePosNegField=GWEN_DB_GetIntValue(params, "usePosNegField", 0, 0);
  //defaultIsPositive=GWEN_DB_GetIntValue(params, "defaultIsPositive", 0, 1);
  posNegFieldName=GWEN_DB_GetCharValue(params, "posNegFieldName", 0, "posNeg");
  splitValueInOut=GWEN_DB_GetIntValue(params, "splitValueInOut", 0, 0);

  valueFormat=GWEN_DB_GetCharValue(params, "valueFormat", 0, "float");

  /* create db, store transactions in it */
  dbData=GWEN_DB_Group_new("transactions");
  ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while (ai) {
    const AB_TRANSACTION_LIST *tl;

    tl=AB_ImExporterAccountInfo_GetTransactionList(ai);
    if (tl) {
      const AB_TRANSACTION *t;

      t=AB_Transaction_List_First(tl);
      while (t) {
        GWEN_DB_NODE *dbTransaction;
        const GWEN_DATE *dt;
        const char *s;

        dbTransaction=GWEN_DB_Group_new("transaction");
        rv=AB_Transaction_toDb(t, dbTransaction);
        if (rv) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Could not transform transaction to db");
          GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                               "Error transforming data to db");
          GWEN_DB_Group_free(dbData);
          GWEN_DB_Group_free(dbTransaction);
          return GWEN_ERROR_GENERIC;
        }

        /* translate purpose */
        s=AB_Transaction_GetPurpose(t);
        if (s && *s) {
          GWEN_STRINGLIST *sl;

          sl=GWEN_StringList_fromString(s, "\n", 0);
          if (sl) {
            GWEN_STRINGLISTENTRY *se;

            GWEN_DB_DeleteVar(dbTransaction, "purpose");
            se=GWEN_StringList_FirstEntry(sl);
            while (se) {
              const char *p;

              p=GWEN_StringListEntry_Data(se);
              if (p && *p)
                GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_DEFAULT, "purpose", p);
              se=GWEN_StringListEntry_Next(se);
            }
            GWEN_StringList_free(sl);
          }
        }

        /* transform dates */
        GWEN_DB_DeleteGroup(dbTransaction, "date");
        GWEN_DB_DeleteGroup(dbTransaction, "valutaDate");
        GWEN_DB_DeleteGroup(dbTransaction, "mandateDate");

        dt=AB_Transaction_GetDate(t);
        if (dt) {
          GWEN_BUFFER *tbuf;
          int rv;

          tbuf=GWEN_Buffer_new(0, 32, 0, 1);
          rv=GWEN_Date_toStringWithTemplate(dt, dateFormat, tbuf);
          if (rv<0) {
            DBG_WARN(AQBANKING_LOGDOMAIN, "Bad date format string/date");
          }
          else
            GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                 "date", GWEN_Buffer_GetStart(tbuf));
          GWEN_Buffer_free(tbuf);
        }

        dt=AB_Transaction_GetValutaDate(t);
        if (dt) {
          GWEN_BUFFER *tbuf;
          int rv;

          tbuf=GWEN_Buffer_new(0, 32, 0, 1);
          rv=GWEN_Date_toStringWithTemplate(dt, dateFormat, tbuf);
          if (rv<0) {
            DBG_WARN(AQBANKING_LOGDOMAIN, "Bad date format string/date");
          }
          else
            GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                 "valutaDate", GWEN_Buffer_GetStart(tbuf));
          GWEN_Buffer_free(tbuf);
        }

        dt=AB_Transaction_GetMandateDate(t);
        if (dt) {
          GWEN_BUFFER *tbuf;
          int rv;

          tbuf=GWEN_Buffer_new(0, 32, 0, 1);
          rv=GWEN_Date_toStringWithTemplate(dt, dateFormat, tbuf);
          if (rv<0) {
            DBG_WARN(AQBANKING_LOGDOMAIN, "Bad date format string/date");
          }
          else
            GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                 "mandateDate", GWEN_Buffer_GetStart(tbuf));
          GWEN_Buffer_free(tbuf);
        }

        /* possibly transform value */
        if (usePosNegField) {
          const AB_VALUE *v;
          const char *s;

          v=AB_Transaction_GetValue(t);
          if (v) {
            if (!AB_Value_IsNegative(v)) {
              s=GWEN_DB_GetCharValue(params, "positiveValues", 0, 0);
              if (s) {
                GWEN_DB_SetCharValue(dbTransaction,
                                     GWEN_DB_FLAGS_OVERWRITE_VARS,
                                     posNegFieldName,
                                     s);
              }
              else {
                DBG_ERROR(AQBANKING_LOGDOMAIN,
                          "No value for \"positiveValues\" in params");
                GWEN_DB_Group_free(dbData);
                return GWEN_ERROR_GENERIC;
              }
            }
            else {
              s=GWEN_DB_GetCharValue(params, "negativeValues", 0, 0);
              if (s) {
                AB_VALUE *nv;
                GWEN_DB_NODE *dbV;

                GWEN_DB_SetCharValue(dbTransaction,
                                     GWEN_DB_FLAGS_OVERWRITE_VARS,
                                     posNegFieldName,
                                     s);
                nv=AB_Value_dup(v);
                AB_Value_Negate(nv);
                dbV=GWEN_DB_GetGroup(dbTransaction,
                                     GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                                     "value");
                assert(dbV);
                if (AB_Value_toDb(nv, dbV)) {
                  DBG_ERROR(AQBANKING_LOGDOMAIN,
                            "Could not store value to DB");
                  GWEN_DB_Group_free(dbData);
                  return GWEN_ERROR_GENERIC;
                }
              }
              else {
                DBG_ERROR(AQBANKING_LOGDOMAIN,
                          "No value for \"negativeValues\" in params");
                GWEN_DB_Group_free(dbData);
                return GWEN_ERROR_GENERIC;
              }
            }
          }
        }
        else if (splitValueInOut) {
          const AB_VALUE *v;

          v=AB_Transaction_GetValue(t);
          if (v) {
            const char *gn;
            GWEN_DB_NODE *dbV;

            if (AB_Value_IsNegative(v))
              gn="valueOut";
            else
              gn="valueIn";
            dbV=GWEN_DB_GetGroup(dbTransaction,
                                 GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                                 gn);
            assert(dbV);
            if (strcasecmp(valueFormat, "float")==0)
              AB_Value_toDbFloat(v, dbV);
            else
              AB_Value_toDb(v, dbV);

            GWEN_DB_ClearGroup(dbTransaction, "value");
          }
        }
        else {
          const AB_VALUE *v;

          v=AB_Transaction_GetValue(t);
          if (v) {
            GWEN_DB_NODE *dbV;

            GWEN_DB_DeleteVar(dbTransaction, "value");
            dbV=GWEN_DB_GetGroup(dbTransaction,
                                 GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                                 "value");
            assert(dbV);
            if (strcasecmp(valueFormat, "float")==0)
              AB_Value_toDbFloat(v, dbV);
            else
              AB_Value_toDb(v, dbV);
          }
        }

        /* add transaction db */
        GWEN_DB_AddGroup(dbData, dbTransaction);

        t=AB_Transaction_List_Next(t);
      } /* while t */
    } /* if tl */
    ai=AB_ImExporterAccountInfo_List_Next(ai);
  } /* while ai */

  rv=GWEN_DBIO_Export(ieh->dbio, sio, dbData, dbSubParams, GWEN_DB_FLAGS_DEFAULT);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error exporting data (%d)", rv);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         "Error exporting data");
    GWEN_DB_Group_free(dbData);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_DB_Group_free(dbData);

  return 0;
}



int AH_ImExporterCSV_GetEditProfileDialog(AB_IMEXPORTER *ie,
                                          GWEN_DB_NODE *dbProfile,
                                          const char *testFileName,
                                          GWEN_DIALOG **pDlg)
{
  GWEN_DIALOG *dlg;

  dlg=AB_CSV_EditProfileDialog_new(ie, dbProfile, testFileName);
  if (dlg==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Unable to create the dialog");
    return GWEN_ERROR_INTERNAL;
  }
  *pDlg=dlg;
  return 0;
}





