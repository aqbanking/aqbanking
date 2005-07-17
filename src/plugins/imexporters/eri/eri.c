#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eri_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/waitcallback.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_ERI);


AB_IMEXPORTER *eri_factory(AB_BANKING *ab, GWEN_DB_NODE *db) {
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_ERI *ieh;

  ie = AB_ImExporter_new(ab, "eri");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_ERI, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_ERI, ie, ieh,
		       AH_ImExporterERI_FreeData);

  ieh->dbData = db;
  /*  ieh->dbio = GWEN_DBIO_GetPlugin("eri");

  if (!ieh->dbio) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "GWEN DBIO plugin \"ERI\" not available");

    AB_ImExporter_free(ie);
    return 0;
    } */

  AB_ImExporter_SetImportFn(ie, AH_ImExporterERI_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterERI_Export);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterERI_CheckFile);

  return ie;
}



void AH_ImExporterERI_FreeData(void *bp, void *p) {
  AH_IMEXPORTER_ERI *ieh;

  ieh = (AH_IMEXPORTER_ERI*) p;
  GWEN_FREE_OBJECT(ieh);
}

int AH_ImExporterERI_Import(AB_IMEXPORTER *ie,
			    AB_IMEXPORTER_CONTEXT *ctx,
			    GWEN_BUFFEREDIO *bio,
			    GWEN_DB_NODE *params) {
  AH_IMEXPORTER_ERI *ieeri;
  AB_IMEXPORTER_ACCOUNTINFO *iea = 0;
  AB_TRANSACTION *t = 0;
  AB_VALUE *vAmount = 0;
  GWEN_TIME *ti = 0;

  assert(ie);
  ieeri = GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_ERI, ie);
  assert(ieeri);

  iea = AB_ImExporterAccountInfo_new();
  AB_ImExporterContext_AddAccountInfo(ctx, iea);

  AB_ImExporterAccountInfo_SetAccountName(iea, "P.J. de Vrijer");
  AB_ImExporterAccountInfo_SetDescription(iea, "Betaalrekening");
  AB_ImExporterAccountInfo_SetType(iea, AB_AccountType_Bank);
  AB_ImExporterAccountInfo_SetBankName(iea, "Rabobank");
  AB_ImExporterAccountInfo_SetAccountNumber(iea, "3066900");

  vAmount = AB_Value_new(12.34, "EUR");

  t = AB_Transaction_new();
  AB_Transaction_SetValue(t, vAmount);
  AB_Transaction_AddRemoteName(t, "AH vd Capellenstraat, Zwolle", 0);
  AB_Transaction_AddPurpose(t, "Testboeking 1", 0);

  ti = GWEN_Time_fromString("13062220050717", "hhmmssYYYYMMDD");

  AB_Transaction_SetValutaDate(t, ti);
  AB_Transaction_SetDate(t,ti);

  AB_ImExporterAccountInfo_AddTransaction(iea, t);

  return 0;
}

int AH_ImExporterERI_Export(AB_IMEXPORTER *ie,
			    AB_IMEXPORTER_CONTEXT *ctx,
			    GWEN_BUFFEREDIO *bio,
			    GWEN_DB_NODE *params) {
  return AB_ERROR_NOT_SUPPORTED;
}

int AH_ImExporterERI_CheckFile(AB_IMEXPORTER *ie, const char *fname) {
  int fd;
  char lbuffer[CHECKBUF_LENGTH];
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;

  assert(ie);
  assert(fname);

  fd = open(fname, O_RDONLY);
  if (fd == -1) {
    /* error */
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "open(%s): %s", fname, strerror(errno));
    return AB_ERROR_NOT_FOUND;
  }

  bio = GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, CHECKBUF_LENGTH);

  err = GWEN_BufferedIO_ReadLine(bio, lbuffer, CHECKBUF_LENGTH);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "File \"%s\" is not supported by this plugin",
	     fname);
    GWEN_BufferedIO_Close(bio);
    GWEN_BufferedIO_free(bio);
    return AB_ERROR_BAD_DATA;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN,
	   "Line read in buffer is: %s", 
	   lbuffer);

  if ( -1 != GWEN_Text_ComparePattern(lbuffer, "*EUR99999999992000*", 0)) {
    /* match */
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "File \"%s\" is supported by this plugin",
	     fname);
    GWEN_BufferedIO_Close(bio);
    GWEN_BufferedIO_free(bio);
    return 0;
  }

  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  return AB_ERROR_BAD_DATA;
}
