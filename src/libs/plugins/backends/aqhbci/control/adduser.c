/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals_l.h"
#include "aqhbci/banking/user.h"
#include "aqhbci/banking/provider.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/ct.h>
#include <gwenhywfar/ctplugin.h>



static GWEN_DB_NODE *readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);
/*static int getBankUrl(AB_BANKING *ab, AH_CRYPT_MODE cryptMode, const char *bankId, GWEN_BUFFER *bufServer);*/
static int checkRdhType(int rdhType, int cryptModeRAH);
static int checkHbciVersion(int hbciVersion);
static int fillFromToken(AB_USER *user, const char *tokenType, const char *tokenName, uint32_t cid, int cryptModeRAH);
static int fillUserDataFromContext(AB_USER *user, const GWEN_CRYPT_TOKEN_CONTEXT *ctx);
static int determineCryptMode(AB_USER *user, GWEN_CRYPT_TOKEN *ct, const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
                              int cryptModeRAH);
static int finishUser(AB_USER *user);




int AH_Control_AddUser(AB_PROVIDER *pro,
                       GWEN_DB_NODE *dbArgs,
                       int argc,
                       char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  AB_USER *user=NULL;
  const char *tokenName;
  const char *tokenType;
  const char *bankId;
  const char *userId;
  const char *customerId;
  const char *server;
  const char *userName;
  int hbciVersion;
  int rdhType;
  int cryptModeRAH=0;
  uint32_t cid;

  /* parse command line */
  db=readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return 1;
  }

  tokenType=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
  tokenName=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);
  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  userId=GWEN_DB_GetCharValue(db, "userId", 0, 0);
  customerId=GWEN_DB_GetCharValue(db, "customerId", 0, 0);
  server=GWEN_DB_GetCharValue(db, "serverAddr", 0, 0);
  cid=GWEN_DB_GetIntValue(db, "context", 0, 1);
  hbciVersion=GWEN_DB_GetIntValue(db, "hbciVersion", 0, 0);
  rdhType=GWEN_DB_GetIntValue(db, "rdhType", 0, 1);
  cryptModeRAH=(NULL!=GWEN_DB_FindFirstVar(db, "cryptModeRAH"))?1:0;
  userName=GWEN_DB_GetCharValue(db, "userName", 0, 0);
  assert(userName);

  /* generic check for some arguments */
  if (hbciVersion>0 && rdhType>1) {
    if (hbciVersion<300 && rdhType>1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "RDH Types 2 and above only work with HBCI version 300 or later");
      return 1;
    }
  }

  if (checkHbciVersion(hbciVersion))
    return 1;

  if (checkRdhType(rdhType, cryptModeRAH))
    return 1;


  /* create user */
  user=AB_Provider_CreateUserObject(pro);
  assert(user);

  /* setup user data from input so far */
  AB_User_SetUserName(user, userName);
  AB_User_SetCountry(user, "de");
  AB_User_SetBankCode(user, bankId);
  AB_User_SetUserId(user, userId);
  AB_User_SetCustomerId(user, customerId?customerId:userId);
  AH_User_SetTokenType(user, tokenType);
  AH_User_SetTokenName(user, tokenName);
  AH_User_SetTokenContextId(user, cid);
  AH_User_SetHbciVersion(user, hbciVersion);
  if (rdhType>0)
    AH_User_SetRdhType(user, rdhType);
  if (server && *server) {
    GWEN_URL *url;

    url=GWEN_Url_fromString(server);
    if (url==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad URL \"%s\"", server);
      AB_User_free(user);
      return 3;
    }
    AH_User_SetServerUrl(user, url);
    GWEN_Url_free(url);
  }

  /* fill from CryptToken, if necessary */
  if (strcasecmp(tokenType, "pintan")==0) {
    AH_User_SetCryptMode(user, AH_CryptMode_Pintan);
  }
  else {
    int rv;

    rv=fillFromToken(user, tokenType, tokenName, cid, cryptModeRAH);
    if (rv)
      return rv;
  }

  /* check settings for user before adding user */
  rv=finishUser(user);
  if (rv)
    return rv;

  if (AH_User_GetCryptMode(user)==AH_CryptMode_Ddv)
    AH_User_SetStatus(user, AH_UserStatusEnabled);

  /* add user */
  rv=AB_Provider_AddUser(pro, user);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add new user (%d)", rv);
    AB_User_free(user);
    return 4;
  }

  /* cleanup */
  AB_User_free(user);

  return 0;
}



