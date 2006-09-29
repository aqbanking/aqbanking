/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: iniletter.c 964 2006-03-17 10:35:21Z cstim $
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
#include <gwenhywfar/md.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int test1(AB_BANKING *ab,
          GWEN_DB_NODE *dbArgs,
          int argc,
          char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  AB_USER_LIST2 *ul;
  AB_USER *u=0;
  int rv;
  const char *bankId;
  const char *userId;
  const char *customerId;
  const char *keyFile;
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
    GWEN_ArgsTypeChar,            /* type */
    "keyFile",                    /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "k",                          /* short option */
    "keyfile",                    /* long option */
    "Specify the key file to be read",    /* short description */
    "Specify the key file to read the DES key from"  /* long description */
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

  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, "*");
  userId=GWEN_DB_GetCharValue(db, "userId", 0, "*");
  customerId=GWEN_DB_GetCharValue(db, "customerId", 0, "*");

  keyFile=GWEN_DB_GetCharValue(db, "keyFile", 0, 0);

  ul=AB_Banking_FindUsers(ab, AH_PROVIDER_NAME,
                          "de", bankId, userId, customerId);
  if (ul) {
    if (AB_User_List2_GetSize(ul)!=1) {
      DBG_ERROR(0, "Ambiguous customer specification");
      AB_Banking_Fini(ab);
      return 3;
    }
    else {
      AB_USER_LIST2_ITERATOR *cit;

      cit=AB_User_List2_First(ul);
      assert(cit);
      u=AB_User_List2Iterator_Data(cit);
      AB_User_List2Iterator_free(cit);
    }
    AB_User_List2_free(ul);
  }
  if (!u) {
    DBG_ERROR(0, "No matching customer");
    AB_Banking_Fini(ab);
    return 3;
  }
  else {
    AH_MEDIUM *m;
    GWEN_CRYPTKEY *skey;
    GWEN_DB_NODE *dbKey;
    GWEN_BUFFER *msgBuf;
    GWEN_BUFFER *encryptBuf;

    m=AH_User_GetMedium(u);
    assert(m);

    rv=AH_Medium_Mount(m);
    if (rv) {
      DBG_ERROR(0, "Could not mount medium (%d)", rv);
      AB_Banking_Fini(ab);
      return 3;
    }

    rv=AH_Medium_SelectContext(m, AH_User_GetContextIdx(u));
    if (rv) {
      DBG_ERROR(0, "Could not select context %d (%d)",
                AH_User_GetContextIdx(u), rv);
      AB_Banking_Fini(ab);
      return 3;
    }

    dbKey=GWEN_DB_Group_new("keydata");
    if (GWEN_DB_ReadFile(dbKey,
                         keyFile,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_PATH_FLAGS_CREATE_GROUP)) {
      DBG_ERROR(0, "Could not read DES key file \"%s\"", keyFile);
      AB_Banking_Fini(ab);
      return 3;
    }

    skey=GWEN_CryptKey_fromDb(dbKey);
    if (!skey) {
      DBG_ERROR(0, "File \"%s\" does not contain a DES key", keyFile);
      AB_Banking_Fini(ab);
      return 3;
    }

    msgBuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(msgBuf, "This is a little test string.\n");
    encryptBuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=AH_Medium_EncryptWithKey(m, msgBuf, encryptBuf, skey);
    if (rv) {
      DBG_ERROR(0, "Could not encrypt msg (%d)", rv);
      AB_Banking_Fini(ab);
      return 3;
    }

    GWEN_Buffer_Dump(encryptBuf, stderr, 2);
    GWEN_Buffer_free(encryptBuf);
    GWEN_Buffer_free(msgBuf);
    GWEN_DB_Group_free(dbKey);
    GWEN_CryptKey_free(skey);
  }


  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}


int test2(AB_BANKING *ab,
          GWEN_DB_NODE *dbArgs,
          int argc,
          char **argv) {
#if 0  /* disabled because it uses internal functions */
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  AB_USER_LIST2 *ul;
  AB_USER *u=0;
  int rv;
  const char *bankId;
  const char *userId;
  const char *customerId;
  const char *keyFile;
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
    GWEN_ArgsTypeChar,            /* type */
    "keyFile",                    /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "k",                          /* short option */
    "keyfile",                    /* long option */
    "Specify the key file to be read",    /* short description */
    "Specify the key file to read the DES key from"  /* long description */
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

  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, "*");
  userId=GWEN_DB_GetCharValue(db, "userId", 0, "*");
  customerId=GWEN_DB_GetCharValue(db, "customerId", 0, "*");

  keyFile=GWEN_DB_GetCharValue(db, "keyFile", 0, 0);

  ul=AB_Banking_FindUsers(ab, AH_PROVIDER_NAME,
                          "de", bankId, userId, customerId);
  if (ul) {
    if (AB_User_List2_GetSize(ul)!=1) {
      DBG_ERROR(0, "Ambiguous customer specification");
      AB_Banking_Fini(ab);
      return 3;
    }
    else {
      AB_USER_LIST2_ITERATOR *cit;

      cit=AB_User_List2_First(ul);
      assert(cit);
      u=AB_User_List2Iterator_Data(cit);
      AB_User_List2Iterator_free(cit);
    }
    AB_User_List2_free(ul);
  }
  if (!u) {
    DBG_ERROR(0, "No matching customer");
    AB_Banking_Fini(ab);
    return 3;
  }
  else {
    AH_MEDIUM *m;
    GWEN_DB_NODE *dbKey;
    GWEN_BUFFER *msgBuf;
    GWEN_BUFFER *encryptBuf;
    unsigned int keyLen;
    const void *p;

    m=AH_User_GetMedium(u);
    assert(m);

    rv=AH_Medium_Mount(m);
    if (rv) {
      DBG_ERROR(0, "Could not mount medium (%d)", rv);
      AB_Banking_Fini(ab);
      return 3;
    }

    rv=AH_Medium_SelectContext(m, AH_User_GetContextIdx(u));
    if (rv) {
      DBG_ERROR(0, "Could not select context %d (%d)",
                AH_User_GetContextIdx(u), rv);
      AB_Banking_Fini(ab);
      return 3;
    }

    dbKey=GWEN_DB_Group_new("keydata");
    if (GWEN_DB_ReadFile(dbKey,
                         keyFile,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_PATH_FLAGS_CREATE_GROUP)) {
      DBG_ERROR(0, "Could not read DES key file \"%s\"", keyFile);
      AB_Banking_Fini(ab);
      return 3;
    }

    p=GWEN_DB_GetBinValue(dbKey, "data/keyData", 0,
                          0, 0, &keyLen);
    if (!p || !keyLen) {
      DBG_ERROR(0, "File \"%s\" does not contain a DES key", keyFile);
      AB_Banking_Fini(ab);
      return 3;
    }

    msgBuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendBytes(msgBuf, (const char*)p, keyLen);
    encryptBuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=AH_Medium_EncryptKey(m, msgBuf, encryptBuf);
    if (rv) {
      DBG_ERROR(0, "Could not encrypt msg (%d)", rv);
      AB_Banking_Fini(ab);
      return 3;
    }

    GWEN_Buffer_Dump(encryptBuf, stderr, 2);
    GWEN_Buffer_free(encryptBuf);
    GWEN_Buffer_free(msgBuf);
    GWEN_DB_Group_free(dbKey);
  }


  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }
#endif

  return 0;
}









