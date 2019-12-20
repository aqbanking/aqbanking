/***************************************************************************
 begin       : Sat Oct 26 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_p.h"

#include "aqfints.h"
#include "control/control.h"

#include <aqbanking/backendsupport/provider_be.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _provider_FreeData(void *bp, void *p);
static int GWENHYWFAR_CB _provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int GWENHYWFAR_CB _provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */




GWEN_INHERIT(AB_PROVIDER, AF_PROVIDER);




AB_PROVIDER *AF_Provider_new(AB_BANKING *ab)
{
  AB_PROVIDER *pro;
  AF_PROVIDER *xpro;

  pro=AB_Provider_new(ab, AF_PROVIDER_NAME);
  assert(pro);

  AB_Provider_SetInitFn(pro, _provider_Init);
  AB_Provider_SetFiniFn(pro, _provider_Fini);
  AB_Provider_SetControlFn(pro, AF_Control);

#if 0
  AB_Provider_SetGetNewUserDialogFn(pro, AH_Provider_GetNewUserDialog);
  AB_Provider_SetGetEditUserDialogFn(pro, AH_Provider_GetEditUserDialog);
  AB_Provider_SetGetUserTypeDialogFn(pro, AH_Provider_GetUserTypeDialog);
  AB_Provider_SetGetEditAccountDialogFn(pro, AH_Provider_GetEditAccountDialog);

  AB_Provider_SetSendCommandsFn(pro, AH_Provider_SendCommands);
  AB_Provider_SetCreateAccountObjectsFn(pro, AH_Provider_CreateAccountObject);
  AB_Provider_SetCreateUserObjectsFn(pro, AH_Provider_CreateUserObject);

  AB_Provider_SetUpdateAccountSpecFn(pro, AH_Provider_UpdateAccountSpec);

  AB_Provider_AddFlags(pro,
                       AB_PROVIDER_FLAGS_HAS_NEWUSER_DIALOG |
                       AB_PROVIDER_FLAGS_HAS_EDITUSER_DIALOG |
                       AB_PROVIDER_FLAGS_HAS_EDITACCOUNT_DIALOG |
                       AB_PROVIDER_FLAGS_HAS_USERTYPE_DIALOG);
#endif

  GWEN_NEW_OBJECT(AF_PROVIDER, xpro);
  GWEN_INHERIT_SETDATA(AB_PROVIDER, AF_PROVIDER, pro, xpro, _provider_FreeData);


  return pro;
}



void _provider_FreeData(GWEN_UNUSED void *bp, void *p)
{
  AF_PROVIDER *xpro;

  xpro=(AF_PROVIDER *)p;

  AQFINTS_Parser_free(xpro->parser);

  GWEN_FREE_OBJECT(xpro);

}



int _provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
{
  AF_PROVIDER *xpro;
  const char *logLevelName;
  uint32_t currentVersion;

  /* setup logging */
  logLevelName=getenv("AQFINTS_LOGLEVEL");
  if (logLevelName) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(logLevelName);
    if (ll!=GWEN_LoggerLevel_Unknown) {
      GWEN_Logger_SetLevel(AQFINTS_LOGDOMAIN, ll);
      DBG_WARN(AQFINTS_LOGDOMAIN, "Overriding loglevel for AqFinTS with \"%s\"", logLevelName);
    }
    else {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "Unknown loglevel \"%s\"", logLevelName);
    }
  }

  DBG_NOTICE(AQFINTS_LOGDOMAIN, "Initializing AqFinTS backend");


  assert(pro);
  xpro=GWEN_INHERIT_GETDATA(AB_PROVIDER, AF_PROVIDER, pro);
  assert(xpro);

  currentVersion=
    (AQBANKING_VERSION_MAJOR<<24) |
    (AQBANKING_VERSION_MINOR<<16) |
    (AQBANKING_VERSION_PATCHLEVEL<<8) |
    AQBANKING_VERSION_BUILD;
  xpro->lastVersion=GWEN_DB_GetIntValue(dbData, "lastVersion", 0, 0);
  if (xpro->lastVersion<currentVersion) {
    DBG_WARN(AQFINTS_LOGDOMAIN, "AqFinTS version is newer.");
  }

  GWEN_PathManager_DefinePath(AF_PM_LIBNAME, AF_PM_FINTSDATADIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
  GWEN_PathManager_AddRelPath(AF_PM_LIBNAME,
                              AF_PM_LIBNAME,
                              AF_PM_FINTSDATADIR,
                              AF_FINTSDATADIR,
                              GWEN_PathManager_RelModeExe);
#else
  GWEN_PathManager_AddPath(AF_PM_LIBNAME,
                           AF_PM_LIBNAME,
                           AF_PM_FINTSDATADIR,
                           AF_FINTSDATADIR);
#endif

  return 0;
}



int _provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
{
  uint32_t currentVersion;

  currentVersion=
    (AQBANKING_VERSION_MAJOR<<24) |
    (AQBANKING_VERSION_MINOR<<16) |
    (AQBANKING_VERSION_PATCHLEVEL<<8) |
    AQBANKING_VERSION_BUILD;

  DBG_NOTICE(AQFINTS_LOGDOMAIN, "Setting version %08x", currentVersion);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "lastVersion", currentVersion);

  /* undefine our own paths */
  GWEN_PathManager_UndefinePath(AF_PM_LIBNAME, AF_PM_FINTSDATADIR);
  /* remove AqFinTS additions to all pathmanagers */
  GWEN_PathManager_RemovePaths(AF_PM_LIBNAME);

  return 0;
}




AQFINTS_PARSER *AF_Provider_CreateParser(AB_PROVIDER *pro)
{
  GWEN_STRINGLIST *paths;

  paths=GWEN_PathManager_GetPaths(AF_PM_LIBNAME, AF_PM_FINTSDATADIR);
  if (paths) {
    AQFINTS_PARSER *parser;
    GWEN_STRINGLISTENTRY *se;
    int rv;

    parser=AQFINTS_Parser_new();

    se=GWEN_StringList_FirstEntry(paths);
    while (se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s)
        AQFINTS_Parser_AddPath(parser, s);

      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(paths);

    rv=AQFINTS_Parser_ReadFiles(parser);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      AQFINTS_Parser_free(parser);
      return NULL;
    }

    return parser;
  }
  else {
    DBG_INFO(AQFINTS_LOGDOMAIN, "No paths");
    return NULL;
  }
}






