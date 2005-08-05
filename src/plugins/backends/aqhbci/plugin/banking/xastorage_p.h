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


#ifndef AQHBCIBANK_XASTORAGE_P_H
#define AQHBCIBANK_XASTORAGE_P_H

#include <aqhbci/xastorage.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/idlist.h>


typedef struct AH_STORAGE_DAY AH_STORAGE_DAY;

GWEN_LIST_FUNCTION_DEFS(AH_STORAGE_DAY, AH_Storage_Day);

struct AH_STORAGE_DAY {
  GWEN_LIST_ELEMENT(AH_STORAGE_DAY);
  unsigned int year;
  unsigned int month;
  unsigned int day;
  GWEN_TYPE_UINT32 id;
  char *file;
  int readOnly;
  int changed;
  GWEN_DB_NODE *transactions;
  GWEN_DB_NODE *current;
  GWEN_TYPE_UINT32 usage;
};


AH_STORAGE_DAY *AH_Storage_Day_new(unsigned int year,
				     unsigned int month,
				     unsigned int day,
				     const char *file,
				     int readOnly,
                                     GWEN_DB_NODE *transactions);
void AH_Storage_Day_free(AH_STORAGE_DAY *dy);

unsigned int AH_Storage_Day_GetYear(const AH_STORAGE_DAY *dy);
unsigned int AH_Storage_Day_GetMonth(const AH_STORAGE_DAY *dy);
unsigned int AH_Storage_Day_GetDay(const AH_STORAGE_DAY *dy);
GWEN_TYPE_UINT32 AH_Storage_Day_GetId(const AH_STORAGE_DAY *dy);
const char *AH_Storage_Day_GetFileName(const AH_STORAGE_DAY *dy);
int AH_Storage_Day_GetReadOnly(const AH_STORAGE_DAY *dy);
int AH_Storage_Day_GetChanged(const AH_STORAGE_DAY *dy);
void AH_Storage_Day_SetChanged(AH_STORAGE_DAY *dy, int chg);

GWEN_DB_NODE *AH_Storage_Day_GetTransactions(AH_STORAGE_DAY *dy);
GWEN_DB_NODE *AH_Storage_Day_GetFirstTransaction(AH_STORAGE_DAY *dy);
GWEN_DB_NODE *AH_Storage_Day_GetNextTransaction(AH_STORAGE_DAY *dy);



struct AH_STORAGE {
  AH_STORAGE_DAY_LIST *days;
  char *path;

  GWEN_IDLIST *availDays;
};


GWEN_TYPE_UINT32 AH_Storage__MakeId(unsigned int year,
				     unsigned int month,
				     unsigned int day);

AH_STORAGE_DAY *AH_Storage_FindDay(AH_STORAGE *st, GWEN_TYPE_UINT32 id);


AH_STORAGE_DAY *AH_Storage_GetDay(AH_STORAGE *st,
				    unsigned int year,
				    unsigned int month,
				    unsigned int day,
				    int ro);
int AH_Storage_SaveDay(AH_STORAGE *st, AH_STORAGE_DAY *dy);


int AH_Storage_BuildIndex(AH_STORAGE *st);



#endif /* AQHBCIBANK_XASTORAGE_P_H */






