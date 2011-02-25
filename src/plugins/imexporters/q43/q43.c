/***************************************************************************
    begin       : Mon May 03 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "q43_p.h"


#include "i18n_l.h"
#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/gwendate.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/text.h>

#include <ctype.h>

#define YEAR_2000_CUTOFF 80



GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_Q43);



GWEN_PLUGIN *imexporter_q43_factory(GWEN_PLUGIN_MANAGER *pm,
				    const char *name,
				    const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_ImExporter_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_ImExporter_SetFactoryFn(pl, AB_Plugin_ImExporterQ43_Factory);

  return pl;
}



AB_IMEXPORTER *AB_Plugin_ImExporterQ43_Factory(GWEN_PLUGIN *pl, AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_Q43 *ieh;

  ie=AB_ImExporter_new(ab, "q43");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_Q43, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_Q43, ie, ieh,
                       AH_ImExporterQ43_FreeData);

  AB_ImExporter_SetImportFn(ie, AH_ImExporterQ43_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterQ43_Export);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterQ43_CheckFile);
  return ie;
}



void GWENHYWFAR_CB AH_ImExporterQ43_FreeData(void *bp, void *p){
  AH_IMEXPORTER_Q43 *ieh;

  ieh=(AH_IMEXPORTER_Q43*)p;
  GWEN_FREE_OBJECT(ieh);
}



/* this needs to be replaced later by a more generic approach */
const char *AH_ImExporterQ43_GetCurrencyCode(int code) {
  switch(code) {
  case 978: return "EUR"; break;
  default:
    break;
  }

  return NULL;
}



int AH_ImExporterQ43_ReadInt(const char *p, int len) {
  int res=0;
  int i;

  for (i=0; i<len; i++) {
    char c;

    c=*(p++);
    if (!isdigit(c))
      return res;
    res*=10;
    res+=c-'0';
  }

  return res;
}



