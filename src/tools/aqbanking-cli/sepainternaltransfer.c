/***************************************************************************
 begin       : Tue Oct 12 2021
 copyright   : (C) 2021 by Stefan Bayer, Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* tool includes */
#include "globals.h"

/* aqbanking includes */
#include <aqbanking/types/transaction.h>

/* gwenhywfar includes */
#include <gwenhywfar/text.h>

/* forward declarations */
#define ACC_CHOOSER_INPUT_SIZE 10
static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc,
                                      char **argv);
static AB_REFERENCE_ACCOUNT *_chooseReferenceAccount(
  AB_REFERENCE_ACCOUNT_LIST *ral);

int sepaInternalTransfer(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc,
                         char **argv)
{
  GWEN_DB_NODE *db;
  AB_ACCOUNT_SPEC *as;

  int rv;
  int rvExec = 0;
  const char *ctxFile;
  AB_TRANSACTION *t;
  AB_REFERENCE_ACCOUNT_LIST *ral;
  int noCheck;

  /* parse command line arguments */
  db = _readCommandLine(dbArgs, argc, argv);
  if (db == NULL) {
    /* error in command line */
    return 1;
  }

  /* read arguments */
  noCheck = GWEN_DB_GetIntValue(db, "noCheck", 0, 0);
  ctxFile = GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);

  /* init AqBanking */
  rv = AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  /* get account to work with */
  as = getSingleSelectedAccount(ab, db);
  if (as == NULL) {
    AB_Banking_Fini(ab);
    return 2;
  }

  /* get the target account */
  ral = AB_AccountSpec_GetRefAccountList(as);

  /* transfer arguments */
  if (AB_ReferenceAccount_List_GetCount(ral) == 0) {
    DBG_ERROR(0,
              "No reference accounts defined, maybe you need to run 'aqhbci-tool4 gettargetacc'");
    AB_Banking_Fini(ab);
    return 2;
  }
  else {
    AB_REFERENCE_ACCOUNT *ra;
    const char *iban = GWEN_DB_GetCharValue(db, "remoteIBAN", 0, 0);
    const char *refAccountName = GWEN_DB_GetCharValue(db, "remoteAccountName", 0, 0);
    const char *ownerName;

    ra = NULL;
    if (iban != NULL && refAccountName != NULL) {
      ra = AB_ReferenceAccount_List_FindFirst(ral, iban, NULL, NULL, NULL, NULL,
                                              NULL, NULL, refAccountName);
    }
    if (ra == NULL) {
      ra = _chooseReferenceAccount(ral);
      if (ra == NULL) {
        DBG_ERROR(0, "No reference account chosen, abort");
        AB_Banking_Fini(ab);
        return 2;
      }
    }
    /* transfer reference account info to the argument db */
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteIban",
                         AB_ReferenceAccount_GetIban(ra));
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteBic",
                         AB_ReferenceAccount_GetBic(ra));
    ownerName=GWEN_DB_GetCharValue(db, "remoteName", 0, NULL);
    if (!(ownerName && *ownerName))
      ownerName=AB_ReferenceAccount_GetOwnerName(ra);
    if (!(ownerName && *ownerName))
      ownerName=AB_AccountSpec_GetOwnerName(as);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteName", ownerName);
  }

  /* create transaction from arguments */
  t = mkSepaTransfer(db, AB_Transaction_CommandSepaInternalTransfer);
  if (t == NULL) {
    DBG_ERROR(0, "Could not create SEPA internal transaction from arguments");
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return 2;
  }
  AB_Transaction_SetUniqueAccountId(t, AB_AccountSpec_GetUniqueId(as));

  /* check for date; if given create a dated transfer */
  /*if (AB_Transaction_GetDate(t))
   AB_Transaction_SetCommand(t, AB_Transaction_CommandCreateDatedTransfer);*/

  /* set local account info from selected AB_ACCOUNT_SPEC */
  AB_Banking_FillTransactionFromAccountSpec(t, as);

  /* some checks */
  rv = checkTransactionIbans(t);
  if (rv != 0) {
    AB_Transaction_free(t);
    AB_AccountSpec_free(as);
    AB_Banking_Fini(ab);
    return rv;
  }

  /* probably check against transaction limits */
  if (!noCheck) {
    rv = checkTransactionLimits(t,
                                AB_AccountSpec_GetTransactionLimitsForCommand(as,
                                                                              AB_Transaction_GetCommand(t)),
                                AQBANKING_TOOL_LIMITFLAGS_PURPOSE |
                                AQBANKING_TOOL_LIMITFLAGS_NAMES |
                                AQBANKING_TOOL_LIMITFLAGS_DATE |
                                AQBANKING_TOOL_LIMITFLAGS_SEPA);
    if (rv != 0) {
      AB_Transaction_free(t);
      AB_AccountSpec_free(as);
      AB_Banking_Fini(ab);
      return rv;
    }
  }
  AB_AccountSpec_free(as);

  /* execute job */
  rv = execSingleBankingJob(ab, t, ctxFile);
  if (rv) {
    fprintf(stderr, "Error on executeQueue (%d)\n", rv);
    rvExec = rv;
  }

  /* cleanup */
  AB_Transaction_free(t);

  /* that's it */
  rv = AB_Banking_Fini(ab);
  if (rv < 0) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    if (rvExec == 0)
      rvExec = 5;
  }

  return rvExec;
}

