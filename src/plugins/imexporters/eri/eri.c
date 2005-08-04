/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Thu 21-07-2005
    copyright   : (C) 2005 by Peter de Vrijer
    email       : pdevrijer@home.nl

 ***************************************************************************
 *    Please see the file COPYING in this directory for license details    *
 ***************************************************************************/

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
#include <string.h>
#include <assert.h>

GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_ERI)


/* varstrcut cuts strings of length n and startig at position start from
   the source string and copies them to the destination string
   start is decreaseb by one, because column 1 is at position 0 in string */
void AB_ERI_varstrcut(char *dest, char *src, int start, int n) {
  int i;

  start--;

  /* go to start char */
  /*  for (i = 0; i < start; ++i) {
   *src++;
   } */

  src += start;  /* will this work on every machine and compiler? */

  /* copy wanted length of string and add '\0' */
  for (i = 0; i < n; ++i) {
    *dest++ = *src++;
  }
  *dest = 0;
  return;
}



/* stripPzero strips leading zeroes in accountNumber strings 
   and the P for Postgiro accounts.
   since there are records with remote accountnumbers of all spaces these will
   be stripped also! (Bank Costs mostly) */
void AB_ERI_stripPzero(char *dest, char *src) {

  while ((*src == 'P') || (*src == '0') || (*src == ' ')) { 
    src++;
  }

  /* if string was all zeroes, the result is an empty string */
  if (!*src) {
    *dest = 0;
    return;
  }

  /* copy the remaining string */
  while (*src) {
    *dest++ = *src++;
  }

  *dest = 0;
  return;
}



/* The strings are of fixed length. If info is shorter than the rest
   is filled with spaces. We want to get rid of those trailing spaces 
   and shorten the strings if possible */
void AB_ERI_stripTrailSpaces(char *buffer) {
  char *p;

  p = buffer;

  /* find trailing zero */
  while (*p) p++;

  /* check for empty strings */
  if (p == buffer) return;

  /* Go back one to last char of string */
  p--;

  /* Strip trailing spaces (beware of strings containing all spaces */
  while ((p >= buffer) && (*p == '\x20')) p--;

  /* Go forward one char and add trailing '\0' */
  *++p = 0;
}



int AB_ERI_ReadRecord(GWEN_BUFFEREDIO *bio,
		      char *buffer) {
  GWEN_ERRORCODE gwerr;
  int serr;
  unsigned int count, *cnt = &count;
  char c;

  /* check if there are no CR and or LF in the buffer */
  while (((c = GWEN_BufferedIO_PeekChar(bio)) == '\n') || (c == '\r')) 
                     c = GWEN_BufferedIO_ReadChar(bio);

  /* We did a peek for the next character, we can use that to detect EOF 
     in the first column of the next line */

#ifdef ERI_DEBUG
  printf("Char is 0x%X\n", c);
#endif

  if (c == GWEN_BUFFEREDIO_CHAR_EOF) {
    /* seems stupid to return ERROR_READ, but calling function uses that to
       detect End Of File at this point */
    return GWEN_ERROR_READ;
  }

  *cnt = REC_LENGTH;
  gwerr = GWEN_BufferedIO_ReadRawForced(bio, buffer, cnt);

  serr = GWEN_Error_GetSimpleCode(gwerr);

#ifdef ERI_DEBUG
  printf("Error Code is %d\n", serr);
#endif

  /* since there is no other way than EOF to see that the last record has been
   read, ERROR_READ may not be an error in the ERI file and should not be
   handled here ERROR_PARTIAL is a corrupt ERI file and is also handled by the
   calling function. Other errors are should not happen at all */
  if ((serr == GWEN_SUCCESS) ||
      (serr == GWEN_ERROR_READ) ||
      (serr == GWEN_ERROR_PARTIAL)) {
    return serr;
  }
  else {
    DBG_INFO_ERR(AQBANKING_LOGDOMAIN, gwerr);
    return GWEN_ERROR_GENERIC;
  }
}