int AH_ImExporterQ43_ReadDocument(AB_IMEXPORTER *ie,
				  AB_IMEXPORTER_CONTEXT *ctx,
                                  GWEN_FAST_BUFFER *fb,
				  GWEN_DB_NODE *params){
  AB_IMEXPORTER_ACCOUNTINFO *iea=NULL;
  AB_TRANSACTION *t=NULL;
  GWEN_DATE *date=NULL;
  GWEN_BUFFER *lbuf;
  const char *currency=NULL;
  int rv;
  int hadSome=0;
  int records=0;

  lbuf=GWEN_Buffer_new(0, 256, 0, 1);

  do {
    rv=GWEN_FastBuffer_ReadLineToBuffer(fb, lbuf);
    if (rv==0) {
      int code;
      char *p;
      int size;

      size=GWEN_Buffer_GetUsedBytes(lbuf);
      if (size<2) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Line too short (%d bytes)",
		  GWEN_Buffer_GetUsedBytes(lbuf));
	AB_Transaction_free(t);
	GWEN_Date_free(date);
	GWEN_Buffer_free(lbuf);
	return GWEN_ERROR_BAD_DATA;
      }
      p=GWEN_Buffer_GetStart(lbuf);
      code=((p[0]-'0')*10)+(p[1]-'0');
      DBG_INFO(AQBANKING_LOGDOMAIN, "Got record %02d", code);

      switch(code) {
      case 0: { /* file header */
	int y, m, d;

	if (size<12) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Record %02d too short (%d bytes)",
		    code, GWEN_Buffer_GetUsedBytes(lbuf));
	  AB_Transaction_free(t);
          GWEN_Date_free(date);
	  GWEN_Buffer_free(lbuf);
	  return GWEN_ERROR_BAD_DATA;
	}

        /* extract date */
	y=((p[6] -'0')*10)+(p[7] -'0');
	m=((p[8] -'0')*10)+(p[9] -'0');
	d=((p[10]-'0')*10)+(p[11]-'0');
	if (y>YEAR_2000_CUTOFF)
	  y+=1900;
	else
          y+=2000;
	GWEN_Date_free(date);
	date=GWEN_Date_fromGregorian(y, m, d);
	if (date==NULL) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid date in record %02d", code);
	  AB_Transaction_free(t);
	  GWEN_Date_free(date);
	  GWEN_Buffer_free(lbuf);
	  return GWEN_ERROR_BAD_DATA;
	}
        break;
      }

      case 11: { /* account header */
	char bankCode[9];
	char accountNumber[11];
        int cy;

	if (size<80) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Record %02d too short (%d bytes)",
		    code, GWEN_Buffer_GetUsedBytes(lbuf));
	  AB_Transaction_free(t);
          GWEN_Date_free(date);
	  GWEN_Buffer_free(lbuf);
	  return GWEN_ERROR_BAD_DATA;
	}

	/* get bankcode (combine bank code key and office branch code */
	strncpy(bankCode, p+2, 8);
	bankCode[8]=0;

	/* get account number */
	strncpy(accountNumber, p+10, 10);
	accountNumber[10]=0;

        /* get account info (or create it if necessary) */
	iea=AB_ImExporterContext_GetAccountInfo(ctx, bankCode, accountNumber);
	assert(iea);

	cy=((p[47]-'0')*100)+((p[48]-'0')*10)+(p[49]-'0');
	currency=AH_ImExporterQ43_GetCurrencyCode(cy);
	if (!currency) {
	  DBG_WARN(AQBANKING_LOGDOMAIN, "Unknown currency code %d, ignoring", cy);
	}

	/* TODO: read initial balance */
        hadSome++;
        break;
      }

      case 22: {
	GWEN_TIME *ti;
        AB_VALUE *v;
	int y, m, d;
	char amount[32];
        const char *s;

	if (size<80) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Record %02d too short (%d bytes)",
		    code, GWEN_Buffer_GetUsedBytes(lbuf));
	  AB_Transaction_free(t);
	  GWEN_Date_free(date);
	  GWEN_Buffer_free(lbuf);
	  return GWEN_ERROR_BAD_DATA;
	}

	if (iea==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad order of records (22 before 11)");
	  AB_Transaction_free(t);
	  GWEN_Date_free(date);
	  GWEN_Buffer_free(lbuf);
	  return GWEN_ERROR_BAD_DATA;
	}

	if (t)
	  AB_ImExporterAccountInfo_AddTransaction(iea, t);
	t=AB_Transaction_new();

	/* extract booking date */
	y=((p[10]-'0')*10)+(p[11]-'0');
	m=((p[12]-'0')*10)+(p[13]-'0');
	d=((p[14]-'0')*10)+(p[15]-'0');
	if (y>YEAR_2000_CUTOFF)
	  y+=1900;
	else
          y+=2000;
	ti=GWEN_Time_new(y, m-1, d, 12, 0, 0, 1);
	if (ti==NULL) {
	  DBG_WARN(AQBANKING_LOGDOMAIN, "Invalid booking date in record %02d, ignoring", code);
	}
	else {
	  AB_Transaction_SetDate(t, ti);
	  GWEN_Time_free(ti);
	}

	/* extract valuta date */
	y=((p[16]-'0')*10)+(p[17]-'0');
	m=((p[18]-'0')*10)+(p[18]-'0');
	d=((p[19]-'0')*10)+(p[20]-'0');
	if (y>YEAR_2000_CUTOFF)
	  y+=1900;
	else
          y+=2000;
	ti=GWEN_Time_new(y, m-1, d, 12, 0, 0, 1);
	if (ti==NULL) {
	  DBG_WARN(AQBANKING_LOGDOMAIN, "Invalid valuta date in record %02d, ignoring", code);
	}
	else {
	  AB_Transaction_SetValutaDate(t, ti);
	  GWEN_Time_free(ti);
	}

	/* get amount */
	strncpy(amount, p+28, 14);
	amount[14]=0;
	strncat(amount, "/100:", sizeof(amount)-1);
        if (currency)
	  strncat(amount, currency, sizeof(amount)-1);
	amount[sizeof(amount)-1]=0;
	v=AB_Value_fromString(amount);
	if (v==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid amount in transaction");
	  AB_Transaction_free(t);
	  GWEN_Date_free(date);
	  GWEN_Buffer_free(lbuf);
	  return GWEN_ERROR_BAD_DATA;
	}
	else {
	  if (p[27]=='1')
	    /* FIXME: Do we have to negate on "1" or "2"? */
	    AB_Value_Negate(v);
	  AB_Transaction_SetValue(t, v);
          AB_Value_free(v);
	}

        /* copy local account info */
	s=AB_ImExporterAccountInfo_GetAccountNumber(iea);
        AB_Transaction_SetLocalAccountNumber(t, s);
	s=AB_ImExporterAccountInfo_GetBankCode(iea);
	AB_Transaction_SetLocalBankCode(t, s);
        break;
      }

      case 23: { /* transaction comments */
	GWEN_BUFFER *tbuf;

	if (size<80) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Record %02d too short (%d bytes)",
		    code, GWEN_Buffer_GetUsedBytes(lbuf));
	  AB_Transaction_free(t);
	  GWEN_Date_free(date);
	  GWEN_Buffer_free(lbuf);
	  return GWEN_ERROR_BAD_DATA;
	}

	if (t==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad order of records (23 before 22)");
	  AB_Transaction_free(t);
	  GWEN_Date_free(date);
	  GWEN_Buffer_free(lbuf);
	  return GWEN_ERROR_BAD_DATA;
	}

	tbuf=GWEN_Buffer_new(0, 256, 0, 1);
	/* comment 1 */
	GWEN_Buffer_AppendBytes(tbuf, p+4, 38);
	GWEN_Text_CondenseBuffer(tbuf);
        if (GWEN_Buffer_GetUsedBytes(tbuf))
	  AB_Transaction_AddPurpose(t, GWEN_Buffer_GetStart(tbuf), 0);
	GWEN_Buffer_Reset(tbuf);

	/* comment 2 */
	GWEN_Buffer_AppendBytes(tbuf, p+42, 38);
	GWEN_Text_CondenseBuffer(tbuf);
	if (GWEN_Buffer_GetUsedBytes(tbuf))
	  AB_Transaction_AddPurpose(t, GWEN_Buffer_GetStart(tbuf), 0);
        GWEN_Buffer_free(tbuf);
	break;
      }

      case 33: { /* end of accunt record */
	/* store current transaction if any */
	if (t) {
	  AB_ImExporterAccountInfo_AddTransaction(iea, t);
	  t=NULL;
	}

	// TODO: check the control fields here, read final account balance
        break;
      }

      case 88: {
	int numrecs;

	numrecs=AH_ImExporterQ43_ReadInt(p+20, 6);
	if (numrecs!=records) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Number of records doesn't match (%d != %d)",
		    numrecs, records);
	  AB_Transaction_free(t);
	  GWEN_Date_free(date);
	  GWEN_Buffer_free(lbuf);
	  return GWEN_ERROR_BAD_DATA;
	}
	break;
      }

      default:
        DBG_WARN(AQBANKING_LOGDOMAIN, "Ignoring line with code %02d", code);
      }

      GWEN_Buffer_Reset(lbuf);
      if (code!=0)
	records++;
    }
  } while (rv>=0);

  if (rv==GWEN_ERROR_EOF && hadSome)
    /* ignore EOF when we received some data */
    rv=0;

  if (t) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "There is still a transaction open...");
    AB_Transaction_free(t);
  }

  /* done */
  GWEN_Date_free(date);
  GWEN_Buffer_free(lbuf);

  return rv;
}



int AH_ImExporterQ43_Import(AB_IMEXPORTER *ie,
			    AB_IMEXPORTER_CONTEXT *ctx,
			    GWEN_SYNCIO *sio,
			    GWEN_DB_NODE *params){
  AH_IMEXPORTER_Q43 *ieh;
  GWEN_FAST_BUFFER *fb;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_Q43, ie);
  assert(ieh);

  fb=GWEN_FastBuffer_new(1024, sio);
  rv=AH_ImExporterQ43_ReadDocument(ie, ctx, fb, params);
  GWEN_FastBuffer_free(fb);

  return rv;
}



int AH_ImExporterQ43_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  AH_IMEXPORTER_Q43 *ieh;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_Q43, ie);
  assert(ieh);

  /* always return indifferent (for now) */
  return AB_ERROR_INDIFFERENT;
}



int AH_ImExporterQ43_Export(AB_IMEXPORTER *ie,
			    AB_IMEXPORTER_CONTEXT *ctx,
			    GWEN_SYNCIO *sio,
			    GWEN_DB_NODE *params){
  AH_IMEXPORTER_Q43 *ieh;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_Q43, ie);
  assert(ieh);


  // TODO

  return GWEN_ERROR_NOT_IMPLEMENTED;
}


