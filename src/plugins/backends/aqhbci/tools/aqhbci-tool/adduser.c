/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals.h"

#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



int getBankUrl(AB_BANKING *ab,
               AH_CRYPT_MODE cm,
	       const char *bankId,
	       GWEN_BUFFER *bufServer) {
  AB_BANKINFO *bi;
  
  bi=AB_Banking_GetBankInfo(ab, "de", 0, bankId);
  if (bi) {
    AB_BANKINFO_SERVICE_LIST *l;
    AB_BANKINFO_SERVICE *sv;
  
    l=AB_BankInfo_GetServices(bi);
    assert(l);
    sv=AB_BankInfoService_List_First(l);
    while(sv) {
      const char *st;
  
      st=AB_BankInfoService_GetType(sv);
      if (st && *st && strcasecmp(st, "hbci")==0) {
	const char *svm;
  
	svm=AB_BankInfoService_GetMode(sv);
	if (svm && *svm) {
	  if (!
	      ((strcasecmp(svm, "pintan")==0) ^
               (cm==AH_CryptMode_Pintan))){
            const char *addr;

            addr=AB_BankInfoService_GetAddress(sv);
	    if (addr && *addr) {
	      GWEN_Buffer_Reset(bufServer);
	      GWEN_Buffer_AppendString(bufServer, addr);
              return 0;
	    }
	  }
	}
      }
      sv=AB_BankInfoService_List_Next(sv);
    }
    AB_BankInfo_free(bi);
  } /* if bank info */

  return -1;
}