int AB_ERI_parseFirstRecord(char *recbuf, ERI_TRANSACTION *current) {
  char varbuf[MAXVARLEN], s[MAXVARLEN];

  /* Sanity check, is this an ERI file?? */
  AB_ERI_varstrcut(varbuf, recbuf, 11, 17);
  if (strcmp(varbuf, "EUR99999999992000") != 0) {
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError, 
			  "ERI plugin: Error in syntax of first record!");
    return REC_BAD;
  }

  /* first local account number */
  AB_ERI_varstrcut(varbuf, recbuf, 1, 10);
  AB_ERI_stripPzero(s, varbuf);
  strcpy(current->localAccountNumber, s);

  /* remote account number 
     can be empty if bank itself is Payee */
  AB_ERI_varstrcut(varbuf, recbuf, 39, 10);
  AB_ERI_stripPzero(s, varbuf);
  strcpy(current->remoteAccountNumber, s);

  /* Name payee */
  AB_ERI_varstrcut(varbuf, recbuf, 49, 24);
  AB_ERI_stripTrailSpaces(varbuf);
  strcpy(current->namePayee, varbuf);

  /* Amount of transaction */
  AB_ERI_varstrcut(varbuf, recbuf, 74, 13);
  current->amount = strtod(varbuf, (char**)NULL)/100;

  /* Sign of transaction C is plus, D is minus */
  AB_ERI_varstrcut(varbuf, recbuf, 87, 1);
  if (*varbuf == 'D') {
    current->amount *= -1;
  }

  /* Transaction date next, Is simple string, No changes needed */
  AB_ERI_varstrcut(current->date, recbuf, 88, 6);

  /* Valuta date same */
  AB_ERI_varstrcut(current->valutaDate, recbuf, 94, 6);

  /* Transaction Id, only valid when BETALINGSKENM. is present 
     (see ERI format description) */
  AB_ERI_varstrcut(varbuf, recbuf, 109, 16);
  AB_ERI_stripTrailSpaces(varbuf);
  strcpy(current->transactionId, varbuf);

  return REC_OK;
}



int AB_ERI_parseSecondRecord(char *recbuf, ERI_TRANSACTION *current) {
  char varbuf[MAXVARLEN];

  /* Sanity check, is this record type 3? */
  AB_ERI_varstrcut(varbuf, recbuf, 11, 14);
  if (strcmp(varbuf, "EUR99999999993") != 0) {
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError, 
			  "ERI plugin: Error in syntax of second record!");
    return REC_BAD;
  }

  /* Check if theres is a transaction Id */
  AB_ERI_varstrcut(varbuf, recbuf, 25, 14);
  if (strcmp(varbuf, "BETALINGSKENM.") == 0) {
    current->transactionIdValid = TRUE;
  }

  /* Purpose line 1 */
  AB_ERI_varstrcut(varbuf, recbuf, 57, 32);
  AB_ERI_stripTrailSpaces(varbuf);
  strcpy(current->purpose1, varbuf);

  /* Purpose line 2 */
  AB_ERI_varstrcut(varbuf, recbuf, 89, 32);
  AB_ERI_stripTrailSpaces(varbuf);
  strcpy(current->purpose2, varbuf);

  return REC_OK;
}



int AB_ERI_parseThirdRecord(char *recbuf, ERI_TRANSACTION *current) {
  char varbuf[MAXVARLEN];

  /* Sanity check, is this record type 4? */
  AB_ERI_varstrcut(varbuf, recbuf, 11, 14);
  if (strcmp(varbuf, "EUR99999999994") != 0) {
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError, 
			  "ERI plugin: Error in syntax of third record!");
    return REC_BAD;
  }

  /* Purpose line 3 */
  AB_ERI_varstrcut(varbuf, recbuf, 25, 32);
  AB_ERI_stripTrailSpaces(varbuf);
  strcpy(current->purpose3, varbuf);

  /* Purpose line 4 */
  AB_ERI_varstrcut(varbuf, recbuf, 57, 32);
  AB_ERI_stripTrailSpaces(varbuf);
  strcpy(current->purpose4, varbuf);

  /* Purpose line 5 */
  AB_ERI_varstrcut(varbuf, recbuf, 89, 32);
  AB_ERI_stripTrailSpaces(varbuf);
  strcpy(current->purpose5, varbuf);

  /* Check if theres is a transaction Id */
  AB_ERI_varstrcut(varbuf, recbuf, 25, 14);
  if (strcmp(varbuf, "BETALINGSKENM.") == 0) {
    current->transactionIdValid = TRUE;
    /* If so kill purpose3 field, it contains no real info */
    *current->purpose3 = 0;
  }

  return REC_OK;
}



