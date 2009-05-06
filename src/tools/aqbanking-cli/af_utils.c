/***************************************************************************
 begin       : Fri Nov 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#ifdef WITH_AQFINANCE


#include <aqfinance/engine/ae.h>
#include <aqfinance/engine/db/aedb_db.h>
#include <aqfinance/engine/book/ae_book.h>
#include <aqfinance/engine/modules/ae_statementimport.h>
#include <aqfinance/engine/modules/ae_statementexport.h>
#include <aqfinance/engine/modules/ae_recontransfers.h>



static int get_db_url(AB_BANKING *ab, GWEN_BUFFER *pbuf) {
  int rv;
  const char *s;

  rv=GWEN_Init();
  if (rv) {
    DBG_ERROR_ERR(0, rv);
    return rv;
  }

  if (!GWEN_Logger_IsOpen(AE_LOGDOMAIN)) {
    GWEN_Logger_Open(AE_LOGDOMAIN,
		     "aqfinance", 0,
		     GWEN_LoggerType_Console,
		     GWEN_LoggerFacility_User);
  }

  s=getenv("AE_LOGLEVEL");
  if (s && *s) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(s);
    GWEN_Logger_SetLevel(AE_LOGDOMAIN, ll);
  }
  else
    GWEN_Logger_SetLevel(AE_LOGDOMAIN, GWEN_LoggerLevel_Notice);

  GWEN_Buffer_AppendString(pbuf, "dir://");
  rv=AB_Banking_GetSharedDataDir(ab, "aqfinance", pbuf);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to get shared data dir (%d)\n", rv);
    return rv;
  }

  return 0;
}



int AFM_create_book(AB_BANKING *ab) {
  GWEN_BUFFER *pbuf;
  AEDB_DB *db=NULL;
  AE_BOOK *b;
  int rv;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=get_db_url(ab, pbuf);
  if (rv<0) {
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  rv=AE_DbFactory(GWEN_Buffer_GetStart(pbuf), &db);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to instantiate db (%d)\n", rv);
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  GWEN_Buffer_free(pbuf);

  b=AE_Book_new(db);

  rv=AE_Book_Create(b, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to create book (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_Open(b, AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open book (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_CreateTable(b, AE_Book_TableType_IdCounter, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to create table 1 (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_CreateTable(b, AE_Book_TableType_BankAccount, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to create table BankAccount (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_CreateTable(b, AE_Book_TableType_BookedBalance, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to create table BookedBalance (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_CreateTable(b, AE_Book_TableType_NotedBalance, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to create table NotedBalance (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_CreateTable(b, AE_Book_TableType_BankStatement, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to create table 5 (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_CreateTable(b, AE_Book_TableType_BankTransfer, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to create table 6 (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  AE_Book_free(b);

  return 0;
}



int AFM_add_transfer_or_debitnote(AB_BANKING *ab, AB_TRANSACTION *t,
                                  AE_BOOK_TABLE_TYPE tt,
                                  AE_STATEMENT_STATUS status) {
  GWEN_BUFFER *pbuf;
  AEDB_DB *db=NULL;
  AE_BOOK *b;
  int rv;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=get_db_url(ab, pbuf);
  if (rv<0) {
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  rv=AE_DbFactory(GWEN_Buffer_GetStart(pbuf), &db);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to instantiate db (%d)\n", rv);
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  GWEN_Buffer_free(pbuf);

  b=AE_Book_new(db);

  rv=AE_Book_Open(b, AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open book (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_OpenTable(b, AE_Book_TableType_IdCounter,
		       AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open table 1 (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_OpenTable(b, tt, AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open table %d (%d)\n", tt, rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  if (t) {
    AE_STATEMENT *st;

    /* transform transfer to statement */
    st=AE_Statement_new();
    rv=AE_StatementImport_TransactionToStatement(t, st);
    if (rv<0) {
      fprintf(stderr, "ERROR: Unable to open table 6 (%d)\n", rv);
      AE_Statement_free(st);
      AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
      AE_Book_free(b);
      return rv;
    }

    if (AE_Statement_GetDate(st)==NULL) {
      GWEN_TIME *ti;

      ti=GWEN_CurrentTime();
      AE_Statement_SetDate(st, ti);
      GWEN_Time_free(ti);
    }

    AE_Statement_SetStatus(st, status);

    rv=AE_Book_AddStatement(b, tt, st, 1);
    if (rv<0) {
      DBG_INFO(AE_LOGDOMAIN, "here (%d)", rv);
      AE_Statement_free(st);
      AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
      AE_Book_free(b);
      return rv;
    }

    /* store statement id in transfer for later recognition */
    AB_Transaction_SetIdForApplication(t, AE_Statement_GetId(st));
    AE_Statement_free(st);
  } /* if t */

  rv=AE_Book_Close(b, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to close book (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  AE_Book_free(b);

  return 0;
}



int AFM_add_transfers_or_debitnotes(AB_BANKING *ab, const AB_TRANSACTION_LIST2 *tl,
                                    AE_BOOK_TABLE_TYPE tt,
                                    AE_STATEMENT_STATUS status) {
  AB_TRANSACTION_LIST2_ITERATOR *it;

  it=AB_Transaction_List2_First(tl);
  if (it) {
    GWEN_BUFFER *pbuf;
    AEDB_DB *db=NULL;
    AE_BOOK *b;
    int rv;
    AB_TRANSACTION *t;
  
    pbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=get_db_url(ab, pbuf);
    if (rv<0) {
      GWEN_Buffer_free(pbuf);
      AB_Transaction_List2Iterator_free(it);
      return rv;
    }
    rv=AE_DbFactory(GWEN_Buffer_GetStart(pbuf), &db);
    if (rv<0) {
      fprintf(stderr, "ERROR: Unable to instantiate db (%d)\n", rv);
      GWEN_Buffer_free(pbuf);
      AB_Transaction_List2Iterator_free(it);
      return rv;
    }
    GWEN_Buffer_free(pbuf);
  
    b=AE_Book_new(db);
  
    rv=AE_Book_Open(b, AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
    if (rv<0) {
      fprintf(stderr, "ERROR: Unable to open book (%d)\n", rv);
      AE_Book_free(b);
      AB_Transaction_List2Iterator_free(it);
      return rv;
    }
  
    rv=AE_Book_OpenTable(b, AE_Book_TableType_IdCounter,
                         AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
    if (rv<0) {
      fprintf(stderr, "ERROR: Unable to open table 1 (%d)\n", rv);
      AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
      AE_Book_free(b);
      AB_Transaction_List2Iterator_free(it);
      return rv;
    }
  
    rv=AE_Book_OpenTable(b, tt, AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
    if (rv<0) {
      fprintf(stderr, "ERROR: Unable to open table %d (%d)\n", tt, rv);
      AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
      AE_Book_free(b);
      AB_Transaction_List2Iterator_free(it);
      return rv;
    }
  
    t=AB_Transaction_List2Iterator_Data(it);
    while(t) {
      AE_STATEMENT *st;
  
      /* transform transfer to statement */
      st=AE_Statement_new();
      rv=AE_StatementImport_TransactionToStatement(t, st);
      if (rv<0) {
        fprintf(stderr, "ERROR: Unable to open table 6 (%d)\n", rv);
        AE_Statement_free(st);
        AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
        AE_Book_free(b);
        AB_Transaction_List2Iterator_free(it);
        return rv;
      }
  
      if (AE_Statement_GetDate(st)==NULL) {
        GWEN_TIME *ti;
  
        ti=GWEN_CurrentTime();
        AE_Statement_SetDate(st, ti);
        GWEN_Time_free(ti);
      }
  
      AE_Statement_SetStatus(st, status);
  
      rv=AE_Book_AddStatement(b, tt, st, 1);
      if (rv<0) {
        DBG_INFO(AE_LOGDOMAIN, "here (%d)", rv);
        AE_Statement_free(st);
        AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
        AE_Book_free(b);
        AB_Transaction_List2Iterator_free(it);
        return rv;
      }
  
      /* store statement id in transfer for later recognition */
      AB_Transaction_SetIdForApplication(t, AE_Statement_GetId(st));
      AE_Statement_free(st);

      t=AB_Transaction_List2Iterator_Next(it);
    } /* while t */
    AB_Transaction_List2Iterator_free(it);

    rv=AE_Book_Close(b, 0);
    if (rv<0) {
      fprintf(stderr, "ERROR: Unable to close book (%d)\n", rv);
      AE_Book_free(b);
      return rv;
    }
  
    AE_Book_free(b);
  }

  return 0;
}



int AFM_update_transfers_or_debitnotes(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *iec,
                                       AE_BOOK_TABLE_TYPE tt,
                                       AB_TRANSACTION_TYPE ttype) {
  GWEN_BUFFER *pbuf;
  AEDB_DB *db=NULL;
  AE_BOOK *b;
  int rv;
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=get_db_url(ab, pbuf);
  if (rv<0) {
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  rv=AE_DbFactory(GWEN_Buffer_GetStart(pbuf), &db);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to instantiate db (%d)\n", rv);
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  GWEN_Buffer_free(pbuf);

  b=AE_Book_new(db);

  rv=AE_Book_Open(b, AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open book (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_OpenTable(b, tt, AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open table BankDebitNote (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  /* lock database */
  rv=AE_Book_BeginEdit(b, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to begin editing (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  /* handle all transfers */
  iea=AB_ImExporterContext_GetFirstAccountInfo(iec);
  while(iea) {
    const AB_TRANSACTION *t;

    t=AB_ImExporterAccountInfo_GetFirstTransfer(iea);
    while(t) {
      if (AB_Transaction_GetType(t)==ttype) {
        AEDB_ID stmId;
    
        stmId=AB_Transaction_GetIdForApplication(t);
        if (stmId) {
          AE_STATEMENT *st=NULL;

          rv=AE_Book_ReadStatement(b, tt, stmId, &st);
          if (rv<0) {
            fprintf(stderr, "WARNING: Debit note not found in internal db (%d)\n", rv);
          }
          else {
            /* transform transfer to statement (thereby overwriting existing transfer with new data) */
            rv=AE_StatementImport_TransactionToStatement(t, st);
            if (rv<0) {
              fprintf(stderr, "ERROR: Unable to transform transfer to statement (%d)\n", rv);
              AE_Statement_free(st);
              AE_Book_EndEdit(b, AEDB_ACTION_FLAGS_ABORT);
              AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
              AE_Book_free(b);
              return rv;
            }

            rv=AE_Book_WriteStatement(b, tt, st, 0);
            if (rv<0) {
              fprintf(stderr, "ERROR: Unable to transform transfer to statement (%d)\n", rv);
              AE_Statement_free(st);
              AE_Book_EndEdit(b, AEDB_ACTION_FLAGS_ABORT);
              AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
              AE_Book_free(b);
              return rv;
            }

            AE_Statement_free(st);
          }
        } /* if stmId */
      }
      t=AB_ImExporterAccountInfo_GetNextTransfer(iea);
    } /* while t */
    iea=AB_ImExporterContext_GetNextAccountInfo(iec);
  } /* while iea */


  /* unlock database */
  rv=AE_Book_EndEdit(b, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to end editing (%d)\n", rv);
    AE_Book_EndEdit(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_Close(b, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to close book (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  AE_Book_free(b);

  return 0;
}



int AFM_add_transfers(AB_BANKING *ab, const AB_TRANSACTION_LIST2 *tl,
                      AE_STATEMENT_STATUS status) {
  return AFM_add_transfers_or_debitnotes(ab, tl, AE_Book_TableType_BankTransfer, status);
}



int AFM_add_transfer(AB_BANKING *ab, AB_TRANSACTION *t, AE_STATEMENT_STATUS status) {
  return AFM_add_transfer_or_debitnote(ab, t, AE_Book_TableType_BankTransfer, status);
}



int AFM_update_transfers(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *iec) {
  return AFM_update_transfers_or_debitnotes(ab, iec,
                                            AE_Book_TableType_BankTransfer,
                                            AB_Transaction_TypeTransfer);
}



int AFM_add_debitnote(AB_BANKING *ab, AB_TRANSACTION *t, AE_STATEMENT_STATUS status) {
  return AFM_add_transfer_or_debitnote(ab, t, AE_Book_TableType_BankDebitNote, status);
}



int AFM_add_debitnotes(AB_BANKING *ab, const AB_TRANSACTION_LIST2 *tl,
                       AE_STATEMENT_STATUS status) {
  return AFM_add_transfers_or_debitnotes(ab, tl, AE_Book_TableType_BankDebitNote, status);
}



int AFM_update_debitnotes(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *iec) {
  return AFM_update_transfers_or_debitnotes(ab, iec,
                                            AE_Book_TableType_BankDebitNote,
                                            AB_Transaction_TypeDebitNote);
}



int AFM_import_statements(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *iec) {
  GWEN_BUFFER *pbuf;
  AEDB_DB *db=NULL;
  AE_BOOK *b;
  int rv;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=get_db_url(ab, pbuf);
  if (rv<0) {
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  rv=AE_DbFactory(GWEN_Buffer_GetStart(pbuf), &db);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to instantiate db (%d)\n", rv);
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  GWEN_Buffer_free(pbuf);

  b=AE_Book_new(db);

  rv=AE_Book_Open(b, AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open book (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_OpenTable(b, AE_Book_TableType_IdCounter,
		       AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open table IdCounter (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_OpenTable(b, AE_Book_TableType_BankAccount,
                       AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
      fprintf(stderr, "ERROR: Unable to open table BankAccount (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_OpenTable(b, AE_Book_TableType_BookedBalance,
                       AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
      fprintf(stderr, "ERROR: Unable to open table BookedBalance (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_OpenTable(b, AE_Book_TableType_NotedBalance,
                       AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
      fprintf(stderr, "ERROR: Unable to open table NotedBalance (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_OpenTable(b, AE_Book_TableType_BankStatement,
                       AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
      fprintf(stderr, "ERROR: Unable to open table BankStatement (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_OpenTable(b, AE_Book_TableType_BankTransfer,
                       AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open table BankStatement (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_StatementImport_ImportContext(b, iec, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to import context (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_Close(b, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to close book (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  AE_Book_free(b);

  return 0;
}



int AFM_import_recon_transfers(AB_BANKING *ab) {
  GWEN_BUFFER *pbuf;
  AEDB_DB *db=NULL;
  AE_BOOK *b;
  int rv;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=get_db_url(ab, pbuf);
  if (rv<0) {
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  rv=AE_DbFactory(GWEN_Buffer_GetStart(pbuf), &db);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to instantiate db (%d)\n", rv);
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  GWEN_Buffer_free(pbuf);

  b=AE_Book_new(db);

  rv=AE_Book_Open(b, AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open book (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_OpenTable(b, AE_Book_TableType_BankStatement,
		       AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open table 6 (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_OpenTable(b, AE_Book_TableType_BankTransfer,
		       AEDB_ACTION_FLAGS_READ | AEDB_ACTION_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open table 6 (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_ReconTransfers_ReconTransfers(b);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to reconcile transfers (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_Close(b, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to close book (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  AE_Book_free(b);

  return 0;
}



int AFM_import_export_statements(AB_BANKING *ab,
				 AB_IMEXPORTER_CONTEXT *iec,
				 AE_BOOK_TABLE_TYPE tt,
				 const char *queryString) {
  GWEN_BUFFER *pbuf;
  AEDB_DB *db=NULL;
  AE_BOOK *b;
  int rv;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=get_db_url(ab, pbuf);
  if (rv<0) {
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  rv=AE_DbFactory(GWEN_Buffer_GetStart(pbuf), &db);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to instantiate db (%d)\n", rv);
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  GWEN_Buffer_free(pbuf);

  b=AE_Book_new(db);

  rv=AE_Book_Open(b, AEDB_ACTION_FLAGS_READ);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open book (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_OpenTable(b, tt, AEDB_ACTION_FLAGS_READ);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to open table 6 (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_StatementImport_ExportContext(b, iec, tt, queryString);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to export context (%d)\n", rv);
    AE_Book_Close(b, AEDB_ACTION_FLAGS_ABORT);
    AE_Book_free(b);
    return rv;
  }

  rv=AE_Book_Close(b, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Unable to close book (%d)\n", rv);
    AE_Book_free(b);
    return rv;
  }

  AE_Book_free(b);

  return 0;
}






#endif

