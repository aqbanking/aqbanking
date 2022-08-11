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



