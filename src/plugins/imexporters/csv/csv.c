/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004,2012 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "csv_p.h"
#include "csv_editprofile_l.h"
#include "i18n_l.h"

#include "imexporter_be.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/gui.h>


GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_CSV);


GWEN_PLUGIN *imexporter_csv_factory(GWEN_PLUGIN_MANAGER *pm,
				    const char *name,
				    const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_ImExporter_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_ImExporter_SetFactoryFn(pl, AB_Plugin_ImExporterCSV_Factory);

  return pl;
}



AB_IMEXPORTER *AB_Plugin_ImExporterCSV_Factory(GWEN_PLUGIN *pl,
					       AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_CSV *ieh;

  ie=AB_ImExporter_new(ab, "csv");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_CSV, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CSV, ie, ieh,
                       AH_ImExporterCSV_FreeData);
  ieh->dbio=GWEN_DBIO_GetPlugin("csv");
  if (!ieh->dbio) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "GWEN DBIO plugin \"CSV\" not available");
    AB_ImExporter_free(ie);
    return 0;
  }

  AB_ImExporter_SetImportFn(ie, AH_ImExporterCSV_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterCSV_Export);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterCSV_CheckFile);
  AB_ImExporter_SetGetEditProfileDialogFn(ie, AH_ImExporterCSV_GetEditProfileDialog);

  /* announce special features */
  AB_ImExporter_AddFlags(ie, AB_IMEXPORTER_FLAGS_GETPROFILEEDITOR_SUPPORTED);

  return ie;
}



