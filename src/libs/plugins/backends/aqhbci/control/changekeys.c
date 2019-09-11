/***************************************************************************

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals_l.h"
#include "aqhbci/banking/user.h"

#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/ctplugin.h>

#include <stdarg.h>
#include <unistd.h>


int AH_Control_ChangeKeys(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db = NULL;
  int res = 0;
  uint32_t uid = 0;
  AB_USER *u = NULL;

  // flags, type, name, minnum, maxnum, short option, long option, short description , long description
  const GWEN_ARGS args[] = {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, GWEN_ArgsType_Int, "userId", 1, 1,
      "u", "user", "user id", "user id"
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, GWEN_ArgsType_Char, "tokenType", 0, 1,
      "t", "tokentype", "token type", "type of new crypt token (file, card)"
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, GWEN_ArgsType_Char, "tokenName", 0, 1,
      "n", "tokenname", "token name", "name of new crypt token (filename, card-nr)"
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, GWEN_ArgsType_Int, "context", 0, 1,
      "c", "context", "token context", "context on new token"
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, GWEN_ArgsType_Char, "cryptMode", 0, 1,
      "m", "cryptmode", "crypt mode", "crypt mode of new token (RDH, RAH)"
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, GWEN_ArgsType_Int, "cryptType", 0, 1,
      "T", "crypttype", "crypt type", "crypt type of new token (1 - 10, depends on crypt mode)"
    },
    {
      GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, GWEN_ArgsType_Int, "help",  0, 0,
      "h", "help", "Show this help screen", "Show this help screen"
    }
  };

  db = GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "local");
  if (!db || ((res = GWEN_Args_Check(argc, argv, 1, 0, args, db)) == GWEN_ARGS_RESULT_ERROR)) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return 1;
  }

  if (res == GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *b = GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, b, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    // TODO: help text default args
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(b));
    GWEN_Buffer_free(b);
    return 0;
  }

  if ((uid = (uint32_t)GWEN_DB_GetIntValue(db, "userId", 0, 0)) == 0) {
    fprintf(stderr, "ERROR: Invalid or missing unique user id\n");
    return 1;
  }

  if ((AB_Provider_HasUser(pro, uid) < 0) || (AB_Provider_GetUser(pro, uid, 1, 1, &u) < 0) || !u) {
    fprintf(stderr, "ERROR: could not find user with id %ld.", (long)uid);
    return -1;
  }

  res = AH_Provider_ChangeUserKeys(pro, u, db, 1, 0, 0);

  AB_User_free(u);
  return res;
}
