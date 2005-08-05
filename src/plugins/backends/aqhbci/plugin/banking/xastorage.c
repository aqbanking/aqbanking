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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "xastorage_p.h"
#include "aqhbci_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/directory.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_LIST_FUNCTIONS(AH_STORAGE_DAY, AH_Storage_Day);



GWEN_TYPE_UINT32 AH_Storage__MakeId(unsigned int year,
                                     unsigned int month,
                                     unsigned int day){
  GWEN_TYPE_UINT32 id;

  id=((year&0xffff)<<16) + ((month&0xff)<<8) + (day&0xff);
  return id;
}



AH_STORAGE_DAY *AH_Storage_Day_new(unsigned int year,
                                     unsigned int month,
                                     unsigned int day,
                                     const char *file,
                                     int readOnly,
                                     GWEN_DB_NODE *transactions){
  AH_STORAGE_DAY *dy;

  GWEN_NEW_OBJECT(AH_STORAGE_DAY, dy);
  GWEN_LIST_INIT(AH_STORAGE_DAY, dy);
  dy->year=year;
  dy->month=month;
  dy->day=day;
  dy->file=strdup(file);
  dy->readOnly=readOnly;
  dy->transactions=transactions;
  dy->id=AH_Storage__MakeId(year, month, day);
  dy->usage=1;
  return dy;
}



void AH_Storage_Day_free(AH_STORAGE_DAY *dy){
  if (dy) {
    assert(dy->usage);
    if (--(dy->usage)==0) {
      GWEN_LIST_FINI(AH_STORAGE_DAY, dy);
      free(dy->file);
      GWEN_DB_Group_free(dy->transactions);
      GWEN_FREE_OBJECT(dy);
    }
  }
}



void AH_Storage_Day_Attach(AH_STORAGE_DAY *dy){
  assert(dy);
  dy->usage++;
}



unsigned int AH_Storage_Day_GetYear(const AH_STORAGE_DAY *dy){
  assert(dy);
  return dy->year;
}



unsigned int AH_Storage_Day_GetMonth(const AH_STORAGE_DAY *dy){
  assert(dy);
  return dy->month;
}



unsigned int AH_Storage_Day_GetDay(const AH_STORAGE_DAY *dy){
  assert(dy);
  return dy->day;
}



GWEN_TYPE_UINT32 AH_Storage_Day_GetId(const AH_STORAGE_DAY *dy){
  assert(dy);
  return dy->id;
}



const char *AH_Storage_Day_GetFileName(const AH_STORAGE_DAY *dy){
  assert(dy);
  return dy->file;
}



int AH_Storage_Day_GetReadOnly(const AH_STORAGE_DAY *dy){
  assert(dy);
  return dy->readOnly;
}



int AH_Storage_Day_GetChanged(const AH_STORAGE_DAY *dy){
  assert(dy);
  if (dy->changed ||
      (GWEN_DB_GetNodeFlags(dy->transactions) & GWEN_DB_NODE_FLAGS_DIRTY))
    return 1;
  return 0;
}



void AH_Storage_Day_SetChanged(AH_STORAGE_DAY *dy, int chg){
  assert(dy);
  dy->changed=chg;
  GWEN_DB_ModifyBranchFlagsDown(dy->transactions,
				0,
				GWEN_DB_NODE_FLAGS_DIRTY);
}



GWEN_DB_NODE *AH_Storage_Day_GetTransactions(AH_STORAGE_DAY *dy){
  assert(dy);
  return dy->transactions;
}



GWEN_DB_NODE *AH_Storage_Day_GetFirstTransaction(AH_STORAGE_DAY *dy){
  GWEN_DB_NODE *t;

  assert(dy);
  assert(dy->transactions);
  t=GWEN_DB_FindFirstGroup(dy->transactions, "transaction");
  dy->current=t;
  return t;
}