/* parse command line */
GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[] = { {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "ctxFile", /* name */
      0, /* minnum */
      1, /* maxnum */
      "c", /* short option */
      "ctxfile", /* long option */
      "Specify the file to store the context in", /* short description */
      "Specify the file to store the context in" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int, /* type */
      "uniqueAccountId", /* name */
      0, /* minnum */
      1, /* maxnum */
      NULL, /* short option */
      "aid", /* long option */
      "Specify the unique account id", /* short description */
      "Specify the unique account id" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "backendName", /* name */
      0, /* minnum */
      1, /* maxnum */
      NULL, /* short option */
      "backend", /* long option */
      "Specify the name of the backend for your account", /* short description */
      "Specify the name of the backend for your account" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "country", /* name */
      0, /* minnum */
      1, /* maxnum */
      NULL, /* short option */
      "country", /* long option */
      "Specify the country for your account (e.g. \"de\")", /* short description */
      "Specify the country for your account (e.g. \"de\")" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "accountId", /* name */
      0, /* minnum */
      1, /* maxnum */
      "a", /* short option */
      "account", /* long option */
      "overwrite the account number", /* short description */
      "overwrite the account number" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "remoteAccountName", /* name */
      0, /* minnum */
      1, /* maxnum */
      NULL, /* short option */
      "raccname", /* long option */
      "specify the reference account name", /* short description */
      "specify the reference account number" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "subAccountId", /* name */
      0, /* minnum */
      1, /* maxnum */
      "aa", /* short option */
      "subaccount", /* long option */
      "Specify the sub account id (Unterkontomerkmal)", /* short description */
      "Specify the sub account id (Unterkontomerkmal)" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "accountType", /* name */
      0, /* minnum */
      1, /* maxnum */
      "at", /* short option */
      "accounttype", /* long option */
      "Specify the account type", /* short description */
      "Specify the account type" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "iban", /* name */
      0, /* minnum */
      1, /* maxnum */
      "A", /* short option */
      "iban", /* long option */
      "Specify the iban of your account", /* short description */
      "Specify the iban of your account" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "remoteIBAN", /* name */
      0, /* minnum */
      1, /* maxnum */
      0, /* short option */
      "riban", /* long option */
      "Specify the remote IBAN", /* short description */
      "Specify the remote IBAN" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "value", /* name */
      1, /* minnum */
      1, /* maxnum */
      "v", /* short option */
      "value", /* long option */
      "Specify the transfer amount", /* short description */
      "Specify the transfer amount" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "name", /* name */
      0, /* minnum */
      1, /* maxnum */
      0, /* short option */
      "name", /* long option */
      "Specify your name", /* short description */
      "Specify your name" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "purpose", /* name */
      1, /* minnum */
      6, /* maxnum */
      "p", /* short option */
      "purpose", /* long option */
      "Specify the purpose", /* short description */
      "Specify the purpose" /* long description */
    }, {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char, /* type */
      "endToEndReference", /* name */
      0, /* minnum */
      1, /* maxnum */
      "E", /* short option */
      "endtoendid", /* long option */
      "Specify the SEPA End-to-end-reference", /* short description */
      "Specify the SEPA End-to-end-reference" /* long description */
    }, {
      0, /* flags */
      GWEN_ArgsType_Int, /* type */
      "noCheck", /* name */
      0, /* minnum */
      1, /* maxnum */
      NULL, /* short option */
      "noCheck", /* long option */
      "Dont check transaction limits", /* short description */
      "Dont check transaction limits"
    }, {
      GWEN_ARGS_FLAGS_HELP
      | GWEN_ARGS_FLAGS_LAST, /* flags */
      GWEN_ArgsType_Int, /* type */
      "help", /* name */
      0, /* minnum */
      0, /* maxnum */
      "h", /* short option */
      "help", /* long option */
      "Show this help screen", /* short description */
      "Show this help screen" /* long description */
    }
  };

  db = GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "local");
  rv = GWEN_Args_Check(argc, argv, 1, 0 /*GWEN_ARGS_MODE_ALLOW_FREEPARAM*/,
                       args, db);
  if (rv == GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return NULL;
  }
  else if (rv == GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf = GWEN_Buffer_new(0, 1024, 0, 1);
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

AB_REFERENCE_ACCOUNT *_chooseReferenceAccount(AB_REFERENCE_ACCOUNT_LIST *ral)
{
  AB_REFERENCE_ACCOUNT *ra = NULL;
  GWEN_BUFFER *ubuf;
  int16_t numAccounts;
  char inputBuffer[ACC_CHOOSER_INPUT_SIZE];
  int chosenAccount;
  uint32_t flags;
  int16_t counter = 1;
  int16_t scan_result = EOF;
  const char *accName;
  const char *iban;
  ubuf = GWEN_Buffer_new(0, 8096, 0, 1);

  numAccounts = AB_ReferenceAccount_List_GetCount(ral);
  ra = AB_ReferenceAccount_List_First(ral);
  GWEN_Buffer_AppendString(ubuf, "0) abort\n");
  while (ra) {
    iban = AB_ReferenceAccount_GetIban(ra);
    accName = AB_ReferenceAccount_GetAccountName(ra);
    snprintf(inputBuffer, ACC_CHOOSER_INPUT_SIZE, "%d) ", counter++);
    GWEN_Buffer_AppendString(ubuf, inputBuffer);
    if (accName) {
      GWEN_Buffer_AppendString(ubuf, accName);
      GWEN_Buffer_AppendString(ubuf, ": ");
    }
    GWEN_Buffer_AppendString(ubuf, iban);
    GWEN_Buffer_AppendString(ubuf, "\n");

    ra = AB_ReferenceAccount_List_Next(ra);
  }

  flags = 0;
  ra = NULL;
  while (scan_result == EOF) {
    /*rv =*/ GWEN_Gui_InputBox(flags, I18N("Choose a reference account"),
                               GWEN_Buffer_GetStart(ubuf), inputBuffer, 0,
                               ACC_CHOOSER_INPUT_SIZE - 1, 0);

    scan_result = sscanf(inputBuffer, "%d", &chosenAccount);
    if (scan_result != EOF) {
      if (chosenAccount == 0) {
        break;
      }
      else if (chosenAccount > numAccounts) {
        scan_result=EOF;
      }
      else {
        ra = AB_ReferenceAccount_List_First(ral);
        for (counter = 1 ; counter < chosenAccount ; counter++) {
          ra = AB_ReferenceAccount_List_Next(ra);
        }
        break;
      }
    }
  }

  GWEN_Buffer_free(ubuf);

  return ra;

}