void GWENHYWFAR_CB AH_ImExporterCSV_FreeData(void *bp, void *p){
  AH_IMEXPORTER_CSV *ieh;

  ieh=(AH_IMEXPORTER_CSV*)p;
  GWEN_DBIO_free(ieh->dbio);
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterCSV_Import(AB_IMEXPORTER *ie,
			    AB_IMEXPORTER_CONTEXT *ctx,
			    GWEN_SYNCIO *sio,
			    GWEN_DB_NODE *params){
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



AB_VALUE *AH_ImExporterCSV__ValueFromDb(GWEN_DB_NODE *dbV, int commaThousands, int commaDecimal) {
  const char *sv;
  const char *sc;
  char *cbuf=NULL;
  AB_VALUE *val;

  sv=GWEN_DB_GetCharValue(dbV, "value", 0, 0);
  sc=GWEN_DB_GetCharValue(dbV, "currency", 0, "EUR");
  if (commaThousands || commaDecimal) {
    const char *pSrc;
    char *pDst;

    cbuf=(char*) malloc(strlen(sv)+1);
    pSrc=sv;
    pDst=cbuf;

    /* copy all but thousands commas to new buffer */
    while(*pSrc) {
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

    sv=(const char*) cbuf;
  }

  val=AB_Value_fromString(sv);
  if (cbuf)
    free(cbuf);
  if (val && sc)
    AB_Value_SetCurrency(val, sc);

  return val;
}



int AH_ImExporterCSV__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                      GWEN_DB_NODE *db,
				      GWEN_DB_NODE *dbParams) {
  GWEN_DB_NODE *dbT;
  const char *dateFormat;
  int inUtc;
  int usePosNegField;
  int splitValueInOut;
  int defaultIsPositive;
  int switchLocalRemote;
  int switchOnNegative;
  const char *posNegFieldName;
  int commaThousands=0;
  int commaDecimal=0;
  uint32_t progressId;
  const char *s;

  dateFormat=GWEN_DB_GetCharValue(dbParams, "dateFormat", 0, "YYYY/MM/DD");
  inUtc=GWEN_DB_GetIntValue(dbParams, "utc", 0, 0);
  usePosNegField=GWEN_DB_GetIntValue(dbParams, "usePosNegField", 0, 0);
  defaultIsPositive=GWEN_DB_GetIntValue(dbParams, "defaultIsPositive", 0, 1);
  posNegFieldName=GWEN_DB_GetCharValue(dbParams, "posNegFieldName", 0,
				       "posNeg");
  splitValueInOut=GWEN_DB_GetIntValue(dbParams, "splitValueInOut", 0, 0);

  switchLocalRemote=GWEN_DB_GetIntValue(dbParams, "switchLocalRemote", 0, 0);
  switchOnNegative=GWEN_DB_GetIntValue(dbParams, "switchOnNegative", 0, 1);

  s=GWEN_DB_GetCharValue(dbParams, "commaThousands", 0, 0);
  if (s)
    commaThousands=*s;
  s=GWEN_DB_GetCharValue(dbParams, "commaDecimal", 0, 0);
  if (s)
    commaDecimal=*s;

  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
				    GWEN_GUI_PROGRESS_ALLOW_EMBED |
				    GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				    GWEN_GUI_PROGRESS_SHOW_ABORT,
				    I18N("Importing parsed data..."),
				    NULL,
				    GWEN_DB_Groups_Count(db),
				    0);
  dbT=GWEN_DB_GetFirstGroup(db);
  while(dbT) {
    int matches;
    int i;
    const char *p;
    const char *gn;

    /* check whether the name of the current groups matches */
    matches=0;
    gn=GWEN_DB_GroupName(dbT);
    for (i=0; ; i++) {
      p=GWEN_DB_GetCharValue(dbParams, "groupNames", i, 0);
      if (!p)
        break;
      if (strcasecmp(gn, p)==0) {
        matches=1;
        break;
      }
    } /* for */

    if (!matches && i==0) {
      /* no names given, check default */
      if ((strcasecmp(GWEN_DB_GroupName(dbT), "transaction")==0) ||
          (strcasecmp(GWEN_DB_GroupName(dbT), "debitnote")==0) ||
          (strcasecmp(GWEN_DB_GroupName(dbT), "line")==0))
        matches=1;
    }

    if (matches) {
      /* possibly merge in/out values */
      if (splitValueInOut) {
	AB_VALUE *tv=NULL;
	const char *s;
        const char *tc;

	tc=GWEN_DB_GetCharValue(dbT, "value/currency", 0, NULL);
	s=GWEN_DB_GetCharValue(dbT, "valueIn/value", 0, 0);
	if (s && *s) {
	  GWEN_DB_NODE *dbV;

	  dbV=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "valueIn");
	  tv=AH_ImExporterCSV__ValueFromDb(dbV, commaThousands, commaDecimal);
	}
	else {
	  s=GWEN_DB_GetCharValue(dbT, "valueOut/value", 0, 0);
	  if (s && *s) {
	    GWEN_DB_NODE *dbV;

	    dbV=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "valueOut");
	    if (dbV) {
	      tv=AH_ImExporterCSV__ValueFromDb(dbV, commaThousands, commaDecimal);
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

      if (GWEN_DB_GetCharValue(dbT, "value/value", 0, 0)) {
        AB_TRANSACTION *t;
        const char *p;

        DBG_DEBUG(AQBANKING_LOGDOMAIN, "Found a possible transaction");
        t=AB_Transaction_fromDb(dbT);
        if (!t) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in config file");
          GWEN_Gui_ProgressLog(progressId, GWEN_LoggerLevel_Error,
			       "Error in config file");
	  GWEN_Gui_ProgressEnd(progressId);
          return GWEN_ERROR_GENERIC;
        }
  
        /* translate date */
        p=GWEN_DB_GetCharValue(dbT, "date", 0, 0);
        if (p) {
          GWEN_TIME *ti;
  
          ti=AB_ImExporter_DateFromString(p, dateFormat, inUtc);
          if (ti)
            AB_Transaction_SetDate(t, ti);
          GWEN_Time_free(ti);
        }
  
        /* translate valutaDate */
        p=GWEN_DB_GetCharValue(dbT, "valutaDate", 0, 0);
        if (p) {
          GWEN_TIME *ti;
  
          ti=AB_ImExporter_DateFromString(p, dateFormat, inUtc);
          if (ti)
            AB_Transaction_SetValutaDate(t, ti);
          GWEN_Time_free(ti);
        }

        /* translate mandateDate */
        p=GWEN_DB_GetCharValue(dbT, "mandateDate", 0, 0);
        if (p) {
	  GWEN_DATE *dt;

	  dt=GWEN_Date_fromStringWithTemplate(p, dateFormat);
	  if (dt) {
	    AB_Transaction_SetMandateDate(t, dt);
	    GWEN_Date_free(dt);
	  }
	}

        /* possibly translate value */
	if (usePosNegField) {
          const char *s;
	  int determined=0;

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
		if (-1!=GWEN_Text_ComparePattern(s, patt, 0)) {
                  const AB_VALUE *pv;

		  /* value must be negated */
		  pv=AB_Transaction_GetValue(t);
		  if (pv) {
		    AB_VALUE *v;

		    v=AB_Value_dup(pv);
                    AB_Value_Negate(v);
		    AB_Transaction_SetValue(t, v);
                    AB_Value_free(v);
		  }
		  determined=1;
                  break;
		}
	      } /* for */
	    }
	  }

	  /* still undecided? */
	  if (!determined && !defaultIsPositive) {
	    const AB_VALUE *pv;

	    /* value must be negated, because default is negative */
	    pv=AB_Transaction_GetValue(t);
	    if (pv) {
	      AB_VALUE *v;

	      v=AB_Value_dup(pv);
	      AB_Value_Negate(v);
	      AB_Transaction_SetValue(t, v);
	      AB_Value_free(v);
	    }
	  }

        } /* if usePosNegField */

	else if (switchLocalRemote) {
	  const AB_VALUE *pv;

	  /* value must be negated, because default is negative */
	  pv=AB_Transaction_GetValue(t);
	  if (pv) {
	    if (!(AB_Value_IsNegative(pv) ^ (switchOnNegative!=0))) {
	      const GWEN_STRINGLIST *csl;
	      GWEN_BUFFER *b1;
	      GWEN_BUFFER *b2;

	      /* need to switch local/remote name */
	      b1=GWEN_Buffer_new(0, 64, 0, 1);
	      b2=GWEN_Buffer_new(0, 64, 0, 1);

	      /* get data */
	      csl=AB_Transaction_GetRemoteName(t);
	      if (csl && GWEN_StringList_Count(csl))
		GWEN_Buffer_AppendString(b1, GWEN_StringList_FirstString(csl));
	      if (AB_Transaction_GetLocalName(t))
		GWEN_Buffer_AppendString(b2, AB_Transaction_GetLocalName(t));

              /* clear */
	      AB_Transaction_ClearRemoteName(t);
	      AB_Transaction_SetLocalName(t, NULL);

              /* set reversed */
	      if (GWEN_Buffer_GetUsedBytes(b2))
		AB_Transaction_AddRemoteName(t, GWEN_Buffer_GetStart(b2), 0);

	      if (GWEN_Buffer_GetUsedBytes(b1))
		AB_Transaction_SetLocalName(t, GWEN_Buffer_GetStart(b1));

	      /* cleanup */
              GWEN_Buffer_free(b2);
	      GWEN_Buffer_free(b1);
	    }
	  }
	}

        DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding transaction");
        AB_ImExporterContext_AddTransaction(ctx, t);
      }
      else {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Empty group");
      }
    }
    else {
      int rv;

      DBG_INFO(AQBANKING_LOGDOMAIN, "Not a transaction, checking subgroups");
      /* not a transaction, check subgroups */
      rv=AH_ImExporterCSV__ImportFromGroup(ctx, dbT, dbParams);
      if (rv) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "here");
	GWEN_Gui_ProgressEnd(progressId);
        return rv;
      }
    }

    if (GWEN_Gui_ProgressAdvance(progressId, GWEN_GUI_PROGRESS_ONE)==
	GWEN_ERROR_USER_ABORTED) {
      GWEN_Gui_ProgressLog(progressId, GWEN_LoggerLevel_Error,
			   I18N("Aborted by user"));
      GWEN_Gui_ProgressEnd(progressId);
      return GWEN_ERROR_USER_ABORTED;
    }

    dbT=GWEN_DB_GetNextGroup(dbT);
  } // while

  GWEN_Gui_ProgressEnd(progressId);

  return 0;
}



