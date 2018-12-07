/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals.h"
#include <aqebics/user.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/ct.h>
#include <gwenhywfar/ctplugin.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



static int getBankUrl(AB_BANKING *ab,
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
      if (st && *st && strcasecmp(st, "ebics")==0) {
	const char *addr;

	addr=AB_BankInfoService_GetAddress(sv);
	if (addr && *addr) {
	  GWEN_Buffer_Reset(bufServer);
	  GWEN_Buffer_AppendString(bufServer, addr);
	  return 0;
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
  const char *userName;
  const char *hostName;
  const char *server;
  const char *ebicsVersion;
  int importing;
  uint32_t cid;
  const GWEN_ARGS args[]={
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
    GWEN_ArgsType_Char,           /* type */
    "hostName",                 /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "H",                          /* short option */
    "hostname",                     /* long option */
    "Specify the EBICS hostname",     /* short description */
    "Specify the EBICS hostname"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "userName",                 /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "N",                          /* short option */
    "username",                     /* long option */
    "Specify the realname of the user",     /* short description */
    "Specify the realname of the user"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "ebicsVersion",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "E",                          /* short option */
    "ebicsversion",               /* long option */
    "Specify the EBICS version to use (e.g. H002)",     /* short description */
    "Specify the EBICS version to use (e.g. H002)"      /* long description */
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
    0,                            /* flags */
    GWEN_ArgsType_Int,            /* type */
    "import",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "import",                     /* long option */
    "Import a user which has already been in use (e.g. with previous versions)",
    "Import a user which has already been in use (e.g. with previous versions)"
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
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
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

  pro=AB_Banking_GetProvider(ab, EBC_PROVIDER_NAME);
  assert(pro);

  tokenType=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
  tokenName=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);
  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  userId=GWEN_DB_GetCharValue(db, "userId", 0, 0);
  customerId=GWEN_DB_GetCharValue(db, "customerId", 0, 0);
  hostName=GWEN_DB_GetCharValue(db, "hostName", 0, 0);
  userName=GWEN_DB_GetCharValue(db, "userName", 0, 0);
  server=GWEN_DB_GetCharValue(db, "serverAddr", 0, 0);
  cid=GWEN_DB_GetIntValue(db, "context", 0, 0);
  importing=GWEN_DB_GetIntValue(db, "import", 0, 0);
  ebicsVersion=GWEN_DB_GetCharValue(db, "ebicsVersion", 0, "H003");

  if (1) {
    const char *lbankId;
    const char *luserId;
    const char *lcustomerId;
    const char *lserverAddr;
    GWEN_URL *url;
    GWEN_CRYPT_TOKEN_CONTEXT *ctx=NULL;
    AB_USER *user;

    if (1) {
      GWEN_PLUGIN_MANAGER *pm;
      GWEN_PLUGIN *pl;
      GWEN_CRYPT_TOKEN *ct;
      const GWEN_CRYPT_TOKEN_CONTEXT *cctx;

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
	return 3;
      }
      ctx=GWEN_Crypt_Token_Context_dup(cctx);
      lbankId=bankId?bankId:GWEN_Crypt_Token_Context_GetServiceId(ctx);

      luserId=userId?userId:GWEN_Crypt_Token_Context_GetUserId(ctx);
      lcustomerId=customerId?customerId:luserId;

      lserverAddr=server?server:GWEN_Crypt_Token_Context_GetAddress(ctx);

      rv=GWEN_Crypt_Token_Close(ct, 0, 0);
      if (rv) {
	DBG_ERROR(0, "Could not close token (%d)", rv);
	return 3;
      }

      GWEN_Crypt_Token_free(ct);
    }

    if (!lbankId || !*lbankId) {
      DBG_ERROR(0, "No bank id stored and none given");
      return 3;
    }
    if (!luserId || !*luserId) {
      DBG_ERROR(0, "No user id (Benutzerkennung) stored and none given");
      return 3;
    }

    user=AB_Banking_FindUser(ab, EBC_PROVIDER_NAME,
			     "de",
			     lbankId, luserId, lcustomerId);
    if (user) {
      DBG_ERROR(0, "User %s already exists", luserId);
      return 3;
    }

    user=AB_Banking_CreateUser(ab, EBC_PROVIDER_NAME);
    assert(user);

    AB_User_SetCountry(user, "de");
    AB_User_SetBankCode(user, lbankId);
    AB_User_SetUserId(user, luserId);
    AB_User_SetCustomerId(user, lcustomerId);
    EBC_User_SetPeerId(user, hostName);
    AB_User_SetUserName(user, userName);
    EBC_User_SetTokenType(user, tokenType);
    EBC_User_SetTokenName(user, tokenName);
    EBC_User_SetTokenContextId(user, cid);
    if (ebicsVersion) {
      if (strcasecmp(ebicsVersion, "H002")==0) {
	EBC_User_SetProtoVersion(user, "H002");
	EBC_User_SetSignVersion(user, "A004");
	EBC_User_SetAuthVersion(user, "X001");
	EBC_User_SetCryptVersion(user, "E001");
      }
      else if (strcasecmp(ebicsVersion, "H003")==0) {
	EBC_User_SetProtoVersion(user, "H003");
	EBC_User_SetSignVersion(user, "A005");
	EBC_User_SetAuthVersion(user, "X002");
	EBC_User_SetCryptVersion(user, "E002");
      }
      else {
	fprintf(stderr, "%s",
		I18N("Invalid protocol version.\n"
		     "Possible versions are H002 and H003.\n"));
        return 3;
      }
    }

    /* try to get server address from database if still unknown */
    if (!lserverAddr || *lserverAddr==0) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (getBankUrl(ab,
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

    GWEN_Url_SetProtocol(url, "https");
    if (GWEN_Url_GetPort(url)==0)
      GWEN_Url_SetPort(url, 443);

    /* set url */
    if (1) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=GWEN_Url_toString(url, tbuf);
      if (rv<0) {
        DBG_ERROR(0, "Internal error storing URL");
        return 3;
      }
      EBC_User_SetServerUrl(user, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }
    GWEN_Url_free(url);

    if (importing) {
      EBC_User_AddFlags(user, EBC_USER_FLAGS_INI | EBC_USER_FLAGS_HIA);
      EBC_User_SetStatus(user, EBC_UserStatus_Enabled);
    }

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




