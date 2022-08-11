/***************************************************************************
 begin       : Thur Aug 11 2022
 copyright   : (C) 2022 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


/*
 * This file is included by provider.c
 */




void AB_Provider_DumpTransactionsIfDebug(const AB_IMEXPORTER_ACCOUNTINFO *ai, const char *logdomain)
{
  if (GWEN_Logger_GetLevel(logdomain)>=GWEN_LoggerLevel_Debug) {
    GWEN_DB_NODE *gn;
    AB_TRANSACTION *ttmp;

    DBG_INFO(logdomain, "*** Dumping transactions *******************");
    ttmp=AB_ImExporterAccountInfo_GetFirstTransaction(ai, 0, 0);
    while (ttmp) {
      DBG_INFO(logdomain, "*** --------------------------------------");
      gn=GWEN_DB_Group_new("transaction");
      AB_Transaction_toDb(ttmp, gn);
      GWEN_DB_Dump(gn, 2);
      GWEN_DB_Group_free(gn);
      ttmp=AB_Transaction_List_Next(ttmp);
    }

    DBG_INFO(logdomain, "*** End dumping transactions ***************");
  }
}



void AB_Provider_MergeContextsSetTypeAndFreeSrc(AB_IMEXPORTER_ACCOUNTINFO *destAccountInfo, AB_IMEXPORTER_CONTEXT *srcContext, int ty)
{
  AB_IMEXPORTER_ACCOUNTINFO *srcAccountInfo;

  /* copy data from temporary context to real context */
  srcAccountInfo=AB_ImExporterContext_GetFirstAccountInfo(srcContext);
  while (srcAccountInfo) {
    AB_TRANSACTION_LIST *tl;
    AB_BALANCE_LIST *bl;

    /* move transactions, set transaction type */
    tl=AB_ImExporterAccountInfo_GetTransactionList(srcAccountInfo);
    if (tl) {
      AB_TRANSACTION *t;

      while ((t=AB_Transaction_List_First(tl))) {
        AB_Transaction_List_Del(t);
        AB_Transaction_SetType(t, ty);
        AB_ImExporterAccountInfo_AddTransaction(destAccountInfo, t);
      }
    }

    /* move balances */
    bl=AB_ImExporterAccountInfo_GetBalanceList(srcAccountInfo);
    if (bl) {
      AB_BALANCE *bal;

      while ((bal=AB_Balance_List_First(bl))) {
        AB_Balance_List_Del(bal);
        AB_ImExporterAccountInfo_AddBalance(destAccountInfo, bal);
      }
    }

    srcAccountInfo=AB_ImExporterAccountInfo_List_Next(srcAccountInfo);
  }
  AB_ImExporterContext_free(srcContext);
}



AB_IMEXPORTER_ACCOUNTINFO *AB_Provider_GetOrAddAccountInfoForAccount(AB_IMEXPORTER_CONTEXT *ctx, const AB_ACCOUNT *a)
{
  if (a)
    return AB_ImExporterContext_GetOrAddAccountInfo(ctx,
						    AB_Account_GetUniqueId(a),
						    AB_Account_GetIban(a),
						    AB_Account_GetBankCode(a),
						    AB_Account_GetAccountNumber(a),
						    AB_Account_GetAccountType(a));
  else
    return AB_ImExporterContext_GetOrAddAccountInfo(ctx, 0, NULL, NULL, NULL, AB_AccountType_Unknown);
}