int AH_ImExporterCSV_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  AH_IMEXPORTER_CSV *ieh;
  GWEN_DBIO_CHECKFILE_RESULT rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CSV, ie);
  assert(ieh);
  assert(ieh->dbio);

  rv=GWEN_DBIO_CheckFile(ieh->dbio, fname);
  switch(rv) {
  case GWEN_DBIO_CheckFileResultOk:      return 0;
  case GWEN_DBIO_CheckFileResultNotOk:   return GWEN_ERROR_BAD_DATA;
  case GWEN_DBIO_CheckFileResultUnknown: return AB_ERROR_INDIFFERENT;
  default:                               return GWEN_ERROR_GENERIC;
  } /* switch */
}



int AH_ImExporterCSV__ExportTransactions(AB_IMEXPORTER *ie,
					 AB_IMEXPORTER_CONTEXT *ctx,
					 GWEN_SYNCIO *sio,
					 GWEN_DB_NODE *params,
					 int notedOrTransfers){
  AH_IMEXPORTER_CSV *ieh;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbSubParams;
  int rv;
  const char *dateFormat;
  int inUtc;
  int usePosNegField;
  //int defaultIsPositive;
  int splitValueInOut;
  const char *posNegFieldName;
  const char *valueFormat;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CSV, ie);
  assert(ieh);
  assert(ieh->dbio);

  dbSubParams=GWEN_DB_GetGroup(params, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                               "params");
  dateFormat=GWEN_DB_GetCharValue(params, "dateFormat", 0,
				  "YYYY/MM/DD");
  inUtc=GWEN_DB_GetIntValue(params, "utc", 0, 0);
  usePosNegField=GWEN_DB_GetIntValue(params, "usePosNegField", 0, 0);
  //defaultIsPositive=GWEN_DB_GetIntValue(params, "defaultIsPositive", 0, 1);
  posNegFieldName=GWEN_DB_GetCharValue(params, "posNegFieldName", 0,
				       "posNeg");
  splitValueInOut=GWEN_DB_GetIntValue(params, "splitValueInOut", 0, 0);

  valueFormat=GWEN_DB_GetCharValue(params, "valueFormat", 0, "float");

  /* create db, store transactions in it */
  dbData=GWEN_DB_Group_new("transactions");
  ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while(ai) {
    const AB_TRANSACTION *t;

    switch(notedOrTransfers) {
    case AH_IMEXPORTERCSV_SUBJECT_TRANSACTIONS:
      t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
      break;
    case AH_IMEXPORTERCSV_SUBJECT_NOTEDTRANSACTIONS:
      t=AB_ImExporterAccountInfo_GetFirstNotedTransaction(ai);
      break;
    case AH_IMEXPORTERCSV_SUBJECT_TRANSFERS:
      t=AB_ImExporterAccountInfo_GetFirstTransfer(ai);
      break;
    default:
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Invalid subject type %d", notedOrTransfers);
      GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Error,
			    "Invalid subject type %d", notedOrTransfers);
      GWEN_DB_Group_free(dbData);
      return GWEN_ERROR_GENERIC;
    }

    while(t) {
      GWEN_DB_NODE *dbTransaction;
      const GWEN_TIME *ti;
      const GWEN_DATE *dt;

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

      /* transform dates */
      GWEN_DB_DeleteGroup(dbTransaction, "date");
      GWEN_DB_DeleteGroup(dbTransaction, "valutaDate");
      GWEN_DB_DeleteGroup(dbTransaction, "mandateDate");

      ti=AB_Transaction_GetDate(t);
      if (ti) {
	GWEN_BUFFER *tbuf;
        int rv;

	tbuf=GWEN_Buffer_new(0, 32, 0, 1);
	if (inUtc)
	  rv=GWEN_Time_toUtcString(ti, dateFormat, tbuf);
	else
	  rv=GWEN_Time_toString(ti, dateFormat, tbuf);
	if (rv) {
	  DBG_WARN(AQBANKING_LOGDOMAIN, "Bad date format string/date");
	}
	else
	  GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS,
			       "date", GWEN_Buffer_GetStart(tbuf));
        GWEN_Buffer_free(tbuf);
      }

      ti=AB_Transaction_GetValutaDate(t);
      if (ti) {
	GWEN_BUFFER *tbuf;

	tbuf=GWEN_Buffer_new(0, 32, 0, 1);
	if (inUtc)
	  rv=GWEN_Time_toUtcString(ti, dateFormat, tbuf);
	else
	  rv=GWEN_Time_toString(ti, dateFormat, tbuf);
	if (rv) {
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

      if (strcasecmp(valueFormat, "float")==0) {
	GWEN_DB_NODE *dbV;

	dbV=GWEN_DB_GetGroup(dbTransaction,
			     GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			     "value");
	if (dbV) {
	  AB_VALUE *v;

	  v=AB_Value_fromDb(dbV);
	  if (v) {
	    GWEN_DB_ClearGroup(dbV, 0);
	    AB_Value_toDbFloat(v, dbV);
            AB_Value_free(v);
	  }
	}
      }

      /* add transaction db */
      GWEN_DB_AddGroup(dbData, dbTransaction);

      switch(notedOrTransfers) {
      case AH_IMEXPORTERCSV_SUBJECT_TRANSACTIONS:
	t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
	break;
      case AH_IMEXPORTERCSV_SUBJECT_NOTEDTRANSACTIONS:
	t=AB_ImExporterAccountInfo_GetNextNotedTransaction(ai);
	break;
      case AH_IMEXPORTERCSV_SUBJECT_TRANSFERS:
	t=AB_ImExporterAccountInfo_GetNextTransfer(ai);
	break;
      default:
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Invalid subject type %d", notedOrTransfers);
	GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Error,
			      "Invalid subject type %d", notedOrTransfers);
	GWEN_DB_Group_free(dbData);
	return GWEN_ERROR_GENERIC;
      }
    }
    ai=AB_ImExporterContext_GetNextAccountInfo(ctx);
  }

  rv=GWEN_DBIO_Export(ieh->dbio,
		      sio,
		      dbData,
		      dbSubParams,
		      GWEN_DB_FLAGS_DEFAULT);
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