GWEN_DB_NODE *readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]= {
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
      "Select the RDH profile type (1, 2, 3, 5, 7, 9, 10)",
      "Select the RDH profile type (1, 2, 3, 5, 7, 9, 10)"
    },
    {
      0,
      GWEN_ArgsType_Int,
      "cryptModeRAH",
      0,
      1,
      0,
      "cryptmoderah",
      "Selects RAH instead of RDH crypt mode",
      "Selects RAH instead of RDH crypt mode"
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
    return NULL;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return NULL;
    }
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return NULL;
  }

  return db;
}



#if 0
int getBankUrl(AB_BANKING *ab,
               AH_CRYPT_MODE cryptMode,
               const char *bankId,
               GWEN_BUFFER *bufServer)
{
  AB_BANKINFO *bi;

  bi=AB_Banking_GetBankInfo(ab, "de", 0, bankId);
  if (bi) {
    AB_BANKINFO_SERVICE_LIST *l;
    AB_BANKINFO_SERVICE *sv;

    l=AB_BankInfo_GetServices(bi);
    assert(l);
    sv=AB_BankInfoService_List_First(l);
    while (sv) {
      const char *st;

      st=AB_BankInfoService_GetType(sv);
      if (st && *st && strcasecmp(st, "hbci")==0) {
        const char *svm;

        svm=AB_BankInfoService_GetMode(sv);
        if (svm && *svm) {
          if (!
              ((strcasecmp(svm, "pintan")==0) ^
               (cryptMode==AH_CryptMode_Pintan))) {
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
#endif



int checkRdhType(int rdhType, int cryptModeRAH)
{
  if (rdhType>0) {
    if (cryptModeRAH) {
      switch (rdhType) {
      case 7:
      case 9:
      case 10:
        /* supported */
        break;

      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 8:
      default:
        DBG_ERROR(AQHBCI_LOGDOMAIN, "RAH type %d not supported", rdhType);
        return 1;
      }
    }
    else {
      switch (rdhType) {
      case 1:
      case 2:
      case 3:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
        /* supported */
        break;

      case 4:
      default:
        DBG_ERROR(AQHBCI_LOGDOMAIN, "RDH type %d not supported", rdhType);
        return 1;
      }
    }
  }

  return 0;
}



int checkHbciVersion(int hbciVersion)
{
  if (hbciVersion>0) {
    switch (hbciVersion) {
    case 201:
    case 210:
    case 220:
    case 300:
      /* supported */
      break;

    default:
      DBG_ERROR(AQHBCI_LOGDOMAIN, "HBCI/FinTS version %d not supported", hbciVersion);
      return 1;
    }
  }

  return 0;
}



int fillFromToken(AB_USER *user, const char *tokenType, const char *tokenName, uint32_t cid, int cryptModeRAH)
{
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *cryptTokenContext;
  int rv;

  if (cid==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No context given.");
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
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create crypt token");
    return 3;
  }

  /* open crypt token */
  rv=GWEN_Crypt_Token_Open(ct, 0, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not open token (%d)", rv);
    GWEN_Crypt_Token_free(ct);
    return 3;
  }

  /* get real token name */
  if (AH_User_GetTokenName(user)==NULL)
    AH_User_SetTokenName(user, GWEN_Crypt_Token_GetTokenName(ct));

  cryptTokenContext=GWEN_Crypt_Token_GetContext(ct, cid, 0);
  if (cryptTokenContext==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Context %02x not found", cid);
    GWEN_Crypt_Token_Close(ct, 1, 0);
    GWEN_Crypt_Token_free(ct);
    return 3;
  }

  rv=fillUserDataFromContext(user, cryptTokenContext);
  if (rv) {
    GWEN_Crypt_Token_Close(ct, 1, 0);
    GWEN_Crypt_Token_free(ct);
    return rv;
  }

  rv=determineCryptMode(user, ct, cryptTokenContext, cryptModeRAH);
  if (rv) {
    GWEN_Crypt_Token_Close(ct, 1, 0);
    GWEN_Crypt_Token_free(ct);
    return rv;
  }

  /* adapt RDH mode if needed */
  if (AH_User_GetCryptMode(user)==AH_CryptMode_Rdh) {
    int rdhType;

    rdhType=AH_User_GetRdhType(user);
    if (rdhType>1 && rdhType!=GWEN_Crypt_Token_Context_GetProtocolVersion(cryptTokenContext)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Specified RDH version %d differs from RDH version %d on card!",
                rdhType, GWEN_Crypt_Token_Context_GetProtocolVersion(cryptTokenContext));
      return 3;
    }
    else {
      AH_User_SetRdhType(user, GWEN_Crypt_Token_Context_GetProtocolVersion(cryptTokenContext));
    }
  } /* if RDH */


  rv=GWEN_Crypt_Token_Close(ct, 0, 0);
  GWEN_Crypt_Token_free(ct);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not close token (%d)", rv);
    return 3;
  }

  /* done */
  return 0;
}



int fillUserDataFromContext(AB_USER *user, const GWEN_CRYPT_TOKEN_CONTEXT *ctx)
{
  /* fill missing information from CryptToken */
  if (AB_User_GetBankCode(user)==NULL)
    AB_User_SetBankCode(user, GWEN_Crypt_Token_Context_GetServiceId(ctx));

  if (AB_User_GetUserId(user)==NULL)
    AB_User_SetUserId(user, GWEN_Crypt_Token_Context_GetUserId(ctx));

  if (AB_User_GetCustomerId(user)==NULL)
    AB_User_SetCustomerId(user, GWEN_Crypt_Token_Context_GetCustomerId(ctx));

  if (AH_User_GetServerUrl(user)==NULL) {
    const char *sUrl;

    sUrl=GWEN_Crypt_Token_Context_GetAddress(ctx);
    if (sUrl && *sUrl) {
      GWEN_URL *url;

      url=GWEN_Url_fromString(sUrl);
      if (url==NULL) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad URL \"%s\" on crypt token", sUrl);
        return 3;
      }
      AH_User_SetServerUrl(user, url);
      GWEN_Url_free(url);
    }
  }
  return 0;
}


int determineCryptMode(AB_USER *user, GWEN_CRYPT_TOKEN *ct, const GWEN_CRYPT_TOKEN_CONTEXT *ctx, int cryptModeRAH)
{
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  GWEN_CRYPT_CRYPTALGOID algo;

  /* determine crypt mode */
  keyId=GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
  if (keyId==0)
    keyId=GWEN_Crypt_Token_Context_GetVerifyKeyId(ctx);
  if (keyId==0)
    keyId=GWEN_Crypt_Token_Context_GetEncipherKeyId(ctx);
  if (keyId==0)
    keyId=GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx);
  if (keyId==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No keys, unable to determine crypt mode");
    return 3;
  }

  ki=GWEN_Crypt_Token_GetKeyInfo(ct, keyId, 0xffffffff, 0);
  if (ki==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not get keyinfo for key %d, unable to determine crypt mode", keyId);
    return 3;
  }

  algo=GWEN_Crypt_Token_KeyInfo_GetCryptAlgoId(ki);
  if (algo==GWEN_Crypt_CryptAlgoId_Des3K)
    AH_User_SetCryptMode(user, AH_CryptMode_Ddv);
  else if (algo==GWEN_Crypt_CryptAlgoId_Rsa) {
    if (cryptModeRAH)
      AH_User_SetCryptMode(user, AH_CryptMode_Rah);
    else
      AH_User_SetCryptMode(user, AH_CryptMode_Rdh);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Unexpected crypt algorithm \"%s\", unable to determine crypt mode",
              GWEN_Crypt_CryptAlgoId_toString(algo));
    return 3;
  }

  return 0;
}



int finishUser(AB_USER *user)
{
  const char *s;

  s=AB_User_GetUserId(user);
  if (!(s && *s)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing user id");
    return 3;
  }

  s=AB_User_GetCustomerId(user);
  if (!(s && *s)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing customer id");
    return 3;
  }

  s=AB_User_GetBankCode(user);
  if (!(s && *s)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing bank code");
    return 3;
  }

  /* setup URL */
  if (AH_User_GetServerUrl(user)) {
    GWEN_URL *url;

    url=GWEN_Url_dup(AH_User_GetServerUrl(user));
    if (GWEN_Url_GetProtocol(url)==NULL) {
      if (AH_User_GetCryptMode(user)==AH_CryptMode_Pintan) {
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
    }
    GWEN_Url_free(url);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing URL");
    return 3;
  }

  /* set default HBCI/FinTS version */
  if (AH_User_GetHbciVersion(user)==0)
    AH_User_SetHbciVersion(user, 300);

  return 0;
}