GWEN_DB_NODE *AH_Storage_Day_GetNextTransaction(AH_STORAGE_DAY *dy){
  GWEN_DB_NODE *t;

  assert(dy);
  assert(dy->transactions);
  if (!dy->current)
    return 0;
  t=GWEN_DB_FindNextGroup(dy->current, "transaction");
  dy->current=t;
  return t;
}










AH_STORAGE_DAY *AH_Storage_FindDay(AH_STORAGE *st, GWEN_TYPE_UINT32 id) {
  AH_STORAGE_DAY *dy;

  assert(st);

  dy=AH_Storage_Day_List_First(st->days);
  while(dy) {
    if (AH_Storage_Day_GetId(dy)==id)
      break;
    dy=AH_Storage_Day_List_Next(dy);
  } /* while */

  return dy;
}



AH_STORAGE_DAY *AH_Storage_GetDay(AH_STORAGE *st,
                                    unsigned int year,
                                    unsigned int month,
                                    unsigned int day,
                                    int ro) {
  GWEN_BUFFER *pbuf;
  char numbuf[64];
  GWEN_DB_NODE *n;
  AH_STORAGE_DAY *dy;

  pbuf=GWEN_Buffer_new(0, 128, 0, 1);
  GWEN_Buffer_AppendString(pbuf, st->path);
  snprintf(numbuf, sizeof(numbuf)-1, "/%04d/%02d/%02d", year, month, day);
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(pbuf, numbuf);
  GWEN_Buffer_AppendString(pbuf, ".trans");
  n=GWEN_DB_Group_new("transactions");
  if (GWEN_DB_ReadFile(n, GWEN_Buffer_GetStart(pbuf), GWEN_DB_FLAGS_DEFAULT)){
    if (!ro) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not load file \"%s\"", GWEN_Buffer_GetStart(pbuf));
      GWEN_DB_Group_free(n);
      GWEN_Buffer_free(pbuf);
      return 0;
    }
  }
  dy=AH_Storage_Day_new(year, month, day,
                         GWEN_Buffer_GetStart(pbuf), ro,
                         n);
  GWEN_Buffer_free(pbuf);
  return dy;
}



int AH_Storage_SaveDay(AH_STORAGE *st, AH_STORAGE_DAY *dy) {
  if (GWEN_Directory_GetPath(AH_Storage_Day_GetFileName(dy),
                             GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Path \"%s\" unavailable",
              AH_Storage_Day_GetFileName(dy));
    return -1;
  }

  if (GWEN_DB_WriteFile(AH_Storage_Day_GetTransactions(dy),
                        AH_Storage_Day_GetFileName(dy),
                        GWEN_DB_FLAGS_DEFAULT)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not save day \"%s\"",
              AH_Storage_Day_GetFileName(dy));
    return -1;
  }

  return 0;
}










AH_STORAGE *AH_Storage_new(const char *path){
  AH_STORAGE *st;

  assert(path);
  GWEN_NEW_OBJECT(AH_STORAGE, st);
  st->path=strdup(path);
  st->days=AH_Storage_Day_List_new();

  return st;
}



void AH_Storage_free(AH_STORAGE *st){
  if (st) {
    AH_Storage_Day_List_free(st->days);
    GWEN_IdList_free(st->availDays);
    free(st->path);

    GWEN_FREE_OBJECT(st);
  }
}