int addUser(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  AH_HBCI *hbci;
  int rv;
  const char *bankId;
  const char *userId;
  const char *customerId;
  AH_MEDIUM *medium=0;
  int mediumNumber;
  const char *server;
  const AH_MEDIUM_LIST *ml;
  int i;
  int idx;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "bankId",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "b",                          /* short option */
    "bank",                       /* long option */
    "Specify the bank code",      /* short description */
    "Specify the bank code"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "userId",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "u",                          /* short option */
    "user",                       /* long option */
    "Specify the user id",        /* short description */
    "Specify the user id"         /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "customerId",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "c",                          /* short option */
    "customer",                   /* long option */
    "Specify the customer id",    /* short description */
    "Specify the customer id"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeInt,             /* type */
    "mediumNumber",               /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "m",                          /* short option */
    "medium",                     /* long option */
    "Specify the medium number",    /* short description */
    "Specify the medium number"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "serverAddr",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "s",                          /* short option */
    "server",                     /* long option */
    "Specify the server URL",     /* short description */
    "Specify the server URL"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeInt,             /* type */
    "context",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "context",                    /* long option */
    "Select a context on the medium", /* short description */
    "Select a context on the medium"  /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsTypeInt,             /* type */
    "help",                       /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "h",                          /* short option */
    "help",                       /* long option */
    "Show this help screen",      /* short description */
    "Show this help screen"       /* long description */
  }
  };

  db=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "local");
  rv=GWEN_Args_Check(argc, argv, 1,
                     0 /*GWEN_ARGS_MODE_ALLOW_FREEPARAM*/,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutTypeTXT)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  pro=AB_Banking_GetProvider(ab, "aqhbci");
  assert(pro);
  hbci=AH_Provider_GetHbci(pro);
  assert(hbci);

  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  userId=GWEN_DB_GetCharValue(db, "userId", 0, 0);
  customerId=GWEN_DB_GetCharValue(db, "customerId", 0, 0);
  server=GWEN_DB_GetCharValue(db, "serverAddr", 0, 0);
  mediumNumber=GWEN_DB_GetIntValue(db, "mediumNumber", 0, 0);

  ml=AH_HBCI_GetMediaList(hbci);
  medium=0;
  if (ml) {
    i=mediumNumber;
    medium=AH_Medium_List_First(ml);
    while(medium && i--)
      medium=AH_Medium_List_Next(medium);
  }

  if (!medium) {
    DBG_ERROR(0, "Medium number \"%d\" not available", mediumNumber);
    return 3;
  }


  rv=AH_Medium_Mount(medium);
  if (rv) {
    DBG_ERROR(0, "Error mounting medium (%d)", rv);
    return 3;
  }
  idx=GWEN_DB_GetIntValue(db, "context", 0, 0);
  if (idx<0) {
    DBG_ERROR(0, "No context given.");
    return 3;
  }
  if (idx>=0) {
    int country;
    GWEN_BUFFER *bufBankId;
    GWEN_BUFFER *bufUserId;
    GWEN_BUFFER *bufServer;
    int port;
    int rv;
    AH_BANK *bank;
    AH_USER *user;
    AH_CRYPT_MODE cm;
    AH_CUSTOMER *customer;
    AH_BPD_ADDR *ba;
    AH_MEDIUM_CTX *mctx;
    const char *lbankId;
    const char *luserId;
    const char *lcustomerId;
    const char *lserverAddr;
    const GWEN_CRYPTTOKEN_CONTEXT *ctx;
    const GWEN_CRYPTTOKEN_CRYPTINFO *ci;
    GWEN_CRYPTTOKEN_CRYPTALGO ca;

    bufBankId=GWEN_Buffer_new(0, 32, 0, 1);
    bufUserId=GWEN_Buffer_new(0, 32, 0, 1);
    bufServer=GWEN_Buffer_new(0, 32, 0, 1);
    rv=AH_Medium_ReadContext(medium,
                             idx,
                             &country,
                             bufBankId,
                             bufUserId,
                             bufServer,
                             &port);
    if (rv) {
      DBG_ERROR(0, "Error reading context %d from medium", idx);
      return 3;
    }

    lbankId=bankId?bankId:GWEN_Buffer_GetStart(bufBankId);
    if (!lbankId || !*lbankId) {
      DBG_ERROR(0, "No bank id stored and none given");
      GWEN_Buffer_free(bufServer);
      GWEN_Buffer_free(bufUserId);
      GWEN_Buffer_free(bufBankId);
      return 3;
    }
    bank=AH_HBCI_FindBank(hbci, 280, lbankId);
    if (!bank) {
      bank=AH_Bank_new(hbci, 280, lbankId);
      assert(bank);
      AH_HBCI_AddBank(hbci, bank);
    }

    luserId=userId?userId:GWEN_Buffer_GetStart(bufUserId);
    if (!luserId || !*luserId) {
      DBG_ERROR(0, "No user id stored and none given");
      GWEN_Buffer_free(bufServer);
      GWEN_Buffer_free(bufUserId);
      GWEN_Buffer_free(bufBankId);
      return 3;
    }

    user=AH_Bank_FindUser(bank, luserId);
    if (user) {
      DBG_ERROR(0, "User %s already exists", luserId);
      GWEN_Buffer_free(bufServer);
      GWEN_Buffer_free(bufUserId);
      GWEN_Buffer_free(bufBankId);
      return 3;
    }

    rv=AH_Medium_SelectContext(medium, idx);
    if (rv) {
      DBG_ERROR(0, "Could not select context %d (%d)", idx, rv);
      GWEN_Buffer_free(bufServer);
      GWEN_Buffer_free(bufUserId);
      GWEN_Buffer_free(bufBankId);
      return 3;
    }

    mctx=AH_Medium_GetCurrentContext(medium);
    assert(mctx);

    ctx=AH_MediumCtx_GetTokenContext(mctx);
    assert(ctx);
    ci=GWEN_CryptToken_Context_GetCryptInfo(ctx);
    assert(ci);
    ca=GWEN_CryptToken_CryptInfo_GetCryptAlgo(ci);
    if (ca==GWEN_CryptToken_CryptAlgo_RSA)
      cm=AH_CryptMode_Rdh;
    else if (ca==GWEN_CryptToken_CryptAlgo_DES_3K)
      cm=AH_CryptMode_Ddv;
    else if (ca==GWEN_CryptToken_CryptAlgo_None)
      cm=AH_CryptMode_Pintan;
    else {
      DBG_ERROR(0, "Invalid crypt algo (%s), unable to detect crypt mode",
                GWEN_CryptToken_CryptAlgo_toString(ca));
      GWEN_Buffer_free(bufServer);
      GWEN_Buffer_free(bufUserId);
      GWEN_Buffer_free(bufBankId);
      return 3;
    }

    user=AH_User_new(bank, luserId, cm, medium);
    assert(user);
    AH_Bank_AddUser(bank, user);
    AH_User_SetContextIdx(user, idx);

    lcustomerId=customerId?customerId:luserId;
    customer=AH_User_FindCustomer(user, lcustomerId);
    if (customer) {
      DBG_ERROR(0, "Customer %s already exists", lcustomerId);
      GWEN_Buffer_free(bufServer);
      GWEN_Buffer_free(bufUserId);
      GWEN_Buffer_free(bufBankId);
      return 3;
    }

    customer=AH_Customer_new(user, lcustomerId);
    assert(customer);
    AH_User_AddCustomer(user, customer);
    if (cm==AH_CryptMode_Pintan)
      AH_Customer_SetHbciVersion(customer, 220);
    else
      AH_Customer_SetHbciVersion(customer, 210);

    /* try to get server address from database if still unknown */
    if (!server && GWEN_Buffer_GetUsedBytes(bufServer)==0) {
      if (getBankUrl(ab,
                     cm,
                     lbankId,
                     bufServer)) {
        DBG_INFO(0, "Could not find server address for \"%s\"",
                 lbankId);
      }
    }
    lserverAddr=server?server:GWEN_Buffer_GetStart(bufServer);
    if (!lserverAddr || !*lserverAddr) {
      DBG_ERROR(0, "No address given and none available in internal db");
      GWEN_Buffer_free(bufServer);
      GWEN_Buffer_free(bufUserId);
      GWEN_Buffer_free(bufBankId);
      return 3;
    }

    /* set address */
    ba=AH_BpdAddr_new();
    AH_BpdAddr_SetAddr(ba, lserverAddr);
    if (cm==AH_CryptMode_Pintan) {
      AH_BpdAddr_SetType(ba, AH_BPD_AddrTypeSSL);
      AH_BpdAddr_SetSuffix(ba, "443");
    }
    else {
      AH_BpdAddr_SetType(ba, AH_BPD_AddrTypeTCP);
      AH_BpdAddr_SetSuffix(ba, "3000");
    }
    AH_User_SetAddress(user, ba);
    AH_BpdAddr_free(ba);

    GWEN_Buffer_free(bufServer);
    GWEN_Buffer_free(bufUserId);
    GWEN_Buffer_free(bufBankId);

    if (cm==AH_CryptMode_Ddv)
      AH_User_SetStatus(user, AH_UserStatusEnabled);
  }
  else {
    DBG_ERROR(0, "Invalid context %d", idx);
    return 3;
  }


  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}





