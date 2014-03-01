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
#include <aqhbci/user.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/ct.h>
#include <gwenhywfar/ctplugin.h>

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
  int rv;
  GWEN_BUFFER *nameBuffer=NULL;
  const char *tokenName;
  const char *tokenType;
  const char *bankId;
  const char *userId;
  const char *customerId;
  const char *server;
  const char *userName;
  int hbciVersion;
  int rdhType;
  uint32_t cid;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "userName",                   /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "N",                          /* short option */
    "username",                   /* long option */
    "Specify the user name", /* short description */
    "Specify the user name (not the userid!)"  /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
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
    GWEN_ArgsType_Char,           /* type */
    "userId",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "u",                          /* short option */
    "user",                       /* long option */
    "Specify the user id (Benutzerkennung)",        /* short description */
    "Specify the user id (Benutzerkennung)"         /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "customerId",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "c",                          /* short option */
    "customer",                   /* long option */
    "Specify the customer id (Kundennummer)",    /* short description */
    "Specify the customer id (Kundennummer)"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "tokenType",                  /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "t",                          /* short option */
    "tokentype",                  /* long option */
    "Specify the crypt token type", /* short description */
    "Specify the crypt token type"  /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "tokenName",                  /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "n",                          /* short option */
    "tokenname",                  /* long option */
    "Specify the crypt token name", /* short description */
    "Specify the crypt token name"  /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
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
    GWEN_ArgsType_Int,            /* type */
    "context",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "context",                    /* long option */
    "Select a context on the medium", /* short description */
    "Select a context on the medium"  /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,
    GWEN_ArgsType_Int, 
    "hbciversion",  
    0,             
    1,             
    0,             
    "hbciversion", 
    "Select the HBCI version",
    "Select the HBCI protocol version"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,
    GWEN_ArgsType_Int, 
    "rdhType",
    0,
    1,             
    0,             
    "rdhtype",
    "Select the RDH profile type (1, 2, 3, 5, 10)",
    "Select the RDH profile type (1, 2, 3, 5, 10)"
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,            /* type */
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
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
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

  rv=AB_Banking_OnlineInit(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  pro=AB_Banking_GetProvider(ab, "aqhbci");
  assert(pro);

  tokenType=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
  tokenName=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);
  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  userId=GWEN_DB_GetCharValue(db, "userId", 0, 0);
  customerId=GWEN_DB_GetCharValue(db, "customerId", 0, 0);
  server=GWEN_DB_GetCharValue(db, "serverAddr", 0, 0);
  cid=GWEN_DB_GetIntValue(db, "context", 0, 1);
  hbciVersion=GWEN_DB_GetIntValue(db, "hbciVersion", 0, 0);
  rdhType=GWEN_DB_GetIntValue(db, "rdhType", 0, 1);
  userName=GWEN_DB_GetCharValue(db, "userName", 0, 0);
  assert(userName);

  /* generic check for some arguments */
  if (hbciVersion>0 && rdhType>1) {
    if (hbciVersion<300 && rdhType>1) {
      DBG_ERROR(0, "RDH Types 2 and above only work with HBCI version 300 or later");
      return 1;
    }
  }

  if (hbciVersion>0) {
    switch(hbciVersion) {
    case 201:
    case 210:
    case 220:
    case 300:
      /* supported */
      break;

    default:
      DBG_ERROR(0, "HBCI/FinTS version %d not supported", hbciVersion);
      return 1;
    }
  }

  if (rdhType>0) {
    switch(rdhType) {
    case 1:
    case 2:
    case 10:
      /* supported */
      break;

    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    default:
      DBG_ERROR(0, "RDH type %d not supported", rdhType);
      return 1;
    }
  }

  if (1) {
    const char *lbankId;
    const char *luserId;
    const char *lcustomerId;
    const char *lserverAddr;
    AH_CRYPT_MODE cm;
    GWEN_URL *url;
    GWEN_CRYPT_TOKEN_CONTEXT *ctx=NULL;
    AB_USER *user;

    if (strcasecmp(tokenType, "pintan")==0) {
      lbankId=bankId;
      luserId=userId;
      lcustomerId=customerId?customerId:luserId;
      lserverAddr=server;
      cm=AH_CryptMode_Pintan;
    }
    else {
      GWEN_PLUGIN_MANAGER *pm;
      GWEN_PLUGIN *pl;
      GWEN_CRYPT_TOKEN *ct;
      const GWEN_CRYPT_TOKEN_CONTEXT *cctx;
      const GWEN_CRYPT_TOKEN_KEYINFO *ki;
      uint32_t keyId;
      GWEN_CRYPT_CRYPTALGOID algo;

      if (cid==0) {
	DBG_ERROR(0, "No context given.");
	return 1;
      }

      /* get crypt token */
      pm=GWEN_PluginManager_FindPluginManager("ct");
      if (pm==0) {
	DBG_ERROR(0, "Plugin manager not found");
	return 3;
      }

      pl=GWEN_PluginManager_GetPlugin(pm, tokenType);
      if (pl==0) {
	DBG_ERROR(0, "Plugin not found");
	return 3;
      }
      DBG_INFO(0, "Plugin found");

      ct=GWEN_Crypt_Token_Plugin_CreateToken(pl, tokenName);
      if (ct==0) {
	DBG_ERROR(0, "Could not create crypt token");
	return 3;
      }

      /* open crypt token */
      rv=GWEN_Crypt_Token_Open(ct, 0, 0);
      if (rv) {
	DBG_ERROR(0, "Could not open token (%d)", rv);
	GWEN_Crypt_Token_free(ct);
	return 3;
      }

      /* get real token name */
      nameBuffer=GWEN_Buffer_new(0, 64, 0, 1);
      GWEN_Buffer_AppendString(nameBuffer,
			       GWEN_Crypt_Token_GetTokenName(ct));
      tokenName=GWEN_Buffer_GetStart(nameBuffer);

      cctx=GWEN_Crypt_Token_GetContext(ct, cid, 0);
      if (cctx==NULL) {
	DBG_ERROR(0, "Context %02x not found", cid);
	GWEN_Buffer_free(nameBuffer);
	GWEN_Crypt_Token_Close(ct, 1, 0);
	GWEN_Crypt_Token_free(ct);
	return 3;
      }
      ctx=GWEN_Crypt_Token_Context_dup(cctx);
      lbankId=bankId?bankId:GWEN_Crypt_Token_Context_GetServiceId(ctx);

      luserId=userId?userId:GWEN_Crypt_Token_Context_GetUserId(ctx);
      lcustomerId=customerId?customerId:luserId;

      lserverAddr=server?server:GWEN_Crypt_Token_Context_GetAddress(ctx);

      /* determine crypt mode */
      keyId=GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
      if (keyId==0)
	keyId=GWEN_Crypt_Token_Context_GetVerifyKeyId(ctx);
      if (keyId==0)
	keyId=GWEN_Crypt_Token_Context_GetEncipherKeyId(ctx);
      if (keyId==0)
	keyId=GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx);
      GWEN_Crypt_Token_Context_free(ctx);
      if (keyId==0) {
	DBG_ERROR(0, "No keys, unable to determine crypt mode");
	GWEN_Buffer_free(nameBuffer);
	GWEN_Crypt_Token_Close(ct, 1, 0);
	GWEN_Crypt_Token_free(ct);
	return 3;
      }
  
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, keyId, 0xffffffff, 0);
      if (ki==NULL) {
	DBG_ERROR(0,
		  "Could not get keyinfo for key %d, "
		  "unable to determine crypt mode", keyId);
	GWEN_Buffer_free(nameBuffer);
	GWEN_Crypt_Token_Close(ct, 1, 0);
	GWEN_Crypt_Token_free(ct);
	return 3;
      }

      algo=GWEN_Crypt_Token_KeyInfo_GetCryptAlgoId(ki);
      if (algo==GWEN_Crypt_CryptAlgoId_Des3K)
	cm=AH_CryptMode_Ddv;
      else if (algo==GWEN_Crypt_CryptAlgoId_Rsa)
	cm=AH_CryptMode_Rdh;
      else {
	DBG_ERROR(0,
		  "Unexpected crypt algorithm \"%s\", "
		  "unable to determine crypt mode",
		  GWEN_Crypt_CryptAlgoId_toString(algo));
	GWEN_Buffer_free(nameBuffer);
	GWEN_Crypt_Token_Close(ct, 1, 0);
	GWEN_Crypt_Token_free(ct);
	return 3;
      }

      rv=GWEN_Crypt_Token_Close(ct, 0, 0);
      GWEN_Crypt_Token_free(ct);
      if (rv) {
	DBG_ERROR(0, "Could not close token (%d)", rv);
	GWEN_Buffer_free(nameBuffer);
	return 3;
      }
    }

    if (!lbankId || !*lbankId) {
      DBG_ERROR(0, "No bank id stored and none given");
      GWEN_Buffer_free(nameBuffer);
      return 3;
    }
    if (!luserId || !*luserId) {
      DBG_ERROR(0, "No user id (Benutzerkennung) stored and none given");
      GWEN_Buffer_free(nameBuffer);
      return 3;
    }

    user=AB_Banking_FindUser(ab, AH_PROVIDER_NAME,
			     "de",
			     lbankId, luserId, lcustomerId);
    if (user) {
      DBG_ERROR(0, "User %s already exists", luserId);
      return 3;
    }

    user=AB_Banking_CreateUser(ab, AH_PROVIDER_NAME);
    assert(user);

    AB_User_SetUserName(user, userName);
    AB_User_SetCountry(user, "de");
    AB_User_SetBankCode(user, lbankId);
    AB_User_SetUserId(user, luserId);
    AB_User_SetCustomerId(user, lcustomerId);
    AH_User_SetTokenType(user, tokenType);
    AH_User_SetTokenName(user, tokenName);
    AH_User_SetTokenContextId(user, cid);
    AH_User_SetCryptMode(user, cm);
    if (rdhType>0)
      AH_User_SetRdhType(user, rdhType);
    GWEN_Buffer_free(nameBuffer);

    if (hbciVersion==0) {
      if (cm==AH_CryptMode_Pintan)
	AH_User_SetHbciVersion(user, 220);
      else {
        if (rdhType>1)
	  AH_User_SetHbciVersion(user, 300);
        else
	  AH_User_SetHbciVersion(user, 210);
      }
    }
    else {
      AH_User_SetHbciVersion(user, hbciVersion);
    }

    /* try to get server address from database if still unknown */
    if (!lserverAddr || *lserverAddr==0) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (getBankUrl(ab,
                     cm,
                     lbankId,
		     tbuf)) {
	DBG_INFO(0, "Could not find server address for \"%s\"",
		 lbankId);
      }
      if (GWEN_Buffer_GetUsedBytes(tbuf)==0) {
	DBG_ERROR(0, "No address given and none available in internal db");
	return 3;
      }
      url=GWEN_Url_fromString(GWEN_Buffer_GetStart(tbuf));
      if (url==NULL) {
	DBG_ERROR(0, "Bad URL \"%s\" in internal db",
		  GWEN_Buffer_GetStart(tbuf));
	return 3;
      }
      GWEN_Buffer_free(tbuf);
    }
    else {
      /* set address */
      url=GWEN_Url_fromString(lserverAddr);
      if (url==NULL) {
	DBG_ERROR(0, "Bad URL \"%s\"", lserverAddr);
	return 3;
      }
    }

    if (cm==AH_CryptMode_Pintan) {
      GWEN_Url_SetProtocol(url, "https");
      if (GWEN_Url_GetPort(url)==0)
	GWEN_Url_SetPort(url, 443);
    }
    else {
      GWEN_Url_SetProtocol(url, "hbci");
      if (GWEN_Url_GetPort(url)==0)
	GWEN_Url_SetPort(url, 3000);
    }
    AH_User_SetServerUrl(user, url);
    GWEN_Url_free(url);

    if (cm==AH_CryptMode_Ddv)
      AH_User_SetStatus(user, AH_UserStatusEnabled);

    AB_Banking_AddUser(ab, user);
  }

  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }


  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}





