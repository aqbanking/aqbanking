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

#ifndef AQHBCI_IMEX_ERI_P_H
#define AQHBCI_IMEX_ERI_P_H


#include <gwenhywfar/dbio.h>
#include <aqbanking/imexporter_be.h>


typedef struct AH_IMEXPORTER_ERI AH_IMEXPORTER_ERI;
struct AH_IMEXPORTER_ERI {
  GWEN_DB_NODE *dbData;
  GWEN_DBIO *dbio;
};

typedef struct ERI_TRANSACTION ERI_TRANSACTION;
struct ERI_TRANSACTION {
  char localAccountNumber[11];
  char remoteAccountNumber[11];
  char namePayee[25];
  double amount;
  char date[7];
  char valutaDate[7];
  char transactionId[17];
  int transactionIdValid;
  char purpose1[33];
  char purpose2[33];
  char purpose3[33];
  char purpose4[33];
  char purpose5[33];
  char purpose6[97];
};

AB_IMEXPORTER* eri_factory(AB_BANKING *ab, GWEN_DB_NODE *db);
void AH_ImExporterERI_FreeData(void *bp, void *p);

int AH_ImExporterERI_Import(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_BUFFEREDIO *bio,
                            GWEN_DB_NODE *params);
int AH_ImExporterERI_Export(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_BUFFEREDIO *bio,
                            GWEN_DB_NODE *params);

int AH_ImExporterERI_CheckFile(AB_IMEXPORTER *ie, const char *fname);


int AH_ImExporterERI__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                      GWEN_DB_NODE *db,
                                      GWEN_DB_NODE *dbParams);


#define CHECKBUF_LENGTH 128
#define REC_LENGTH 128
#define MAXVARLEN 97
#define LINES2 0
#define LINES3 1
#define LINES4 2
#define FALSE 0
#define TRUE 1

/* my own errorcodes for transactions reading */
#define TRANS_OK 0
#define REC_OK 0
#define TRANS_BAD -1
#define REC_BAD -1
#define TRANS_EOF 1

/* for debugging purposes */
/* #define ERI_DEBUG */

#endif /* AQHBCI_IMEX_ERI_P_H */