int AB_ERI_parseFourthRecord(char *recbuf, ERI_TRANSACTION *current) {
  char varbuf[MAXVARLEN];

  /* Sanity check, is this record type 4? */
  AB_ERI_varstrcut(varbuf, recbuf, 11, 14);
  if (strcmp(varbuf, "EUR99999999994") != 0) {
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError, 
			  "ERI plugin: Error in syntax of fourth record!");
    return REC_BAD;
  }

  /* Purpose line 6 */
  AB_ERI_varstrcut(varbuf, recbuf, 25, 96);
  AB_ERI_stripTrailSpaces(varbuf);
  strcpy(current->purpose6, varbuf);

  /* Check if theres is a transaction Id */
  AB_ERI_varstrcut(varbuf, recbuf, 25, 14);
  if (strcmp(varbuf, "BETALINGSKENM.") == 0) {
    current->transactionIdValid = TRUE;
    /* If so kill purpose 6 field, it contains no real info */
    *current->purpose6 = 0;
  }

  return REC_OK;
}

void AB_ERI_AddPurpose(AB_TRANSACTION *t, char *purpose) {

  if (strlen(purpose) > 0) {
    AB_Transaction_AddPurpose(t, purpose, 0);
  }
}

int AB_ERI_AddTransaction(AB_IMEXPORTER_CONTEXT *ctx,
                          ERI_TRANSACTION *current,
                          GWEN_DB_NODE *params) {
  AB_IMEXPORTER_ACCOUNTINFO *iea = 0;
  AB_TRANSACTION *t = 0;
  AB_VALUE *vAmount = 0;
  GWEN_TIME *ti = 0;
  char *defaultTime = "12000020", dateTime[15];
  const char *bankName;
  const char *dateFormat;
  const char *currency;

  bankName=GWEN_DB_GetCharValue(params, "bankName", 0, "Rabobank");
  dateFormat=GWEN_DB_GetCharValue(params, "dateFormat", 0, "hhmmssYYYYMMDD");
  currency=GWEN_DB_GetCharValue(params, "currency", 0, "EUR");

  /* Search if account number is already in context
     If so add transaction there, else make new account number in context. */
  iea = AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while(iea) {
    if (strcmp(AB_ImExporterAccountInfo_GetAccountNumber(iea),
               current->localAccountNumber) == 0)
      break;
    iea = AB_ImExporterContext_GetNextAccountInfo(ctx);
  }

  if (!iea) {
    /* Not found, add it */
    iea = AB_ImExporterAccountInfo_new();
    AB_ImExporterContext_AddAccountInfo(ctx, iea);
    AB_ImExporterAccountInfo_SetType(iea, AB_AccountType_Bank);
    AB_ImExporterAccountInfo_SetBankName(iea, bankName);
    AB_ImExporterAccountInfo_SetAccountNumber(iea,
                                              current->localAccountNumber);
  }

  /* Now create AB Transaction and start filling it with what we know */
  t = AB_Transaction_new();

  /* remoteAccountNumber */
  AB_Transaction_SetRemoteAccountNumber(t, current->remoteAccountNumber);

  /* namePayee */
  AB_Transaction_AddRemoteName(t, current->namePayee, 0);

  /* amount */
  vAmount = AB_Value_new(current->amount, currency);
  AB_Transaction_SetValue(t, vAmount);
  AB_Value_free(vAmount);

  /* date
     Transaction time, we take noon */
  strcpy(dateTime, defaultTime);
  strcat(dateTime, current->date);

  ti = GWEN_Time_fromString(dateTime, dateFormat);
  AB_Transaction_SetDate(t, ti);
  GWEN_Time_free(ti);

  /* Same for valuta date */
  strcpy(dateTime, defaultTime);
  strcat(dateTime, current->valutaDate);
  ti = GWEN_Time_fromString(dateTime, dateFormat);
  AB_Transaction_SetValutaDate(t, ti);
  GWEN_Time_free(ti);

  /* transactionId if there */
  if (current->transactionIdValid) {
    AB_Transaction_SetCustomerReference(t, current->transactionId);
  }

  /* Now add all the purpose descriptions if there */
  AB_ERI_AddPurpose(t, current->purpose1);
  AB_ERI_AddPurpose(t, current->purpose2);
  AB_ERI_AddPurpose(t, current->purpose3);
  AB_ERI_AddPurpose(t, current->purpose4);
  AB_ERI_AddPurpose(t, current->purpose5);
  AB_ERI_AddPurpose(t, current->purpose6);

  /* Add it to the AccountInfo List */
  AB_ImExporterAccountInfo_AddTransaction(iea, t);

  return TRANS_OK;
}



