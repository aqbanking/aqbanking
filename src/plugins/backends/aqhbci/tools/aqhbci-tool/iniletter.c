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

#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/md.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int iniLetter(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  AH_HBCI *hbci;
  AB_USER_LIST2 *ul;
  AB_USER *u=0;
  int rv;
  const char *bankId;
  const char *userId;
  const char *customerId;
  int bankKey;
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
    "Specify the customer id",    /* short description */
    "Specify the customer id"     /* long description */
  },
  {
    0, /* flags */
    GWEN_ArgsTypeInt,             /* type */
    "bankKey",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "B",                          /* short option */
    "bankkey",                    /* long option */
    "Show iniletter of bank keys",/* short description */
    "Show iniletter of bank keys" /* long description */
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

  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, "*");
  userId=GWEN_DB_GetCharValue(db, "userId", 0, "*");
  customerId=GWEN_DB_GetCharValue(db, "customerId", 0, "*");
  bankKey=GWEN_DB_VariableExists(db, "bankKey");

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
    GWEN_CRYPTKEY *key;
    GWEN_DB_NODE *dbKey;
    const void *p;
    unsigned int l;
    GWEN_BUFFER *bbuf;
    GWEN_BUFFER *lbuf;
    GWEN_BUFFER *keybuf;
    int i;
    GWEN_TIME *ti;
    char numbuf[32];
    char hashbuffer[21];

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

    if (bankKey) {
      key=AH_Medium_GetPubSignKey(m);
      if (!key)
        key=AH_Medium_GetPubCryptKey(m);
      if (!key) {
        AH_Medium_Unmount(m, 1);
        DBG_ERROR(0, "Server keys missing, please get them first");
        AB_Banking_Fini(ab);
        return 3;
      }
    }
    else {
      GWEN_CRYPTKEY *cryptKey;
      GWEN_CRYPTKEY *signKey;

      signKey=AH_Medium_GetLocalPubSignKey(m);
      cryptKey=AH_Medium_GetLocalPubCryptKey(m);
      if (!signKey || !cryptKey) {
        AH_Medium_Unmount(m, 1);
        DBG_ERROR(0, "Keys missing, please create them first");
        AB_Banking_Fini(ab);
        return 3;
      }
      key=signKey;
    }

    dbKey=GWEN_DB_Group_new("keydata");
    if (GWEN_CryptKey_toDb(key, dbKey, 1)) {
      GWEN_DB_Group_free(dbKey);
      DBG_ERROR(0, "Bad key.");
      AB_Banking_Fini(ab);
      return 3;
    }

    lbuf=GWEN_Buffer_new(0, 1024, 0, 1);
    keybuf=GWEN_Buffer_new(0, 257, 0, 1);

    /* prelude */
    GWEN_Buffer_AppendString(lbuf,
                             I18N("\n\n\nINI-Letter\n\n"));
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Date           : "));
    ti=GWEN_CurrentTime();
    assert(ti);
    GWEN_Time_toString(ti, I18N("YYYY/MM/DD"), lbuf);
    GWEN_Buffer_AppendString(lbuf, "\n");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Time           : "));
    GWEN_Time_toString(ti, I18N("hh:mm:ss"), lbuf);
    GWEN_Buffer_AppendString(lbuf, "\n");

    if (bankKey) {
      GWEN_Buffer_AppendString(lbuf,
                               I18N("Bank           : "));
      GWEN_Buffer_AppendString(lbuf, AB_User_GetBankCode(u));
    }
    else {
      GWEN_Buffer_AppendString(lbuf,
                               I18N("User           : "));
      GWEN_Buffer_AppendString(lbuf, AB_User_GetUserId(u));
    }

    GWEN_Buffer_AppendString(lbuf, "\n");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Key number     : "));
    snprintf(numbuf, sizeof(numbuf), "%d",
             GWEN_DB_GetIntValue(dbKey, "number", 0, 0));
    GWEN_Buffer_AppendString(lbuf, numbuf);
    GWEN_Buffer_AppendString(lbuf, "\n");

    GWEN_Buffer_AppendString(lbuf,
                             I18N("Key version    : "));
    snprintf(numbuf, sizeof(numbuf), "%d",
             GWEN_DB_GetIntValue(dbKey, "version", 0, 0));
    GWEN_Buffer_AppendString(lbuf, numbuf);
    GWEN_Buffer_AppendString(lbuf, "\n");

    if (!bankKey) {
      GWEN_Buffer_AppendString(lbuf,
                               I18N("Customer system: "));
      GWEN_Buffer_AppendString(lbuf, AH_HBCI_GetProductName(hbci));
      GWEN_Buffer_AppendString(lbuf, "\n");
    }

    GWEN_Buffer_AppendString(lbuf, "\n");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Public key for electronic signature"));
    GWEN_Buffer_AppendString(lbuf, "\n\n");

    GWEN_Buffer_AppendString(lbuf, "  ");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Exponent\n"));
    GWEN_Buffer_AppendString(lbuf, "\n");

    /* exponent */
    p=GWEN_DB_GetBinValue(dbKey,
                          "data/e",
                          0,
                          0,0,
                          &l);
    if (!p || !l) {
      GWEN_DB_Group_free(dbKey);
      GWEN_Buffer_free(lbuf);
      DBG_ERROR(0, "Bad key.");
      AB_Banking_Fini(ab);
      return 3;
    }

    bbuf=GWEN_Buffer_new(0, 97, 0, 1);
    GWEN_Buffer_AppendBytes(bbuf, p, l);
    GWEN_Buffer_Rewind(bbuf);
    if (l<96)
      GWEN_Buffer_FillLeftWithBytes(bbuf, 0, 96-l);
    p=GWEN_Buffer_GetStart(bbuf);
    l=GWEN_Buffer_GetUsedBytes(bbuf);
    for (i=0; i<6; i++) {
      GWEN_Buffer_AppendString(lbuf, "  ");
      if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
        DBG_ERROR(0, "Error converting to hex??");
        abort();
      }
      p+=16;
      GWEN_Buffer_AppendString(lbuf, "\n");
    }

    GWEN_Buffer_FillWithBytes(keybuf, 0, 128-l);
    GWEN_Buffer_AppendBuffer(keybuf, bbuf);
    GWEN_Buffer_free(bbuf);

    /* modulus */
    GWEN_Buffer_AppendString(lbuf, "\n");
    GWEN_Buffer_AppendString(lbuf, "  ");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Modulus\n"));
    GWEN_Buffer_AppendString(lbuf, "\n");
    p=GWEN_DB_GetBinValue(dbKey,
                          "data/n",
                          0,
                          0,0,
                          &l);
    if (!p || !l) {
      GWEN_Buffer_free(lbuf);
      GWEN_DB_Group_free(dbKey);
      DBG_ERROR(0, "Bad key.");
      AB_Banking_Fini(ab);
      return 3;
    }

    bbuf=GWEN_Buffer_new(0, 97, 0, 1);
    GWEN_Buffer_AppendBytes(bbuf, p, l);
    GWEN_Buffer_Rewind(bbuf);
    if (l<96)
      GWEN_Buffer_FillLeftWithBytes(bbuf, 0, 96-l);
    p=GWEN_Buffer_GetStart(bbuf);
    l=GWEN_Buffer_GetUsedBytes(bbuf);
    for (i=0; i<6; i++) {
      GWEN_Buffer_AppendString(lbuf, "  ");
      if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
        DBG_ERROR(0, "Error converting to hex??");
        abort();
      }
      p+=16;
      GWEN_Buffer_AppendString(lbuf, "\n");
    }

    GWEN_Buffer_FillWithBytes(keybuf, 0, 128-l);
    GWEN_Buffer_AppendBuffer(keybuf, bbuf);
    GWEN_Buffer_free(bbuf);

    GWEN_Buffer_AppendString(lbuf, "\n");
    GWEN_Buffer_AppendString(lbuf, "  ");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Hash\n"));
    GWEN_Buffer_AppendString(lbuf, "\n");
    l=20;
    if (GWEN_MD_Hash("RMD160",
                     GWEN_Buffer_GetStart(keybuf),
                     GWEN_Buffer_GetUsedBytes(keybuf),
                     hashbuffer,
                     &l)) {
      DBG_ERROR(0, "Could not hash");
      abort();
    }
    GWEN_Buffer_free(keybuf);

    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(hashbuffer, 20, lbuf, 2, ' ', 0)) {
      DBG_ERROR(0, "Error converting to hex??");
      abort();
    }
    GWEN_Buffer_AppendString(lbuf, "\n");

    if (!bankKey) {
      GWEN_Buffer_AppendString(lbuf, "\n\n");
      GWEN_Buffer_AppendString(lbuf,
                               I18N("I confirm that I found the above key "
                                    "for my electronic signature.\n"));
      GWEN_Buffer_AppendString(lbuf, "\n\n");
      GWEN_Buffer_AppendString(lbuf,
                               I18N("____________________________  "
                                    "____________________________\n"
                                    "Place, date                   "
                                    "Signature\n"));
    }

    fprintf(stdout, "%s\n",
            GWEN_Buffer_GetStart(lbuf));
    GWEN_Buffer_free(lbuf);
  }


  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}