int AH_Storage_BuildIndex(AH_STORAGE *st) {
  GWEN_DIRECTORYDATA *dyears;
  GWEN_IDLIST *idl;
  GWEN_BUFFER *nbuf;
  unsigned int ypos;

  idl=GWEN_IdList_new();
  dyears=GWEN_Directory_new();
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  GWEN_Buffer_AppendString(nbuf, st->path);
  ypos=GWEN_Buffer_GetPos(nbuf);
  if (!GWEN_Directory_Open(dyears, st->path)) {
    char nbuffer[256];

    // check for years
    while(!GWEN_Directory_Read(dyears,
                               nbuffer,
                               sizeof(nbuffer))) {
      unsigned int year;

      if (1==sscanf(nbuffer, "%d", &year)) {
        GWEN_DIRECTORYDATA *dmonths;
        unsigned int mpos;

        GWEN_Buffer_Crop(nbuf, 0, ypos);
        GWEN_Buffer_SetPos(nbuf, ypos);
        GWEN_Buffer_AppendByte(nbuf, '/');
        GWEN_Buffer_AppendString(nbuf, nbuffer);
        mpos=GWEN_Buffer_GetPos(nbuf);

        // found a year, check it further
        dmonths=GWEN_Directory_new();
        if (!GWEN_Directory_Open(dmonths, GWEN_Buffer_GetStart(nbuf))) {

          // check for months
          while(!GWEN_Directory_Read(dmonths,
                                     nbuffer,
                                     sizeof(nbuffer))) {
            unsigned int month;

            if (1==sscanf(nbuffer, "%d", &month)) {
              GWEN_DIRECTORYDATA *ddays;

              // found a month, check it further
              GWEN_Buffer_Crop(nbuf, 0, mpos);
              GWEN_Buffer_SetPos(nbuf, mpos);
              GWEN_Buffer_AppendByte(nbuf, '/');
              GWEN_Buffer_AppendString(nbuf, nbuffer);

              ddays=GWEN_Directory_new();
              if (!GWEN_Directory_Open(ddays, GWEN_Buffer_GetStart(nbuf))) {
                while(!GWEN_Directory_Read(ddays,
                                           nbuffer,
                                           sizeof(nbuffer))) {
                  int i;

                  i=strlen(nbuffer);
                  if (i>6) {
                    if (strcmp(nbuffer+i-6, ".trans")==0) {
                      unsigned int day;

                      nbuffer[i-6]=0;
                      if (1==sscanf(nbuffer, "%d", &day)) {
                        GWEN_TYPE_UINT32 id;

                        // found a day, save it
                        id=AH_Storage__MakeId(year, month, day);
                        DBG_INFO(AQHBCI_LOGDOMAIN, "Found day %d/%d/%d (%08x)",
                                 year, month, day, id);
                        GWEN_IdList_AddId(idl, id);
                      } // if entry is a day
                    } // if filename ends with ".trans"
                  } // if filename is long enough
                } // while still days
                if (GWEN_Directory_Close(ddays)) {
                  DBG_ERROR(AQHBCI_LOGDOMAIN, "Error closing day");
                  GWEN_IdList_free(idl);
                  GWEN_Buffer_free(nbuf);
                  return -1;
                }
              } // if open
              GWEN_Directory_free(ddays);
            } // if entry is a month
          } // while still months
          if (GWEN_Directory_Close(dmonths)) {
            DBG_ERROR(AQHBCI_LOGDOMAIN, "Error closing month");
            GWEN_IdList_free(idl);
            GWEN_Buffer_free(nbuf);
            return -1;
          }
        } // if monthpath ok
        GWEN_Directory_free(dmonths);
      } // if entry is a year
    } /* while years */
    if (GWEN_Directory_Close(dyears)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error closing year");
      GWEN_IdList_free(idl);
      GWEN_Buffer_free(nbuf);
      return -1;
    }
  } // if open is ok
  GWEN_Directory_free(dyears);
  GWEN_Buffer_free(nbuf);

  GWEN_IdList_Sort(idl);
  GWEN_IdList_free(st->availDays);
  st->availDays=idl;

  return 0;
}