int AB_ERI_parseTransaction(AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_BUFFEREDIO *bio,
                            GWEN_DB_NODE *params) {

  ERI_TRANSACTION trans, *current = &trans;
  int rerr, terr, aerr, translen = 0;
  char recbuf[REC_LENGTH+1];
  recbuf[REC_LENGTH] = 0;
  current->transactionIdValid = FALSE;

  GWEN_BufferedIO_SetReadBuffer(bio, 0, REC_LENGTH);

  /* Read the first record of the transaction */
  rerr = AB_ERI_ReadRecord(bio, recbuf);

  if (rerr == GWEN_ERROR_READ) {
    /* When Error on Read occurs here, buffer was empty, normal EOF */
    return TRANS_EOF;
  }
  else if (rerr == GWEN_ERROR_PARTIAL) {
    /* With Error met EOF, EOF occured in middle of record */
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                          "ERI plugin: Short first record in Transaction!");
    return TRANS_BAD;
  }
  else if (rerr == GWEN_ERROR_GENERIC) {
    /* This error something unexpected went wrong and nothing can be
     trusted */
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                          "ERI plugin: UNKNOWN ERROR, while importing "
                          "ERI file");
    return TRANS_BAD;
  }

  /* Get the info from the first record of the transaction and place them in
   the struct */
  terr = AB_ERI_parseFirstRecord(recbuf, current);

  if (terr == REC_BAD) {
    return TRANS_BAD;
  }

#ifdef ERI_DEBUG
  printf("l %s, r %s, n %s, a %f, d %s, v %s, i %s.\n",
         current->localAccountNumber,
	 current->remoteAccountNumber, current->namePayee, current->amount,
	 current->date, current->valutaDate, current->transactionId);
#endif

  /* Read the second record into recbuf */
  rerr = AB_ERI_ReadRecord(bio, recbuf);

  /* End of File should not happen here! */
  if ((rerr == GWEN_ERROR_READ) || (rerr == GWEN_ERROR_PARTIAL)) {
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError, 
                          "ERI plugin: Transaction not complete, short "
                          "second record!");
    return TRANS_BAD;
  }
  else if (rerr == GWEN_ERROR_GENERIC) {
    /* This error something unexpected went wrong and nothing can be
     trusted */
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                          "ERI plugin: UNKNOWN ERROR, while importing "
                          "ERI file");
    return TRANS_BAD;
  }

  /* check how many records in transaction */
  switch (recbuf[121-1]) {
  case '0':
    translen = LINES2;
    break;
  case '1':
    translen = LINES3;
    break;
  case '2':
    translen = LINES4;
    break;
  }

  /* get the info from the second record and place them in transaction
   struct */
  terr = AB_ERI_parseSecondRecord(recbuf, current);

  if (terr == REC_BAD) {
    return TRANS_BAD;
  }

#ifdef ERI_DEBUG
  printf("p1 %s, p2 %s.\n", current->purpose1, current->purpose2);
