/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: openhbci1_p.h 168 2004-11-26 21:01:00Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_IMEX_YN_P_H
#define AQHBCI_IMEX_YN_P_H


#include <gwenhywfar/dbio.h>
#include <aqbanking/imexporter_be.h>


typedef struct AB_IMEXPORTER_YELLOWNET AB_IMEXPORTER_YELLOWNET;
struct AB_IMEXPORTER_YELLOWNET {
  GWEN_DB_NODE *dbData;
};


AB_IMEXPORTER* yellownet_factory(AB_BANKING *ab, GWEN_DB_NODE *db);
void GWENHYWFAR_CB AB_ImExporterYN_FreeData(void *bp, void *p);

int AB_ImExporterYN_Import(AB_IMEXPORTER *ie,
                           AB_IMEXPORTER_CONTEXT *ctx,
                           GWEN_BUFFEREDIO *bio,
                           GWEN_DB_NODE *params);

int AB_ImExporterYN_CheckFile(AB_IMEXPORTER *ie, const char *fname);


AB_VALUE *AB_ImExporterYN__ReadValue(AB_IMEXPORTER *ie,
                                     GWEN_XMLNODE *node,
                                     int value);

GWEN_TIME *AB_ImExporterYN__ReadTime(AB_IMEXPORTER *ie,
                                     GWEN_XMLNODE *node,
                                     int value);

AB_IMEXPORTER_ACCOUNTINFO*
  AB_ImExporterYN__ReadAccountInfo(AB_IMEXPORTER *ie,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   GWEN_XMLNODE *doc);


AB_TRANSACTION *AB_ImExporterYN__ReadLNE_LNS(AB_IMEXPORTER *ie,
                                             AB_IMEXPORTER_ACCOUNTINFO *ai,
                                             GWEN_XMLNODE *node);

int AB_ImExporterYN__ReadTransactions(AB_IMEXPORTER *ie,
                                      AB_IMEXPORTER_ACCOUNTINFO *ai,
                                      GWEN_XMLNODE *doc);
int AB_ImExporterYN__ReadAccountStatus(AB_IMEXPORTER *ie,
                                       AB_IMEXPORTER_ACCOUNTINFO *ai,
                                       GWEN_XMLNODE *doc);


#endif /* AQHBCI_IMEX_YN_P_H */
