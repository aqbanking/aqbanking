/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Apr 05 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCIBANK_XASTORAGE_H
#define AQHBCIBANK_XASTORAGE_H

#include <gwenhywfar/types.h>
#include <gwenhywfar/db.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AH_STORAGE AH_STORAGE;


AH_STORAGE *AH_Storage_new(const char *path);
void AH_Storage_free(AH_STORAGE *st);

GWEN_TYPE_UINT32 AH_Storage_OpenDay(AH_STORAGE *st,
                                     unsigned int year,
                                     unsigned int month,
                                     unsigned int day,
                                     int ro);
int AH_Storage_CloseDay(AH_STORAGE *st, GWEN_TYPE_UINT32 hdl);
void AH_Storage_AbandonDay(AH_STORAGE *st, GWEN_TYPE_UINT32 hdl);
int AH_Storage_ClearDay(AH_STORAGE *st, GWEN_TYPE_UINT32 hdl);

GWEN_DB_NODE *AH_Storage_GetFirstTransaction(AH_STORAGE *st,
                                              GWEN_TYPE_UINT32 hdl);
GWEN_DB_NODE *AH_Storage_GetNextTransaction(AH_STORAGE *st,
                                             GWEN_TYPE_UINT32 hdl);

int AH_Storage_GetFirstDay(AH_STORAGE *st,
                            unsigned int *year,
                            unsigned int *month,
                            unsigned int *day);

int AH_Storage_GetNextDay(AH_STORAGE *st,
                           unsigned int *year,
                           unsigned int *month,
                           unsigned int *day);

#ifdef __cplusplus
}
#endif

#endif /* AQHBCIBANK_XASTORAGE_H */