#endif

  /* Clear all purpose strings of line 3 and 4. 
     They may contain rubbish when lines are not there */
  *current->purpose3 = 0;
  *current->purpose4 = 0;
  *current->purpose5 = 0;
  *current->purpose6 = 0;

  /* If 1 or 2 type 4 records (3 lines or 4 lines transaction), read and parse
   them */
  if (translen >= LINES3) {
    /* Read third record in recbuf */
    rerr = AB_ERI_ReadRecord(bio, recbuf);

    /* End of File should not happen here! */
    if ((rerr == GWEN_ERROR_READ) || (rerr == GWEN_ERROR_PARTIAL)) {
      GWEN_WaitCallback_Log(GWEN_LoggerLevelError, 
                            "ERI plugin: Transaction not complete, short "
                            "third record!");
      return TRANS_BAD;
    } else if (rerr == GWEN_ERROR_GENERIC) {
      /* This error something unexpected went wrong and nothing can be
       trusted */
      GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                            "ERI plugin: UNKNOWN ERROR, while importing "
                            "ERI file");
      return TRANS_BAD;
    }

    /* get the info from the third record and place them in transaction struct
     */
    terr = AB_ERI_parseThirdRecord(recbuf, current);

    if (terr == REC_BAD) {
      return TRANS_BAD;
    }

#ifdef ERI_DEBUG
    printf("p3 %s, p4 %s, p5 %s.\n", current->purpose3, current->purpose4, 
	   current->purpose5);
#endif

    /* If 4 line is present in transaction, read and parse it */
    if (translen == LINES4) {
      /* Read fourth record in buffer */
      rerr = AB_ERI_ReadRecord(bio, recbuf);

      /* End of File should not happen here! */
      if ((rerr == GWEN_ERROR_READ) || (rerr == GWEN_ERROR_PARTIAL)) {
        GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                              "ERI plugin: Transaction not complete, short "
                              "fourth record!");
        return TRANS_BAD;
      } else if (rerr == GWEN_ERROR_GENERIC) {
        /* This error something unexpected went wrong and nothing can be
         trusted */
        GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                              "ERI plugin: UNKNOWN ERROR, while importing "
                              "ERI file");
        return TRANS_BAD;
      }

      /* get the info from the fourth record and place them in transaction
       struct */
      terr = AB_ERI_parseFourthRecord(recbuf, current);

      if (terr == REC_BAD) {
        return TRANS_BAD;
      }

#ifdef ERI_DEBUG
      printf("p6 %s.\n", current->purpose6);
#endif

    }

  }

#ifdef ERI_DEBUG
  if (current->transactionIdValid) {
    printf("t %s.\n", current->transactionId);
  }
#endif

  aerr = AB_ERI_AddTransaction(ctx, current, params);

  return TRANS_OK;
}



AB_IMEXPORTER *eri_factory(AB_BANKING *ab, GWEN_DB_NODE *db) {
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_ERI *ieh;

  ie = AB_ImExporter_new(ab, "eri");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_ERI, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_ERI, ie, ieh,
		       AH_ImExporterERI_FreeData);

  ieh->dbData = db;

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
  char strbuf[128];
  int err, transcount = 0;

  GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                        "ERI plugin: Importing started.");

  /* check buffered IO is in place */
  assert(bio);

  /* Now start reading and parsing transactions until EOF or error */
  while (!(err = AB_ERI_parseTransaction(ctx, bio, params))) 
    transcount++;

  if (err == TRANS_EOF) {  /* EOF everything Ok */
    sprintf(strbuf, "ERI plugin: File imported Ok, %d transactions read.",
            transcount);
    GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice, strbuf);
    return GWEN_SUCCESS;
  }

  sprintf(strbuf,
          "ERI plugin: File NOT imported Ok! Error in transaction %d.",
          transcount+1);
  GWEN_WaitCallback_Log(GWEN_LoggerLevelError, strbuf);

  if (err == TRANS_BAD)
    /* Something is wrong with the transactions */
    return AB_ERROR_BAD_DATA;

  return AB_ERROR_UNKNOWN;

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