GWEN_TYPE_UINT32 AH_Storage_OpenDay(AH_STORAGE *st,
                                     unsigned int year,
                                     unsigned int month,
                                     unsigned int day,
                                     int ro){
  GWEN_TYPE_UINT32 id;
  AH_STORAGE_DAY *dy;

  if (st->availDays==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Building index");
    if (AH_Storage_BuildIndex(st)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error building index");
      return 0;
    }
  }

  id=AH_Storage__MakeId(year, month, day);
  dy=AH_Storage_FindDay(st, id);
  if (dy) {
    AH_Storage_Day_Attach(dy);
    return id;
  }

  dy=AH_Storage_GetDay(st, year, month, day, ro);
  if (!dy) {
    return 0;
  }
  AH_Storage_Day_List_Add(dy, st->days);

  return AH_Storage_Day_GetId(dy);
}



int AH_Storage_CloseDay(AH_STORAGE *st, GWEN_TYPE_UINT32 hdl){
  AH_STORAGE_DAY *dy;

  dy=AH_Storage_FindDay(st, hdl);
  if (!dy) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Day not open (handle %08x)", hdl);
    return -1;
  }

  if (AH_Storage_Day_GetChanged(dy)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Day modified, saving");
    if (AH_Storage_SaveDay(st, dy)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error saving day (handle %08x)", hdl);
      return -1;
    }
    AH_Storage_Day_SetChanged(dy, 0);
  }
  AH_Storage_Day_free(dy);
  return 0;
}



void AH_Storage_AbandonDay(AH_STORAGE *st, GWEN_TYPE_UINT32 hdl){
  AH_STORAGE_DAY *dy;

  dy=AH_Storage_FindDay(st, hdl);
  if (!dy) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Day not open (handle %08x)", hdl);
    return;
  }

  AH_Storage_Day_free(dy);
}



int AH_Storage_ClearDay(AH_STORAGE *st, GWEN_TYPE_UINT32 hdl){
  AH_STORAGE_DAY *dy;

  dy=AH_Storage_FindDay(st, hdl);
  if (!dy) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Day not open (handle %08x)", hdl);
    return -1;
  }
  GWEN_DB_ClearGroup(AH_Storage_Day_GetTransactions(dy), 0);
  return 0;
}




GWEN_DB_NODE *AH_Storage_GetFirstTransaction(AH_STORAGE *st,
                                              GWEN_TYPE_UINT32 hdl){
  AH_STORAGE_DAY *dy;

  dy=AH_Storage_FindDay(st, hdl);
  if (!dy) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Day not open (handle %08x)", hdl);
    return 0;
  }

  return AH_Storage_Day_GetFirstTransaction(dy);
}



GWEN_DB_NODE *AH_Storage_GetNextTransaction(AH_STORAGE *st,
                                             GWEN_TYPE_UINT32 hdl){
  AH_STORAGE_DAY *dy;

  dy=AH_Storage_FindDay(st, hdl);
  if (!dy) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Day not open (handle %08x)", hdl);
    return 0;
  }

  return AH_Storage_Day_GetNextTransaction(dy);
}



int AH_Storage_GetFirstDay(AH_STORAGE *st,
                            unsigned int *year,
                            unsigned int *month,
                            unsigned int *day){
  GWEN_TYPE_UINT32 id;

  DBG_INFO(AQHBCI_LOGDOMAIN, "(Re)building index");
  if (AH_Storage_BuildIndex(st)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error building index");
    return -1;
  }

  id=GWEN_IdList_GetFirstId(st->availDays);
  if (!id) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No transactions");
    return -1;
  }

  *year=(id>>16)&0xffff;
  *month=(id>>8)&0xff;
  *day=(id&0xff);
  return 0;
}



int AH_Storage_GetNextDay(AH_STORAGE *st,
                           unsigned int *year,
                           unsigned int *month,
                           unsigned int *day){
  GWEN_TYPE_UINT32 id;

  assert(st);
  assert(year);
  assert(month);
  assert(day);
  assert(st->availDays);

  id=GWEN_IdList_GetNextId(st->availDays);
  if (!id) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No transactions");
    return -1;
  }

  *year=(id>>16)&0xffff;
  *month=(id>>8)&0xff;
  *day=(id&0xff);
  return 0;
}