int AH_ImExporterCSV_Export(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_SYNCIO *sio,
			    GWEN_DB_NODE *params){
  AH_IMEXPORTER_CSV *ieh;
  const char *subject;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CSV, ie);
  assert(ieh);
  assert(ieh->dbio);

  subject=GWEN_DB_GetCharValue(params, "subject", 0,
			       "transactions");
  if (strcasecmp(subject, "transactions")==0)
    return AH_ImExporterCSV__ExportTransactions(ie, ctx, sio, params, AH_IMEXPORTERCSV_SUBJECT_TRANSACTIONS);
  else if (strcasecmp(subject, "notedTransactions")==0)
    return AH_ImExporterCSV__ExportTransactions(ie, ctx, sio, params, AH_IMEXPORTERCSV_SUBJECT_NOTEDTRANSACTIONS);
  else if (strcasecmp(subject, "transfers")==0)
    return AH_ImExporterCSV__ExportTransactions(ie, ctx, sio, params, AH_IMEXPORTERCSV_SUBJECT_TRANSFERS);
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Unable to export unknown subject \"%s\"", subject);
    return GWEN_ERROR_INVALID;
  }
}



int AH_ImExporterCSV_GetEditProfileDialog(AB_IMEXPORTER *ie,
					  GWEN_DB_NODE *dbProfile,
					  const char *testFileName,
					  GWEN_DIALOG **pDlg) {
  GWEN_DIALOG *dlg;

  dlg=AB_CSV_EditProfileDialog_new(ie, dbProfile, testFileName);
  if (dlg==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Unable to create the dialog");
    return GWEN_ERROR_INTERNAL;
  }
  *pDlg=dlg;
  return 0;
}





